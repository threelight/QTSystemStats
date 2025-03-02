#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QCloseEvent>
#include "cpu_info.h"
#include "disk_info.h"
#include "network_info.h"
#include "service_info.h"
#include "theme_manager.h"
#include "usb_window.h"
#include "memory_window.h"

class CPUInfoWidget;  // Forward declaration
class DiskWindow;     // Forward declaration
class NetworkWindow;  // Forward declaration
class ServiceWindow;  // Forward declaration
class USBWindow;      // Forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showCPUInfo();
    void showDiskInfo();
    void showNetworkInfo();
    void showServiceInfo();
    void showUSBInfo();
    void showMemoryInfo();
    void about();
    void toggleTheme();

private:
    void setupUI();
    void createMenus();
    void readSettings();
    void writeSettings();

    QStackedWidget *stackedWidget;
    CPUInfoWidget *cpuWidget;
    DiskWindow *diskWidget;
    NetworkWindow *networkWidget;
    ServiceWindow *serviceWidget;
    USBWindow *usbWidget;
    MemoryWindow *memoryWidget;

    // Menu actions
    QAction *cpuInfoAction;
    QAction *diskInfoAction;
    QAction *networkInfoAction;
    QAction *serviceInfoAction;
    QAction *usbInfoAction;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *toggleThemeAction;

    // Settings
    QSettings settings;
};

#endif // MAIN_WINDOW_H