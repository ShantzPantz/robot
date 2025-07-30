// Helper class for handling over the air updates
#ifndef OTAUPDATEMANAGER_H
#define OTAUPDATEMANAGER_H

class OtaUpdateManager {
public:
    OtaUpdateManager();
    ~OtaUpdateManager();

    void init(const char* ota_hostname);
    void checkForUpdate();

private:
    const char* ota_hostname_;
    const char* ota_password_;
};
#endif