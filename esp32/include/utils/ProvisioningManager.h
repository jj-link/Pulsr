#ifndef PROVISIONING_MANAGER_H
#define PROVISIONING_MANAGER_H

#include <Arduino.h>
#include <nvs.h>
#include <nvs_flash.h>

enum class BootMode {
    UNPROVISIONED,        // Missing required NVS keys; start AP setup mode
    PROVISIONED_UNCLAIMED, // WiFi creds exist; attempt claim redemption
    RUNNING,              // Claim complete; run existing Firebase manager loop
    RECOVERY              // Repeated failures; return to AP mode
};

struct ProvisioningData {
    String wifiSSID;
    String wifiPassword;
    String claimCode;
    String deviceId;
    String ownerUid;
    bool provisioned;
    bool claimed;
};

class ProvisioningManager {
public:
    static constexpr const char* NVS_NAMESPACE = "pulsr";
    static constexpr const char* KEY_WIFI_SSID = "wifi_ssid";
    static constexpr const char* KEY_WIFI_PASS = "wifi_pass";
    static constexpr const char* KEY_CLAIM_CODE = "claim_code";
    static constexpr const char* KEY_DEVICE_ID = "device_id";
    static constexpr const char* KEY_OWNER_UID = "owner_uid";
    static constexpr const char* KEY_PROVISIONED = "provisioned";
    static constexpr const char* KEY_CLAIMED = "claimed";
    static constexpr const char* KEY_RECOVERY_COUNT = "recovery_cnt";

    ProvisioningManager();
    bool begin();
    
    BootMode determineBootMode();
    
    // Getters
    String getWiFiSSID();
    String getWiFiPassword();
    String getClaimCode();
    String getDeviceId();
    String getOwnerUid();
    bool isProvisioned();
    bool isClaimed();
    int getRecoveryCount();
    
    // Setters
    bool saveWiFiCredentials(const String& ssid, const String& password);
    bool saveClaimCode(const String& code);
    bool saveDeviceId(const String& id);
    bool saveOwnerUid(const String& uid);
    bool setProvisioned(bool value);
    bool setClaimed(bool value);
    bool incrementRecoveryCount();
    bool clearRecoveryCount();
    
    // Full provisioning save
    bool saveProvisioningData(const String& ssid, const String& password, 
                            const String& claimCode, const String& deviceName);
    
    // Factory reset
    bool factoryReset();
    
    // Generate hardware ID from MAC
    String getHardwareId();

private:
    bool m_initialized;
    nvs_handle_t m_nvsHandle;
    
    String readString(const char* key);
    bool writeString(const char* key, const String& value);
    bool readBool(const char* key, bool defaultValue);
    bool writeBool(const char* key, bool value);
    int readInt(const char* key, int defaultValue);
    bool writeInt(const char* key, int value);
};

#endif // PROVISIONING_MANAGER_H
