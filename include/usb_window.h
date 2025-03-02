#ifndef USB_WINDOW_H
#define USB_WINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <vector>
#include <string>

// Structure to hold USB device information
struct USBDeviceInfo {
    std::string deviceName;
    std::string manufacturer;
    std::string product;
    std::string serialNumber;
    std::string vendorID;
    std::string productID;
    std::string busNumber;
    std::string deviceAddress;
    std::string speed;
    std::string usbVersion;
    bool isConnected;
};

class USBWindow : public QWidget {
    Q_OBJECT

public:
    explicit USBWindow(QWidget *parent = nullptr);

private slots:
    void refreshData();
    void updateTable();

private:
    void setupUI();
    std::vector<USBDeviceInfo> getUSBDevices();

    QTableWidget *deviceTable;
    QLabel *lastUpdateLabel;
    QPushButton *refreshButton;
    QTimer *autoRefreshTimer;
    QVBoxLayout *detailsLayout;
    QLabel *detailsLabel;
};

#endif // USB_WINDOW_H