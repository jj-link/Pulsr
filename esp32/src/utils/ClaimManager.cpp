#include "utils/ClaimManager.h"
#include "utils/ProvisioningManager.h"
#include <ArduinoJson.h>

ClaimManager::ClaimManager(
    ProvisioningManager* provisioningManager,
    const char* apiKey,
    const char* projectId,
    const char* databaseUrl
) : m_provisioningManager(provisioningManager),
    m_apiKey(apiKey),
    m_projectId(projectId),
    m_databaseUrl(databaseUrl) {
}

String ClaimManager::getHardwareId() {
    uint8_t mac[6];
    esp_err_t err = esp_efuse_mac_get_default(mac);
    if (err != ESP_OK) {
        Serial.print("[Claim] Failed to get MAC: ");
        Serial.println(esp_err_to_name(err));
        return "000000000000";
    }
    
    char buffer[13];
    snprintf(buffer, sizeof(buffer), "%02X%02X%02X%02X%02X%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buffer);
}

ClaimResult ClaimManager::redeemClaim() {
    ClaimResult result;
    result.success = false;
    
    String claimCode = m_provisioningManager->getClaimCode();
    String hardwareId = getHardwareId();
    
    if (claimCode.length() == 0) {
        result.errorMessage = "No claim code found";
        Serial.println("[Claim] No claim code in NVS");
        return result;
    }
    
    Serial.println("[Claim] Redeeming claim code...");
    Serial.print("[Claim] Hardware ID: ");
    Serial.println(hardwareId);
    Serial.print("[Claim] Claim code: ");
    Serial.println(claimCode);
    
    HTTPClient http;
    String url = String("https://") + m_projectId + ".web.app/api/claims/redeem";
    
    Serial.print("[Claim] Calling: ");
    Serial.println(url);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", m_apiKey);
    
    StaticJsonDocument<256> requestDoc;
    requestDoc["claimCode"] = claimCode;
    requestDoc["hardwareId"] = hardwareId;
    
    String requestBody;
    serializeJson(requestDoc, requestBody);
    
    int httpCode = http.POST(requestBody);
    String response = http.getString();
    http.end();
    
    Serial.print("[Claim] HTTP response: ");
    Serial.println(httpCode);
    Serial.print("[Claim] Response body: ");
    Serial.println(response);
    
    if (httpCode == 200) {
        StaticJsonDocument<512> responseDoc;
        DeserializationError error = deserializeJson(responseDoc, response);
        
        if (error) {
            result.errorMessage = "Failed to parse response";
            Serial.print("[Claim] JSON parse error: ");
            Serial.println(error.c_str());
            return result;
        }
        
        if (responseDoc["success"] == true) {
            result.success = true;
            result.deviceId = responseDoc["deviceId"].as<String>();
            result.ownerId = responseDoc["ownerId"].as<String>();
            
            m_provisioningManager->saveDeviceId(result.deviceId);
            m_provisioningManager->saveOwnerUid(result.ownerId);
            m_provisioningManager->setClaimed(true);
            
            Serial.println("[Claim] Claim redeemed successfully!");
            Serial.print("[Claim] Device ID: ");
            Serial.println(result.deviceId);
            Serial.print("[Claim] Owner ID: ");
            Serial.println(result.ownerId);
        } else {
            result.errorMessage = responseDoc["error"].as<String>();
            Serial.print("[Claim] Claim failed: ");
            Serial.println(result.errorMessage);
        }
    } else if (httpCode == 410) {
        result.errorMessage = "Claim code expired or already used";
        m_provisioningManager->incrementRecoveryCount();
    } else if (httpCode == 404) {
        result.errorMessage = "Invalid claim code";
        m_provisioningManager->incrementRecoveryCount();
    } else {
        result.errorMessage = "Server error: " + String(httpCode);
        Serial.printf("[Claim] Server error: %d\n", httpCode);
    }
    
    return result;
}
