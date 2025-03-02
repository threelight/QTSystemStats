#include "service_info.h"
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <stdexcept>
#include <regex>
#include <cstdio>
#include <unistd.h>
#include <QDebug>

ServiceInfoCollector::ServiceInfoCollector() {
    qDebug() << "Initializing ServiceInfoCollector...";
    try {
        refresh();
        qDebug() << "ServiceInfoCollector initialized successfully.";
    } catch (const std::exception& e) {
        qCritical() << "Exception during ServiceInfoCollector initialization:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception during ServiceInfoCollector initialization";
    }
}

std::vector<ServiceInfo> ServiceInfoCollector::getAllServices() {
    return services;
}

ServiceInfo ServiceInfoCollector::getServiceDetails(const std::string& serviceName) {
    // Execute systemctl show command to get detailed information
    std::string command = "systemctl show " + serviceName + " --property=Description,LoadState,ActiveState,SubState,UnitFileState";
    std::string output = executeCommand(command);

    ServiceInfo info;
    info.name = serviceName;

    std::istringstream iss(output);
    std::string line;

    while (std::getline(iss, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "Description") {
                info.description = value;
            } else if (key == "LoadState") {
                info.loadState = value;
            } else if (key == "ActiveState") {
                info.activeState = value;
                info.status = getStatusFromActiveState(value);
            } else if (key == "SubState") {
                info.subState = value;
            } else if (key == "UnitFileState") {
                info.unitFileState = value;
            }
        }
    }

    return info;
}

bool ServiceInfoCollector::startService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "start");
}

bool ServiceInfoCollector::stopService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "stop");
}

bool ServiceInfoCollector::restartService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "restart");
}

bool ServiceInfoCollector::reloadService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "reload");
}

bool ServiceInfoCollector::enableService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "enable");
}

bool ServiceInfoCollector::disableService(const std::string& serviceName) {
    return executeServiceCommand(serviceName, "disable");
}

void ServiceInfoCollector::refresh() {
    services.clear();
    collectServiceInfo();
}

void ServiceInfoCollector::collectServiceInfo() {
    // Get list of all services
    std::string output = executeCommand("systemctl list-units --type=service --all --no-legend");

    std::istringstream iss(output);
    std::string line;

    while (std::getline(iss, line)) {
        // Parse service information
        std::regex serviceRegex(R"((\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(.*))");
        std::smatch matches;

        if (std::regex_search(line, matches, serviceRegex) && matches.size() > 5) {
            ServiceInfo service;
            service.name = matches[1].str();
            service.loadState = matches[2].str();
            service.activeState = matches[3].str();
            service.subState = matches[4].str();
            service.description = matches[5].str();
            service.status = getStatusFromActiveState(service.activeState);

            services.push_back(service);
        }
    }
}

std::string ServiceInfoCollector::executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

ServiceStatus ServiceInfoCollector::getStatusFromActiveState(const std::string& activeState) {
    if (activeState == "active") {
        return ServiceStatus::ACTIVE;
    } else if (activeState == "inactive") {
        return ServiceStatus::INACTIVE;
    } else if (activeState == "failed") {
        return ServiceStatus::FAILED;
    } else {
        return ServiceStatus::UNKNOWN;
    }
}

bool ServiceInfoCollector::executeServiceCommand(const std::string& serviceName, const std::string& command) {
    // Check if we're running as root
    if (geteuid() != 0) {
        // If not root, use pkexec to get elevated privileges
        std::string fullCommand = "pkexec systemctl " + command + " " + serviceName;
        int result = system(fullCommand.c_str());
        return result == 0;
    } else {
        // If already root, execute directly
        std::string fullCommand = "systemctl " + command + " " + serviceName;
        int result = system(fullCommand.c_str());
        return result == 0;
    }
}

void ServiceInfoCollector::printAllInfo() const {
    std::cout << "System Services Information:" << std::endl;
    std::cout << "----------------------------" << std::endl;

    for (const auto& service : services) {
        std::cout << "Service: " << service.name << std::endl;
        std::cout << "Description: " << service.description << std::endl;
        std::cout << "Status: " << service.activeState << " (" << service.subState << ")" << std::endl;
        std::cout << "Load State: " << service.loadState << std::endl;
        std::cout << std::endl;
    }
}