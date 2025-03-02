#include "cpu_info.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <thread>
#include <filesystem>
#include <unistd.h>
#include <sys/statvfs.h>
#include <mntent.h>

CPUInfo::CPUInfo()
    : physicalCores(0),
      logicalCores(0),
      currentFrequencyMHz(0.0),
      minFrequencyMHz(0.0),
      maxFrequencyMHz(0.0) {

    // Initialize with available hardware concurrency
    logicalCores = std::thread::hardware_concurrency();

    // Parse CPU information from system files
    parseCPUInfo();
    parseFrequencyInfo();
    parseCacheInfo();
}

std::string CPUInfo::getProcessorName() const {
    return processorName;
}

std::string CPUInfo::getVendorId() const {
    return vendorId;
}

int CPUInfo::getPhysicalCores() const {
    return physicalCores;
}

int CPUInfo::getLogicalCores() const {
    return logicalCores;
}

double CPUInfo::getCurrentFrequencyMHz() const {
    return currentFrequencyMHz;
}

double CPUInfo::getMinFrequencyMHz() const {
    return minFrequencyMHz;
}

double CPUInfo::getMaxFrequencyMHz() const {
    return maxFrequencyMHz;
}

std::map<std::string, size_t> CPUInfo::getCacheSizes() const {
    return cacheSizes;
}

std::vector<std::string> CPUInfo::getCPUFlags() const {
    return cpuFlags;
}

void CPUInfo::parseCPUInfo() {
    std::string cpuInfoContent = readFileContent("/proc/cpuinfo");
    if (cpuInfoContent.empty()) {
        return;
    }

    std::istringstream stream(cpuInfoContent);
    std::string line;

    std::map<std::string, int> uniquePhysicalIds;

    while (std::getline(stream, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos + 2 < line.length()) {
                processorName = line.substr(colonPos + 2);
            }
        } else if (line.find("vendor_id") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos + 2 < line.length()) {
                vendorId = line.substr(colonPos + 2);
            }
        } else if (line.find("physical id") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos + 2 < line.length()) {
                std::string physicalId = line.substr(colonPos + 2);
                uniquePhysicalIds[physicalId]++;
            }
        } else if (line.find("flags") != std::string::npos) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos && colonPos + 2 < line.length()) {
                std::string flagsStr = line.substr(colonPos + 2);
                std::istringstream flagsStream(flagsStr);
                std::string flag;
                while (flagsStream >> flag) {
                    cpuFlags.push_back(flag);
                }
            }
        }
    }

    physicalCores = uniquePhysicalIds.size();
    if (physicalCores == 0) {
        // Fallback if we couldn't determine physical cores
        physicalCores = sysconf(_SC_NPROCESSORS_ONLN);
    }
}

void CPUInfo::parseFrequencyInfo() {
    // Try to read current CPU frequency
    std::string freqContent = readFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (!freqContent.empty()) {
        try {
            currentFrequencyMHz = std::stod(freqContent) / 1000.0;
        } catch (...) {
            currentFrequencyMHz = 0.0;
        }
    }

    // Try to read min CPU frequency
    freqContent = readFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    if (!freqContent.empty()) {
        try {
            minFrequencyMHz = std::stod(freqContent) / 1000.0;
        } catch (...) {
            minFrequencyMHz = 0.0;
        }
    }

    // Try to read max CPU frequency
    freqContent = readFileContent("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    if (!freqContent.empty()) {
        try {
            maxFrequencyMHz = std::stod(freqContent) / 1000.0;
        } catch (...) {
            maxFrequencyMHz = 0.0;
        }
    }
}

void CPUInfo::parseCacheInfo() {
    // Try to read L1, L2, and L3 cache sizes
    for (int level = 1; level <= 3; ++level) {
        std::string cacheDir = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(level - 1);

        if (!std::filesystem::exists(cacheDir)) {
            continue;
        }

        std::string sizeContent = readFileContent(cacheDir + "/size");
        if (!sizeContent.empty()) {
            // Parse cache size (e.g., "32K" or "8M")
            size_t size = 0;
            try {
                size = std::stoul(sizeContent);
                if (sizeContent.find('K') != std::string::npos) {
                    size *= 1024;
                } else if (sizeContent.find('M') != std::string::npos) {
                    size *= 1024 * 1024;
                }
            } catch (...) {
                size = 0;
            }

            std::string cacheType = readFileContent(cacheDir + "/type");
            if (!cacheType.empty()) {
                // Remove newline character
                if (!cacheType.empty() && cacheType.back() == '\n') {
                    cacheType.pop_back();
                }

                std::string cacheKey = "L" + std::to_string(level) + " " + cacheType;
                cacheSizes[cacheKey] = size;
            }
        }
    }
}

std::string CPUInfo::readFileContent(const std::string& filePath) const {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Remove trailing newline if present
    if (!content.empty() && content.back() == '\n') {
        content.pop_back();
    }

    return content;
}

void CPUInfo::printAllInfo() const {
    std::cout << "CPU Information:" << std::endl;
    std::cout << "----------------" << std::endl;

    std::cout << "Processor: " << processorName << std::endl;
    std::cout << "Vendor: " << vendorId << std::endl;
    std::cout << "Physical cores: " << physicalCores << std::endl;
    std::cout << "Logical cores: " << logicalCores << std::endl;

    std::cout << "\nFrequency Information:" << std::endl;
    std::cout << "---------------------" << std::endl;
    if (currentFrequencyMHz > 0) {
        std::cout << "Current frequency: " << currentFrequencyMHz << " MHz" << std::endl;
    }
    if (minFrequencyMHz > 0) {
        std::cout << "Minimum frequency: " << minFrequencyMHz << " MHz" << std::endl;
    }
    if (maxFrequencyMHz > 0) {
        std::cout << "Maximum frequency: " << maxFrequencyMHz << " MHz" << std::endl;
    }

    std::cout << "\nCache Information:" << std::endl;
    std::cout << "-----------------" << std::endl;
    for (const auto& [cacheType, size] : cacheSizes) {
        std::cout << cacheType << ": ";
        if (size >= 1024 * 1024) {
            std::cout << (size / (1024 * 1024)) << " MB" << std::endl;
        } else if (size >= 1024) {
            std::cout << (size / 1024) << " KB" << std::endl;
        } else {
            std::cout << size << " bytes" << std::endl;
        }
    }

    std::cout << "\nCPU Flags:" << std::endl;
    std::cout << "----------" << std::endl;
    const int FLAGS_PER_LINE = 5;
    for (size_t i = 0; i < cpuFlags.size(); ++i) {
        std::cout << cpuFlags[i];
        if ((i + 1) % FLAGS_PER_LINE == 0 || i == cpuFlags.size() - 1) {
            std::cout << std::endl;
        } else {
            std::cout << ", ";
        }
    }
}

void CPUInfo::refresh() {
    // Clear existing data
    processors.clear();
    cacheInfo.clear();
    cpuFlags.clear();

    // Re-collect all CPU information
    collectProcessorInfo();
    collectCacheInfo();
    collectCPUFlags();
}

void CPUInfo::collectProcessorInfo() {
    // Re-parse CPU information
    parseCPUInfo();
    parseFrequencyInfo();
}

void CPUInfo::collectCacheInfo() {
    // Re-parse cache information
    parseCacheInfo();
}

void CPUInfo::collectCPUFlags() {
    // CPU flags are collected in parseCPUInfo()
    // This is just a placeholder in case we want to separate the logic later
}