#include "utils/ProvisioningManager.h"
#include <esp_err.h>

ProvisioningManager::ProvisioningManager() 
    : m_initialized(false), m_nvsHandle(0) {
}

bool ProvisioningManager::begin() {
    if (m_initialized) {
        return true;
    }
    
    Serial.println("[NVS] Initializing NVS...");
    
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("[NVS] Erasing NVS partition...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    if (err != ESP_OK) {
        Serial.print("[NVS] Init failed: ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &m_nvsHandle);
    if (err != ESP_OK) {
        Serial.print("[NVS] Open failed: ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    m_initialized = true;
    Serial.println("[NVS] Initialized successfully");
    return true;
}

BootMode ProvisioningManager::determineBootMode() {
    if (!m_initialized) {
        Serial.println("[Boot] NVS not initialized, forcing UNPROVISIONED");
        return BootMode::UNPROVISIONED;
    }
    
    bool hasWiFi = readString(KEY_WIFI_SSID).length() > 0;
    bool hasClaimCode = readString(KEY_CLAIM_CODE).length() > 0;
    bool provisionedFlag = readBool(KEY_PROVISIONED, false);
    bool claimedFlag = readBool(KEY_CLAIMED, false);
    int recoveryCount = getRecoveryCount();
    
    Serial.println("[Boot] Checking boot mode:");
    Serial.print("[Boot]   WiFi configured: ");
    Serial.println(hasWiFi ? "yes" : "no");
    Serial.print("[Boot]   Claim code: ");
    Serial.println(hasClaimCode ? "yes" : "no");
    Serial.print("[Boot]   Provisioned: ");
    Serial.println(provisionedFlag ? "yes" : "no");
    Serial.print("[Boot]   Claimed: ");
    Serial.println(claimedFlag ? "yes" : "no");
    Serial.print("[Boot]   Recovery count: ");
    Serial.println(recoveryCount);
    
    if (!hasWiFi || !hasClaimCode || !provisionedFlag) {
        return BootMode::UNPROVISIONED;
    }
    
    if (!claimedFlag) {
        return BootMode::PROVISIONED_UNCLAIMED;
    }
    
    if (recoveryCount >= 5) {
        Serial.println("[Boot] Recovery threshold reached, entering recovery mode");
        clearRecoveryCount();
        return BootMode::RECOVERY;
    }
    
    return BootMode::RUNNING;
}

String ProvisioningManager::getWiFiSSID() {
    return readString(KEY_WIFI_SSID);
}

String ProvisioningManager::getWiFiPassword() {
    return readString(KEY_WIFI_PASS);
}

String ProvisioningManager::getClaimCode() {
    return readString(KEY_CLAIM_CODE);
}

String ProvisioningManager::getDeviceId() {
    return readString(KEY_DEVICE_ID);
}

String ProvisioningManager::getOwnerUid() {
    return readString(KEY_OWNER_UID);
}

bool ProvisioningManager::isProvisioned() {
    return readBool(KEY_PROVISIONED, false);
}

bool ProvisioningManager::isClaimed() {
    return readBool(KEY_CLAIMED, false);
}

int ProvisioningManager::getRecoveryCount() {
    return readInt(KEY_RECOVERY_COUNT, 0);
}

bool ProvisioningManager::saveWiFiCredentials(const String& ssid, const String& password) {
    bool ssidResult = writeString(KEY_WIFI_SSID, ssid);
    bool passResult = writeString(KEY_WIFI_PASS, password);
    return ssidResult && passResult;
}

bool ProvisioningManager::saveClaimCode(const String& code) {
    return writeString(KEY_CLAIM_CODE, code);
}

bool ProvisioningManager::saveDeviceId(const String& id) {
    return writeString(KEY_DEVICE_ID, id);
}

bool ProvisioningManager::saveOwnerUid(const String& uid) {
    return writeString(KEY_OWNER_UID, uid);
}

bool ProvisioningManager::setProvisioned(bool value) {
    return writeBool(KEY_PROVISIONED, value);
}

bool ProvisioningManager::setClaimed(bool value) {
    return writeBool(KEY_CLAIMED, value);
}

bool ProvisioningManager::incrementRecoveryCount() {
    int current = getRecoveryCount();
    return writeInt(KEY_RECOVERY_COUNT, current + 1);
}

bool ProvisioningManager::clearRecoveryCount() {
    return writeInt(KEY_RECOVERY_COUNT, 0);
}

bool ProvisioningManager::saveProvisioningData(const String& ssid, const String& password, 
                                               const String& claimCode, const String& deviceName) {
    Serial.println("[NVS] Saving provisioning data...");
    
    bool ssidOk = writeString(KEY_WIFI_SSID, ssid);
    bool passOk = writeString(KEY_WIFI_PASS, password);
    bool claimOk = writeString(KEY_CLAIM_CODE, claimCode);
    bool provOk = writeBool(KEY_PROVISIONED, true);
    bool claimedOk = writeBool(KEY_CLAIMED, false);
    
    if (deviceName.length() > 0) {
        writeString(KEY_DEVICE_ID, deviceName);
    }
    
    bool success = ssidOk && passOk && claimOk && provOk && claimedOk;
    
    if (success) {
        Serial.println("[NVS] Provisioning data saved successfully");
    } else {
        Serial.println("[NVS] Failed to save some provisioning data");
    }
    
    return success;
}

bool ProvisioningManager::factoryReset() {
    Serial.println("[NVS] Factory reset initiated...");
    
    nvs_close(m_nvsHandle);
    
    esp_err_t err = nvs_flash_erase();
    if (err != ESP_OK) {
        Serial.print("[NVS] Erase failed: ");
        Serial.println(esp_err_to_name(err));
        nvs_open(NVS_NAMESPACE, NVS_READWRITE, &m_nvsHandle);
        return false;
    }
    
    err = nvs_flash_init();
    if (err != ESP_OK) {
        Serial.print("[NVS] Re-init failed: ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &m_nvsHandle);
    if (err != ESP_OK) {
        Serial.print("[NVS] Re-open failed: ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    Serial.println("[NVS] Factory reset complete");
    return true;
}

String ProvisioningManager::getHardwareId() {
    uint8_t mac[6];
    esp_err_t err = esp_efuse_mac_get_default(mac);
    if (err != ESP_OK) {
        Serial.print("[Hardware] Failed to get MAC: ");
        Serial.println(esp_err_to_name(err));
        return "0000";
    }
    
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%02X%02X", mac[4], mac[5]);
    return String(buffer);
}

String ProvisioningManager::readString(const char* key) {
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(m_nvsHandle, key, NULL, &required_size);
    if (err != ESP_OK || required_size == 0) {
        return String();
    }
    
    char* buffer = new char[required_size];
    err = nvs_get_str(m_nvsHandle, key, buffer, &required_size);
    if (err != ESP_OK) {
        delete[] buffer;
        return String();
    }
    
    String result = String(buffer);
    delete[] buffer;
    return result;
}

bool ProvisioningManager::writeString(const char* key, const String& value) {
    if (value.length() == 0) {
        nvs_erase_key(m_nvsHandle, key);
        return nvs_commit(m_nvsHandle) == ESP_OK;
    }
    
    esp_err_t err = nvs_set_str(m_nvsHandle, key, value.c_str());
    if (err != ESP_OK) {
        Serial.print("[NVS] Write string failed for ");
        Serial.print(key);
        Serial.print(": ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    err = nvs_commit(m_nvsHandle);
    if (err != ESP_OK) {
        Serial.print("[NVS] Commit failed for ");
        Serial.print(key);
        Serial.print(": ");
        Serial.println(esp_err_to_name(err));
        return false;
    }
    
    return true;
}

bool ProvisioningManager::readBool(const char* key, bool defaultValue) {
    uint8_t value = defaultValue ? 1 : 0;
    esp_err_t err = nvs_get_u8(m_nvsHandle, key, &value);
    return err == ESP_OK ? (value != 0) : defaultValue;
}

bool ProvisioningManager::writeBool(const char* key, bool value) {
    esp_err_t err = nvs_set_u8(m_nvsHandle, key, value ? 1 : 0);
    if (err != ESP_OK) {
        return false;
    }
    return nvs_commit(m_nvsHandle) == ESP_OK;
}

int ProvisioningManager::readInt(const char* key, int defaultValue) {
    int32_t value = defaultValue;
    esp_err_t err = nvs_get_i32(m_nvsHandle, key, &value);
    return err == ESP_OK ? value : defaultValue;
}

bool ProvisioningManager::writeInt(const char* key, int value) {
    esp_err_t err = nvs_set_i32(m_nvsHandle, key, value);
    if (err != ESP_OK) {
        return false;
    }
    return nvs_commit(m_nvsHandle) == ESP_OK;
}
