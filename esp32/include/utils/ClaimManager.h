#ifndef CLAIM_MANAGER_H
#define CLAIM_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "utils/ProvisioningManager.h"

struct ClaimResult {
    bool success;
    String deviceId;
    String ownerId;
    String errorMessage;
};

class ClaimManager {
public:
    ClaimManager(
        ProvisioningManager* provisioningManager,
        const char* apiKey,
        const char* projectId,
        const char* databaseUrl
    );
    
    ClaimResult redeemClaim();

private:
    ProvisioningManager* m_provisioningManager;
    const char* m_apiKey;
    const char* m_projectId;
    const char* m_databaseUrl;
    
    String getHardwareId();
};

#endif // CLAIM_MANAGER_H
