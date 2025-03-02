#include "usb_window.h"
#include <QHeaderView>
#include <QDateTime>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QApplication>

USBWindow::USBWindow(QWidget *parent) : QWidget(parent) {
    setupUI();
    updateTable();

    // Set up auto-refresh timer (every 10 seconds)
    autoRefreshTimer = new QTimer(this);
    connect(autoRefreshTimer, &QTimer::timeout, this, &USBWindow::refreshData);
    autoRefreshTimer->start(10000);
}

void USBWindow::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QFont titleFont = font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    // Create header
    QLabel *headerLabel = new QLabel("USB Devices", this);
    headerLabel->setFont(titleFont);
    layout->addWidget(headerLabel);

    // Create table for USB devices
    deviceTable = new QTableWidget(this);
    deviceTable->setObjectName("usbTable");
    deviceTable->setColumnCount(8);
    deviceTable->setHorizontalHeaderLabels({
        "Device", "Manufacturer", "Product", "Serial Number",
        "Vendor ID", "Product ID", "USB Version", "Status"
    });
    deviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    deviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    deviceTable->setAlternatingRowColors(true);
    layout->addWidget(deviceTable);

    // Create controls at the bottom
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // Auto-refresh checkbox
    QCheckBox *autoRefreshCheckbox = new QCheckBox("Auto-refresh (10s)", this);
    autoRefreshCheckbox->setChecked(true);
    connect(autoRefreshCheckbox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            autoRefreshTimer->start(10000);
        } else {
            autoRefreshTimer->stop();
        }
    });
    controlLayout->addWidget(autoRefreshCheckbox);

    controlLayout->addStretch();

    // Last update label
    lastUpdateLabel = new QLabel(this);
    controlLayout->addWidget(lastUpdateLabel);

    // Refresh button
    refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &USBWindow::refreshData);
    controlLayout->addWidget(refreshButton);

    layout->addLayout(controlLayout);

    // Add detailed information section
    QGroupBox *detailsGroup = new QGroupBox("Device Details", this);
    detailsGroup->setCheckable(true);
    detailsGroup->setChecked(false);

    detailsLayout = new QVBoxLayout(detailsGroup);
    detailsLabel = new QLabel("Select a device to view details", this);
    detailsLayout->addWidget(detailsLabel);

    layout->addWidget(detailsGroup);

    // Connect selection signal
    connect(deviceTable, &QTableWidget::itemSelectionChanged, [this]() {
        if (deviceTable->selectedItems().isEmpty()) {
            detailsLabel->setText("Select a device to view details");
            return;
        }

        int row = deviceTable->selectedItems().first()->row();
        QString deviceName = deviceTable->item(row, 0)->text();

        // Get the selected device details
        for (const auto& device : getUSBDevices()) {
            if (QString::fromStdString(device.deviceName) == deviceName) {
                detailsLabel->setText(QString(
                    "Device Details:\n"
                    "Name: %1\n"
                    "Manufacturer: %2\n"
                    "Product: %3\n"
                    "Serial Number: %4\n"
                    "Vendor ID: %5\n"
                    "Product ID: %6\n"
                    "Bus Number: %7\n"
                    "Device Address: %8\n"
                    "Speed: %9\n"
                    "USB Version: %10\n"
                    "Status: %11"
                ).arg(QString::fromStdString(device.deviceName))
                 .arg(QString::fromStdString(device.manufacturer))
                 .arg(QString::fromStdString(device.product))
                 .arg(QString::fromStdString(device.serialNumber))
                 .arg(QString::fromStdString(device.vendorID))
                 .arg(QString::fromStdString(device.productID))
                 .arg(QString::fromStdString(device.busNumber))
                 .arg(QString::fromStdString(device.deviceAddress))
                 .arg(QString::fromStdString(device.speed))
                 .arg(QString::fromStdString(device.usbVersion))
                 .arg(device.isConnected ? "Connected" : "Disconnected"));
                break;
            }
        }
    });
}

void USBWindow::refreshData() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    updateTable();
    QApplication::restoreOverrideCursor();
}

