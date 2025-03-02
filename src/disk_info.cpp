#include "disk_info.h"
#include <sys/statvfs.h>
#include <mntent.h>
#include <fstream>
#include <sstream>
#include <QDebug>

DiskInfoCollector::DiskInfoCollector() {
    qDebug() << "Initializing DiskInfoCollector...";
    try {
        refresh();
        qDebug() << "DiskInfoCollector initialized successfully.";
    } catch (const std::exception& e) {
        qCritical() << "Exception during DiskInfoCollector initialization:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception during DiskInfoCollector initialization";
    }
}

void DiskInfoCollector::refresh() {
    disks.clear();
    collectDiskInfo();
}

std::vector<DiskInfo> DiskInfoCollector::getDisks() const {
    if (disks.empty()) {
        qWarning() << "No disk information available.";
    }
    return disks;
}

void DiskInfoCollector::collectDiskInfo() {
    FILE* mtab = setmntent("/etc/mtab", "r");
    if (mtab == nullptr) {
        return;
    }

    struct mntent* entry;
    while ((entry = getmntent(mtab)) != nullptr) {
        // Skip pseudo filesystems
        std::string fsType = entry->mnt_type;
        if (fsType == "proc" || fsType == "sysfs" || fsType == "devpts" ||
            fsType == "tmpfs" || fsType == "devtmpfs" || fsType == "debugfs" ||
            fsType == "securityfs" || fsType == "cgroup" || fsType == "pstore" ||
            fsType == "autofs" || fsType == "mqueue" || fsType == "hugetlbfs" ||
            fsType == "fusectl" || fsType == "fuse.gvfsd-fuse" || fsType == "binfmt_misc") {
            continue;
        }

        struct statvfs stat;
        if (statvfs(entry->mnt_dir, &stat) != 0) {
            continue;
        }

        DiskInfo disk;
        disk.device = entry->mnt_fsname;
        disk.mountPoint = entry->mnt_dir;
        disk.fsType = entry->mnt_type;
        disk.totalSize = stat.f_blocks * stat.f_frsize;
        disk.freeSpace = stat.f_bfree * stat.f_frsize;
        disk.availableSpace = stat.f_bavail * stat.f_frsize;

        disks.push_back(disk);
    }

    endmntent(mtab);
}

std::string DiskInfoCollector::formatSize(uint64_t size) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double formattedSize = static_cast<double>(size);

    while (formattedSize >= 1024.0 && unitIndex < 4) {
        formattedSize /= 1024.0;
        unitIndex++;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << formattedSize << " " << units[unitIndex];
    return ss.str();
}

void DiskInfoCollector::printAllInfo() const {
    std::cout << "Disk Information:" << std::endl;
    std::cout << "----------------" << std::endl;

    for (const auto& disk : disks) {
        std::cout << "Device: " << disk.device << std::endl;
        std::cout << "Mount Point: " << disk.mountPoint << std::endl;
        std::cout << "File System: " << disk.fsType << std::endl;

        std::cout << "Total Size: " << formatSize(disk.totalSize) << std::endl;
        std::cout << "Free Space: " << formatSize(disk.freeSpace) << std::endl;
        std::cout << "Available Space: " << formatSize(disk.availableSpace) << std::endl;
        std::cout << "Used Space: " << formatSize(disk.totalSize - disk.freeSpace) << std::endl;
        std::cout << "Usage: " << std::fixed << std::setprecision(2)
                  << (static_cast<double>(disk.totalSize - disk.freeSpace) / disk.totalSize * 100.0)
                  << "%" << std::endl;
        std::cout << std::endl;
    }
}