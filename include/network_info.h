#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#include <string>
#include <vector>
#include <cstdint>

struct NetworkInterface {
    std::string name;
    std::string ipAddress;
    std::string macAddress;
    std::string netmask;
    std::string broadcast;
    bool isUp;
    bool isLoopback;
    bool isWireless;
    uint64_t rxBytes;
    uint64_t txBytes;
    uint64_t rxPackets;
    uint64_t txPackets;
    uint64_t rxErrors;
    uint64_t txErrors;
};

class NetworkInfoCollector {
public:
    NetworkInfoCollector();

    // Get network interfaces
    std::vector<NetworkInterface> getInterfaces() const;

    // Refresh network information
    void refresh();

    // Print network information
    void printAllInfo() const;

    // Format bytes to human-readable string
    std::string formatBytes(uint64_t bytes) const;

private:
    std::vector<NetworkInterface> interfaces;

    // Helper methods
    void collectNetworkInfo();
    bool isWirelessInterface(const std::string& name) const;
};

#endif // NETWORK_INFO_H