void USBWindow::updateTable() {
    std::vector<USBDeviceInfo> devices = getUSBDevices();

    deviceTable->setRowCount(devices.size());

    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& device = devices[i];

        // Device name
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(device.deviceName));
        deviceTable->setItem(i, 0, nameItem);

        // Manufacturer
        QTableWidgetItem *manufacturerItem = new QTableWidgetItem(QString::fromStdString(device.manufacturer));
        deviceTable->setItem(i, 1, manufacturerItem);

        // Product
        QTableWidgetItem *productItem = new QTableWidgetItem(QString::fromStdString(device.product));
        deviceTable->setItem(i, 2, productItem);

        // Serial Number
        QTableWidgetItem *serialItem = new QTableWidgetItem(QString::fromStdString(device.serialNumber));
        deviceTable->setItem(i, 3, serialItem);

        // Vendor ID
        QTableWidgetItem *vendorItem = new QTableWidgetItem(QString::fromStdString(device.vendorID));
        deviceTable->setItem(i, 4, vendorItem);

        // Product ID
        QTableWidgetItem *productIDItem = new QTableWidgetItem(QString::fromStdString(device.productID));
        deviceTable->setItem(i, 5, productIDItem);

        // USB Version
        QTableWidgetItem *versionItem = new QTableWidgetItem(QString::fromStdString(device.usbVersion));
        deviceTable->setItem(i, 6, versionItem);

        // Status
        QTableWidgetItem *statusItem = new QTableWidgetItem(device.isConnected ? "Connected" : "Disconnected");
        statusItem->setData(Qt::UserRole + 1, device.isConnected ? "connected" : "disconnected");
        statusItem->setForeground(device.isConnected ? Qt::green : Qt::red);
        deviceTable->setItem(i, 7, statusItem);
    }

    // Update last refresh time
    QDateTime currentTime = QDateTime::currentDateTime();
    lastUpdateLabel->setText("Last updated: " + currentTime.toString("hh:mm:ss"));
}

