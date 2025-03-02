#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <string>
#include <vector>
#include <map>

class CPUInfo {
public:
    CPUInfo();

    // Getters
    std::string getProcessorName() const;
    std::string getVendorId() const;
    int getPhysicalCores() const;
    int getLogicalCores() const;
    double getCurrentFrequencyMHz() const;
    double getMinFrequencyMHz() const;
    double getMaxFrequencyMHz() const;
    std::map<std::string, size_t> getCacheSizes() const;
    std::vector<std::string> getCPUFlags() const;

    // Print all information
    void printAllInfo() const;

    // Refresh CPU information
    void refresh();

private:
    // CPU information
    std::string processorName;
    std::string vendorId;
    int physicalCores;
    int logicalCores;
    double currentFrequencyMHz;
    double minFrequencyMHz;
    double maxFrequencyMHz;
    std::map<std::string, size_t> cacheSizes;
    std::vector<std::string> cpuFlags;
    std::vector<std::map<std::string, std::string>> processors;
    std::vector<std::map<std::string, std::string>> cacheInfo;

    // Helper methods
    void parseCPUInfo();
    void parseFrequencyInfo();
    void parseCacheInfo();
    std::string readFileContent(const std::string& filePath) const;

    // Collection methods for refresh
    void collectProcessorInfo();
    void collectCacheInfo();
    void collectCPUFlags();
};

#endif // CPU_INFO_H