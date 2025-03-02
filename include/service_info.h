#ifndef SERVICE_INFO_H
#define SERVICE_INFO_H

#include <string>
#include <vector>
#include <functional>

enum class ServiceStatus {
    ACTIVE,
    INACTIVE,
    FAILED,
    UNKNOWN
};

struct ServiceInfo {
    std::string name;
    std::string description;
    std::string loadState;
    std::string activeState;
    std::string subState;
    std::string unitFileState;
    ServiceStatus status;
};

class ServiceInfoCollector {
public:
    ServiceInfoCollector();

    // Get all services
    std::vector<ServiceInfo> getAllServices();

    // Get service details
    ServiceInfo getServiceDetails(const std::string& serviceName);

    // Service control operations
    bool startService(const std::string& serviceName);
    bool stopService(const std::string& serviceName);
    bool restartService(const std::string& serviceName);
    bool reloadService(const std::string& serviceName);
    bool enableService(const std::string& serviceName);
    bool disableService(const std::string& serviceName);

    // Refresh service list
    void refresh();

    // Print service information
    void printAllInfo() const;

private:
    std::vector<ServiceInfo> services;

    // Helper methods
    void collectServiceInfo();
    std::string executeCommand(const std::string& command);
    ServiceStatus getStatusFromActiveState(const std::string& activeState);
    bool executeServiceCommand(const std::string& serviceName, const std::string& command);
};

#endif // SERVICE_INFO_H