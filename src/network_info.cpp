#include "network_info.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/wireless.h>
#include <QDebug>

NetworkInfoCollector::NetworkInfoCollector() {
    qDebug() << "Initializing NetworkInfoCollector...";
    try {
        refresh();
        qDebug() << "NetworkInfoCollector initialized successfully.";
    } catch (const std::exception& e) {
        qCritical() << "Exception during NetworkInfoCollector initialization:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception during NetworkInfoCollector initialization";
    }
}

std::vector<NetworkInterface> NetworkInfoCollector::getInterfaces() const {
    return interfaces;
}

void NetworkInfoCollector::refresh() {
    interfaces.clear();
    collectNetworkInfo();
}

void NetworkInfoCollector::collectNetworkInfo() {
    // Get network interfaces
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return;
    }

    // Create a socket for ioctl calls
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        freeifaddrs(ifaddr);
        return;
    }

    // Iterate through network interfaces
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        // Only handle IPv4 interfaces
        if (ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        // Check if we already have this interface
        bool found = false;
        for (auto& iface : interfaces) {
            if (iface.name == ifa->ifa_name) {
                found = true;
                break;
            }
        }

        if (found) {
            continue;
        }

        NetworkInterface iface;
        iface.name = ifa->ifa_name;

        // Get IP address
        struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
        iface.ipAddress = inet_ntoa(addr->sin_addr);

        // Get netmask
        addr = (struct sockaddr_in*)ifa->ifa_netmask;
        iface.netmask = inet_ntoa(addr->sin_addr);

        // Get broadcast address if available
        if (ifa->ifa_flags & IFF_BROADCAST) {
            addr = (struct sockaddr_in*)ifa->ifa_broadaddr;
            iface.broadcast = inet_ntoa(addr->sin_addr);
        }

        // Get interface flags
        iface.isUp = (ifa->ifa_flags & IFF_UP) != 0;
        iface.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;

        // Check if wireless
        iface.isWireless = isWirelessInterface(iface.name);

        // Get MAC address
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, iface.name.c_str(), IFNAMSIZ - 1);

        if (ioctl(sock, SIOCGIFHWADDR, &ifr) >= 0) {
            unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
            std::stringstream macStream;
            macStream << std::hex << std::setfill('0');
            for (int i = 0; i < 6; ++i) {
                macStream << std::setw(2) << static_cast<int>(mac[i]);
                if (i < 5) macStream << ":";
            }
            iface.macAddress = macStream.str();
        }

        // Get statistics from /proc/net/dev
        std::ifstream netdev("/proc/net/dev");
        std::string line;
        while (std::getline(netdev, line)) {
            std::istringstream iss(line);
            std::string ifname;
            iss >> ifname;

            // Remove colon from interface name
            if (!ifname.empty() && ifname.back() == ':') {
                ifname.pop_back();
            }

            if (ifname == iface.name) {
                iss >> iface.rxBytes >> iface.rxPackets >> iface.rxErrors;
                // Skip some fields
                uint64_t dummy;
                iss >> dummy >> dummy >> dummy >> dummy >> dummy;
                iss >> iface.txBytes >> iface.txPackets >> iface.txErrors;
                break;
            }
        }

        interfaces.push_back(iface);
    }

    close(sock);
    freeifaddrs(ifaddr);
}

bool NetworkInfoCollector::isWirelessInterface(const std::string& name) const {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return false;
    }

    struct iwreq pwrq;
    memset(&pwrq, 0, sizeof(pwrq));
    strncpy(pwrq.ifr_name, name.c_str(), IFNAMSIZ - 1);

    bool isWireless = (ioctl(sock, SIOCGIWNAME, &pwrq) >= 0);

    close(sock);
    return isWireless;
}

std::string NetworkInfoCollector::formatBytes(uint64_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double formattedSize = static_cast<double>(bytes);

    while (formattedSize >= 1024.0 && unitIndex < 4) {
        formattedSize /= 1024.0;
        unitIndex++;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << formattedSize << " " << units[unitIndex];
    return ss.str();
}

void NetworkInfoCollector::printAllInfo() const {
    std::cout << "Network Interface Information:" << std::endl;
    std::cout << "----------------------------" << std::endl;

    for (const auto& iface : interfaces) {
        std::cout << "Interface: " << iface.name;
        std::cout << " [" << (iface.isUp ? "UP" : "DOWN") << "]";
        if (iface.isLoopback) std::cout << " [LOOPBACK]";
        if (iface.isWireless) std::cout << " [WIRELESS]";
        std::cout << std::endl;

        std::cout << "MAC Address: " << iface.macAddress << std::endl;
        std::cout << "IP Address: " << iface.ipAddress << std::endl;
        std::cout << "Netmask: " << iface.netmask << std::endl;
        if (!iface.broadcast.empty()) {
            std::cout << "Broadcast: " << iface.broadcast << std::endl;
        }

        std::cout << "RX: " << formatBytes(iface.rxBytes)
                  << " (" << iface.rxPackets << " packets, "
                  << iface.rxErrors << " errors)" << std::endl;
        std::cout << "TX: " << formatBytes(iface.txBytes)
                  << " (" << iface.txPackets << " packets, "
                  << iface.txErrors << " errors)" << std::endl;

        std::cout << std::endl;
    }
}