#ifndef DISK_INFO_H
#define DISK_INFO_H

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>

struct DiskInfo {
    std::string device;
    std::string mountPoint;
    std::string fsType;
    uint64_t totalSize;
    uint64_t freeSpace;
    uint64_t availableSpace;
};

class DiskInfoCollector {
public:
    DiskInfoCollector();

    // Get disk information
    std::vector<DiskInfo> getDisks() const;

    // Print disk information
    void printAllInfo() const;

    // Add this to the public section of the DiskInfoCollector class
    void refresh();

private:
    std::vector<DiskInfo> disks;

    // Helper methods
    void collectDiskInfo();

    // Format size for display
    std::string formatSize(uint64_t size) const;
};

#endif // DISK_INFO_H