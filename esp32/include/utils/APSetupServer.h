#ifndef AP_SETUP_SERVER_H
#define AP_SETUP_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include "utils/ProvisioningManager.h"

class APSetupServer {
public:
    APSetupServer(ProvisioningManager* provisioningManager);
    void begin();
    void update();
    bool isRunning() const { return m_running; }
    void stop();
    
    // Callback for when provisioning is complete
    using ProvisioningCompleteCallback = std::function<void()>;
    void onProvisioningComplete(ProvisioningCompleteCallback callback) {
        m_onProvisioningComplete = callback;
    }

private:
    WebServer m_server;
    ProvisioningManager* m_provisioningManager;
    bool m_running;
    ProvisioningCompleteCallback m_onProvisioningComplete;
    
    void handleRoot();
    void handleProvision();
    void handleStatus();
    void handleReset();
    void handleScanNetworks();
    
    String generateSetupHTML();
    String generateStatusJSON();
    String generateScanJSON();
};

#endif // AP_SETUP_SERVER_H
