#include "utils/APSetupServer.h"
#include "utils/ProvisioningManager.h"
#include <WiFi.h>
#include <ArduinoJson.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pulsr Setup</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #f5f5f5; padding: 20px; }
        .container { max-width: 400px; margin: 0 auto; background: white; border-radius: 12px; padding: 24px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }
        h1 { text-align: center; margin-bottom: 24px; color: #333; }
        .form-group { margin-bottom: 16px; }
        label { display: block; margin-bottom: 6px; font-weight: 500; color: #555; }
        input, select { width: 100%; padding: 12px; border: 1px solid #ddd; border-radius: 8px; font-size: 16px; }
        input:focus, select:focus { outline: none; border-color: #007AFF; }
        button { width: 100%; padding: 14px; background: #007AFF; color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; }
        button:hover { background: #0056b3; }
        button:disabled { background: #ccc; cursor: not-allowed; }
        .error { color: #ff3b30; font-size: 14px; margin-top: 8px; }
        .success { color: #34c759; font-size: 14px; margin-top: 8px; }
        .hint { font-size: 12px; color: #888; margin-top: 4px; }
        .scan-btn { background: #34c759; margin-top: 8px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Pulsr Setup</h1>
        <form id="setupForm">
            <div class="form-group">
                <label for="ssid">WiFi Network</label>
                <select id="ssid" name="ssid" required>
                    <option value="">Select a network...</option>
                </select>
                <button type="button" class="scan-btn" onclick="scanNetworks()">Scan Networks</button>
            </div>
            <div class="form-group">
                <label for="password">WiFi Password</label>
                <input type="password" id="password" name="password" placeholder="Enter your WiFi password">
            </div>
            <div class="form-group">
                <label for="claimCode">Claim Code</label>
                <input type="text" id="claimCode" name="claimCode" placeholder="e.g. PULSR-ABCD" required style="text-transform: uppercase;">
                <div class="hint">Get this from the Pulsr web app</div>
            </div>
            <div class="form-group">
                <label for="deviceName">Device Name (optional)</label>
                <input type="text" id="deviceName" name="deviceName" placeholder="e.g. Living Room TV">
            </div>
            <button type="submit" id="submitBtn">Save and Reboot</button>
            <div id="message"></div>
        </form>
    </div>
    <script>
        function showMessage(msg, isError) {
            var el = document.getElementById('message');
            el.innerHTML = '<div class="' + (isError ? 'error' : 'success') + '">' + msg + '</div>';
        }
        
        async function scanNetworks() {
            var btn = document.querySelector('.scan-btn');
            btn.disabled = true;
            btn.textContent = 'Scanning...';
            
            try {
                var res = await fetch('/api/scan');
                var networks = await res.json();
                var select = document.getElementById('ssid');
                select.innerHTML = '<option value="">Select a network...</option>';
                networks.sort(function(a, b) { return b.rssi - a.rssi; }).forEach(function(n) {
                    var opt = document.createElement('option');
                    opt.value = n.ssid;
                    opt.textContent = n.ssid + ' (' + n.rssi + ' dBm)';
                    select.appendChild(opt);
                });
            } catch(e) {
                showMessage('Failed to scan networks', true);
            }
            
            btn.disabled = false;
            btn.textContent = 'Scan Networks';
        }
        
        document.getElementById('setupForm').addEventListener('submit', async function(e) {
            e.preventDefault();
            var btn = document.getElementById('submitBtn');
            btn.disabled = true;
            btn.textContent = 'Saving...';
            
            var formData = new FormData(e.target);
            
            try {
                var res = await fetch('/api/provision', {
                    method: 'POST',
                    body: formData
                });
                var data = await res.json();
                
                if (data.success) {
                    showMessage(data.message);
                    btn.textContent = 'Saved! Rebooting...';
                    setTimeout(function() { location.reload(); }, 2000);
                } else {
                    showMessage(data.error || 'Failed to save', true);
                    btn.disabled = false;
                    btn.textContent = 'Save and Reboot';
                }
            } catch(e) {
                showMessage('Request failed: ' + e.message, true);
                btn.disabled = false;
                btn.textContent = 'Save and Reboot';
            }
        });
        
        scanNetworks();
    </script>
</body>
</html>)rawliteral";

APSetupServer::APSetupServer(ProvisioningManager* provisioningManager)
    : m_server(80), m_provisioningManager(provisioningManager), m_running(false) {
}

void APSetupServer::begin() {
    Serial.println("[AP] Starting AP setup mode...");
    
    String apName = "Pulsr-Setup-" + m_provisioningManager->getHardwareId();
    
    Serial.print("[AP] Starting access point: ");
    Serial.println(apName);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str());
    
    delay(500);
    
    Serial.print("[AP] IP address: ");
    Serial.println(WiFi.softAPIP());
    
    m_server.on("/", HTTP_GET, [this]() { handleRoot(); });
    m_server.on("/api/provision", HTTP_POST, [this]() { handleProvision(); });
    m_server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    m_server.on("/api/reset", HTTP_POST, [this]() { handleReset(); });
    m_server.on("/api/scan", HTTP_GET, [this]() { handleScanNetworks(); });
    
    m_server.begin();
    m_running = true;
    Serial.println("[AP] Setup server started");
}

void APSetupServer::update() {
    if (m_running) {
        m_server.handleClient();
    }
}

void APSetupServer::stop() {
    if (m_running) {
        m_server.stop();
        WiFi.softAPdisconnect(true);
        m_running = false;
        Serial.println("[AP] Setup server stopped");
    }
}

void APSetupServer::handleRoot() {
    m_server.send(200, "text/html", INDEX_HTML);
}

void APSetupServer::handleProvision() {
    Serial.println("[AP] Provision request received");
    
    if (!m_server.hasArg("ssid") || !m_server.hasArg("password") || !m_server.hasArg("claimCode")) {
        m_server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing required fields\"}");
        return;
    }
    
    String ssid = m_server.arg("ssid");
    String password = m_server.arg("password");
    String claimCode = m_server.arg("claimCode");
    String deviceName = m_server.arg("deviceName");
    
    Serial.print("[AP] SSID: ");
    Serial.println(ssid);
    Serial.print("[AP] Claim code: ");
    Serial.println(claimCode);
    
    bool success = m_provisioningManager->saveProvisioningData(ssid, password, claimCode, deviceName);
    
    if (success) {
        Serial.println("[AP] Provisioning saved, rebooting...");
        
        if (m_onProvisioningComplete) {
            m_onProvisioningComplete();
        }
        
        m_server.send(200, "application/json", "{\"success\":true,\"message\":\"Provisioning saved. Rebooting...\"}");
        delay(1000);
        ESP.restart();
    } else {
        m_server.send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save provisioning data\"}");
    }
}

void APSetupServer::handleStatus() {
    m_server.send(200, "application/json", generateStatusJSON());
}

void APSetupServer::handleReset() {
    Serial.println("[AP] Factory reset requested");
    
    bool success = m_provisioningManager->factoryReset();
    
    if (success) {
        m_server.send(200, "application/json", "{\"success\":true,\"message\":\"Factory reset complete. Rebooting...\"}");
        delay(1000);
        ESP.restart();
    } else {
        m_server.send(500, "application/json", "{\"success\":false,\"error\":\"Factory reset failed\"}");
    }
}

void APSetupServer::handleScanNetworks() {
    Serial.println("[AP] Scanning for WiFi networks...");
    
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"";
        json += WiFi.SSID(i);
        json += "\",\"rssi\":";
        json += WiFi.RSSI(i);
        json += "}";
    }
    json += "]";
    WiFi.scanDelete();
    
    m_server.send(200, "application/json", json);
}

String APSetupServer::generateStatusJSON() {
    StaticJsonDocument<256> doc;
    doc["provisioned"] = m_provisioningManager->isProvisioned();
    doc["claimed"] = m_provisioningManager->isClaimed();
    doc["deviceId"] = m_provisioningManager->getDeviceId();
    doc["hardwareId"] = m_provisioningManager->getHardwareId();
    
    String output;
    serializeJson(doc, output);
    return output;
}