std::vector<USBDeviceInfo> USBWindow::getUSBDevices() {
    std::vector<USBDeviceInfo> devices;

    // Method 1: Use lsusb command
    QProcess process;
    process.start("lsusb", QStringList() << "-v");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n');

    USBDeviceInfo currentDevice;
    bool inDeviceBlock = false;

    for (const QString& line : lines) {
        if (line.startsWith("Bus ")) {
            // Start of a new device
            if (inDeviceBlock) {
                // Save previous device
                devices.push_back(currentDevice);
            }

            // Reset for new device
            currentDevice = USBDeviceInfo();
            currentDevice.isConnected = true;
            inDeviceBlock = true;

            // Parse bus and device info
            QRegularExpression busDevRegex("Bus (\\d+) Device (\\d+): ID ([0-9a-fA-F]+):([0-9a-fA-F]+)");
            QRegularExpressionMatch match = busDevRegex.match(line);

            if (match.hasMatch()) {
                currentDevice.busNumber = match.captured(1).toStdString();
                currentDevice.deviceAddress = match.captured(2).toStdString();
                currentDevice.vendorID = match.captured(3).toStdString();
                currentDevice.productID = match.captured(4).toStdString();
                currentDevice.deviceName = "USB Device " + currentDevice.vendorID + ":" + currentDevice.productID;
            }
        } else if (line.contains("idVendor")) {
            QRegularExpression vendorRegex("idVendor\\s+0x[0-9a-fA-F]+\\s+(.*)");
            QRegularExpressionMatch match = vendorRegex.match(line);
            if (match.hasMatch()) {
                currentDevice.manufacturer = match.captured(1).trimmed().toStdString();
            }
        } else if (line.contains("idProduct")) {
            QRegularExpression productRegex("idProduct\\s+0x[0-9a-fA-F]+\\s+(.*)");
            QRegularExpressionMatch match = productRegex.match(line);
            if (match.hasMatch()) {
                currentDevice.product = match.captured(1).trimmed().toStdString();
            }
        } else if (line.contains("iSerial")) {
            QRegularExpression serialRegex("iSerial\\s+\\d+\\s+(.*)");
            QRegularExpressionMatch match = serialRegex.match(line);
            if (match.hasMatch()) {
                currentDevice.serialNumber = match.captured(1).trimmed().toStdString();
            }
        } else if (line.contains("bcdUSB")) {
            QRegularExpression usbVersionRegex("bcdUSB\\s+([0-9.]+)");
            QRegularExpressionMatch match = usbVersionRegex.match(line);
            if (match.hasMatch()) {
                currentDevice.usbVersion = match.captured(1).toStdString();
            }
        } else if (line.contains("bMaxPacketSize0")) {
            QRegularExpression speedRegex("bMaxPacketSize0\\s+(\\d+)");
            QRegularExpressionMatch match = speedRegex.match(line);
            if (match.hasMatch()) {
                int packetSize = match.captured(1).toInt();
                if (packetSize == 64) {
                    currentDevice.speed = "High Speed";
                } else if (packetSize == 8) {
                    currentDevice.speed = "Low Speed";
                } else if (packetSize == 512) {
                    currentDevice.speed = "Super Speed";
                } else {
                    currentDevice.speed = "Full Speed";
                }
            }
        }
    }

    // Add the last device if there was one
    if (inDeviceBlock) {
        devices.push_back(currentDevice);
    }

    // Method 2: Check /sys/bus/usb/devices for more detailed info
    QDir usbDir("/sys/bus/usb/devices");
    QStringList usbDevices = usbDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString& deviceDir : usbDevices) {
        // Only process directories that look like USB devices (e.g., 1-1, 2-3.4)
        if (deviceDir.contains('-')) {
            QString fullPath = "/sys/bus/usb/devices/" + deviceDir;

            // Check if this is a USB device (has idVendor file)
            QFile vendorFile(fullPath + "/idVendor");
            if (!vendorFile.exists()) continue;

            USBDeviceInfo sysDevice;
            sysDevice.isConnected = true;
            sysDevice.deviceName = deviceDir.toStdString();

            // Read vendor ID
            if (vendorFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.vendorID = vendorFile.readAll().trimmed().toStdString();
                vendorFile.close();
            }

            // Read product ID
            QFile productFile(fullPath + "/idProduct");
            if (productFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.productID = productFile.readAll().trimmed().toStdString();
                productFile.close();
            }

            // Read manufacturer
            QFile manufacturerFile(fullPath + "/manufacturer");
            if (manufacturerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.manufacturer = manufacturerFile.readAll().trimmed().toStdString();
                manufacturerFile.close();
            }

            // Read product
            QFile productNameFile(fullPath + "/product");
            if (productNameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.product = productNameFile.readAll().trimmed().toStdString();
                productNameFile.close();
            }

            // Read serial
            QFile serialFile(fullPath + "/serial");
            if (serialFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.serialNumber = serialFile.readAll().trimmed().toStdString();
                serialFile.close();
            }

            // Read speed
            QFile speedFile(fullPath + "/speed");
            if (speedFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString speedValue = speedFile.readAll().trimmed();
                sysDevice.speed = speedValue.toStdString() + " Mbps";
                speedFile.close();
            }

            // Read version
            QFile versionFile(fullPath + "/version");
            if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.usbVersion = versionFile.readAll().trimmed().toStdString();
                versionFile.close();
            }

            // Read bus number
            QFile busFile(fullPath + "/busnum");
            if (busFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.busNumber = busFile.readAll().trimmed().toStdString();
                busFile.close();
            }

            // Read device address
            QFile devnumFile(fullPath + "/devnum");
            if (devnumFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                sysDevice.deviceAddress = devnumFile.readAll().trimmed().toStdString();
                devnumFile.close();
            }

            // Only add if we have at least vendor and product IDs
            if (!sysDevice.vendorID.empty() && !sysDevice.productID.empty()) {
                // Check if we already have this device from lsusb
                bool found = false;
                for (auto& device : devices) {
                    if (device.vendorID == sysDevice.vendorID &&
                        device.productID == sysDevice.productID &&
                        device.busNumber == sysDevice.busNumber &&
                        device.deviceAddress == sysDevice.deviceAddress) {

                        // Update with any additional info from sysfs
                        if (device.manufacturer.empty()) device.manufacturer = sysDevice.manufacturer;
                        if (device.product.empty()) device.product = sysDevice.product;
                        if (device.serialNumber.empty()) device.serialNumber = sysDevice.serialNumber;
                        if (device.speed.empty()) device.speed = sysDevice.speed;
                        if (device.usbVersion.empty()) device.usbVersion = sysDevice.usbVersion;

                        found = true;
                        break;
                    }
                }

                if (!found) {
                    devices.push_back(sysDevice);
                }
            }
        }
    }

    return devices;
}