#include "main_window.h"
#include "cpu_info_widget.h"
#include "disk_window.h"
#include "network_window.h"
#include "service_window.h"
#include "usb_window.h"
#include "memory_window.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>
#include <QIcon>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>
#include <QSettings>
#include <QDebug>
#include "theme_manager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      settings("SystemInfo", "SystemInfo") // Organization name, application name
{
    qDebug() << "Initializing MainWindow...";

    // First, explicitly disable native menu bar
    menuBar()->setNativeMenuBar(false);
    menuBar()->setVisible(true);

    setWindowTitle("System Information");

    // Set a reasonable default size
    resize(800, 600);

    try {
        // Create menus first, then UI
        qDebug() << "Creating menus...";
        createMenus();

        qDebug() << "Setting up UI...";
        setupUI();

        // Read saved settings (position, size, etc.)
        qDebug() << "Reading settings...";
        readSettings();

        // Load theme from settings
        qDebug() << "Loading theme...";
        ThemeManager::instance().loadTheme();

        qDebug() << "MainWindow initialization complete.";
    } catch (const std::exception& e) {
        qCritical() << "Exception during MainWindow initialization:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception during MainWindow initialization";
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    qDebug() << "Settings saved on close";
    event->accept();
}

void MainWindow::readSettings()
{
    qDebug() << "Reading settings from:" << settings.fileName();

    // Read window position and size
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        qDebug() << "No saved geometry found, centering window";
        // If no saved position, center the window
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    } else {
        qDebug() << "Restoring geometry from settings";
        restoreGeometry(geometry);
    }

    // Read last active section
    int lastSection = settings.value("lastSection", 1).toInt(); // Default to disk info
    qDebug() << "Last active section:" << lastSection;

    switch (lastSection) {
        case 0:
            showCPUInfo();
            break;
        case 1:
            showDiskInfo();
            break;
        case 2:
            showNetworkInfo();
            break;
        case 3:
            showServiceInfo();
            break;
        case 4:
            showUSBInfo();
            break;
        case 5:
            showMemoryInfo();
            break;
        default:
            showDiskInfo();
            break;
    }
}

void MainWindow::writeSettings()
{
    // Save window position and size
    settings.setValue("geometry", saveGeometry());

    // Save which section was active
    int currentSection = 1; // Default to disk info
    if (stackedWidget->currentWidget() == cpuWidget) {
        currentSection = 0;
    } else if (stackedWidget->currentWidget() == diskWidget) {
        currentSection = 1;
    } else if (stackedWidget->currentWidget() == networkWidget) {
        currentSection = 2;
    } else if (stackedWidget->currentWidget() == serviceWidget) {
        currentSection = 3;
    } else if (stackedWidget->currentWidget() == usbWidget) {
        currentSection = 4;
    } else if (stackedWidget->currentWidget() == memoryWidget) {
        currentSection = 5;
    }
    settings.setValue("lastSection", currentSection);

    // Force settings to be written to disk
    settings.sync();

    qDebug() << "Settings written: geometry and lastSection =" << currentSection;
}

void MainWindow::setupUI() {
    qDebug() << "Creating central widget...";

    // Create a central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Create a custom menu bar as a widget with a distinctive appearance
    qDebug() << "Creating menu widget...";
    QWidget *menuWidget = new QWidget(this);
    menuWidget->setObjectName("mainMenuWidget");
    menuWidget->setProperty("isMenuWidget", true);
    QHBoxLayout *menuLayout = new QHBoxLayout(menuWidget);

    // Create buttons for each section
    qDebug() << "Creating navigation buttons...";
    QPushButton *cpuButton = new QPushButton(QIcon::fromTheme("cpu", QIcon(":/icons/cpu.png")), "CPU Information", this);
    QPushButton *diskButton = new QPushButton(QIcon::fromTheme("drive-harddisk", QIcon(":/icons/disk.png")), "Hard Drive Information", this);
    QPushButton *networkButton = new QPushButton(QIcon::fromTheme("network-wired", QIcon(":/icons/network.png")), "Network Information", this);
    QPushButton *serviceButton = new QPushButton(QIcon::fromTheme("system-run", QIcon(":/icons/service.png")), "System Services", this);
    QPushButton *usbButton = new QPushButton(QIcon::fromTheme("drive-removable-media", QIcon(":/icons/usb.png")), "USB Devices", this);

    // Add buttons to menu layout
    menuLayout->addWidget(cpuButton);
    menuLayout->addWidget(diskButton);
    menuLayout->addWidget(networkButton);
    menuLayout->addWidget(serviceButton);
    menuLayout->addWidget(usbButton);
    menuLayout->addStretch();

    mainLayout->addWidget(menuWidget);

    // Create a stacked widget to switch between views
    qDebug() << "Creating stacked widget...";
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName("mainStackedWidget");
    mainLayout->addWidget(stackedWidget);

    // Create the different views with error checking
    try {
        qDebug() << "Creating CPU widget...";
        cpuWidget = new CPUInfoWidget(this);

        qDebug() << "Creating disk widget...";
        diskWidget = new DiskWindow(this);

        qDebug() << "Creating network widget...";
        networkWidget = new NetworkWindow(this);

        qDebug() << "Creating service widget...";
        serviceWidget = new ServiceWindow(this);

        qDebug() << "Creating USB widget...";
        usbWidget = new USBWindow(this);

        qDebug() << "Creating memory widget...";
        memoryWidget = new MemoryWindow(this);
    } catch (const std::exception& e) {
        qCritical() << "Exception during widget creation:" << e.what();
        throw;
    } catch (...) {
        qCritical() << "Unknown exception during widget creation";
        throw;
    }

    // Add views to stacked widget
    qDebug() << "Adding widgets to stacked widget...";
    stackedWidget->addWidget(cpuWidget);
    stackedWidget->addWidget(diskWidget);
    stackedWidget->addWidget(networkWidget);
    stackedWidget->addWidget(serviceWidget);
    stackedWidget->addWidget(usbWidget);
    stackedWidget->addWidget(memoryWidget);

    // Connect button signals
    qDebug() << "Connecting button signals...";
    connect(cpuButton, &QPushButton::clicked, this, &MainWindow::showCPUInfo);
    connect(diskButton, &QPushButton::clicked, this, &MainWindow::showDiskInfo);
    connect(networkButton, &QPushButton::clicked, this, &MainWindow::showNetworkInfo);
    connect(serviceButton, &QPushButton::clicked, this, &MainWindow::showServiceInfo);
    connect(usbButton, &QPushButton::clicked, this, &MainWindow::showUSBInfo);

    // Make buttons checkable
    cpuButton->setCheckable(true);
    diskButton->setCheckable(true);
    networkButton->setCheckable(true);
    serviceButton->setCheckable(true);
    usbButton->setCheckable(true);

    // Add a button for memory info
    QPushButton *memoryButton = new QPushButton(QIcon::fromTheme("media-memory", QIcon(":/icons/memory.png")), "Memory Information", this);
    menuLayout->addWidget(memoryButton, 2, 0); // Add after CPU button

    // Connect the button
    connect(memoryButton, &QPushButton::clicked, this, &MainWindow::showMemoryInfo);

    // Make the button checkable
    memoryButton->setCheckable(true);

    qDebug() << "UI setup complete.";
}

void MainWindow::createMenus() {
    // Initialize all actions first
    cpuInfoAction = new QAction(tr("&CPU Information"), this);
    diskInfoAction = new QAction(tr("&Disk Information"), this);
    networkInfoAction = new QAction(tr("&Network Information"), this);
    serviceInfoAction = new QAction(tr("&System Services"), this);
    usbInfoAction = new QAction(tr("&USB Devices"), this);
    exitAction = new QAction(tr("E&xit"), this);
    aboutAction = new QAction(tr("&About"), this);
    toggleThemeAction = new QAction(tr("Toggle Dark/Light Mode"), this);

    // Create View menu
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    // CPU Info action
    cpuInfoAction->setStatusTip(tr("Show CPU information"));
    connect(cpuInfoAction, &QAction::triggered, this, &MainWindow::showCPUInfo);
    viewMenu->addAction(cpuInfoAction);

    // Disk Info action
    diskInfoAction->setStatusTip(tr("Show hard drive information"));
    connect(diskInfoAction, &QAction::triggered, this, &MainWindow::showDiskInfo);
    viewMenu->addAction(diskInfoAction);

    // Network Info action
    networkInfoAction->setStatusTip(tr("Show network information"));
    connect(networkInfoAction, &QAction::triggered, this, &MainWindow::showNetworkInfo);
    viewMenu->addAction(networkInfoAction);

    // Service Info action
    serviceInfoAction->setStatusTip(tr("Show system services"));
    connect(serviceInfoAction, &QAction::triggered, this, &MainWindow::showServiceInfo);
    viewMenu->addAction(serviceInfoAction);

    // Add USB info action
    usbInfoAction->setStatusTip(tr("Show USB devices"));
    connect(usbInfoAction, &QAction::triggered, this, &MainWindow::showUSBInfo);
    viewMenu->addAction(usbInfoAction);

    viewMenu->addSeparator();

    // Exit action
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    viewMenu->addAction(exitAction);

    // Create Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    // About action
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAction);

    // Create status bar
    statusBar()->showMessage("Ready");

    // Create a toolbar but make it less prominent
    QToolBar *toolBar = addToolBar(tr("Navigation"));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setVisible(false);  // Hide it by default since we have the button menu

    // Add theme toggle to the toolbar
    toggleThemeAction->setIcon(QIcon::fromTheme("preferences-desktop-theme",
        QIcon(ThemeManager::instance().currentTheme() == ThemeManager::DarkTheme ?
            ":/icons/light_mode.png" : ":/icons/dark_mode.png")));
    toggleThemeAction->setStatusTip(tr("Switch between dark and light mode"));
    connect(toggleThemeAction, &QAction::triggered, this, &MainWindow::toggleTheme);

    // Create a toolbar but make it visible now
    QToolBar *themeToolBar = addToolBar(tr("Theme"));
    themeToolBar->setMovable(false);
    themeToolBar->setFloatable(false);
    themeToolBar->setVisible(true);
    themeToolBar->addAction(toggleThemeAction);

    // Add theme toggle to the view menu as well
    viewMenu->addAction(toggleThemeAction);
}

void MainWindow::showCPUInfo() {
    stackedWidget->setCurrentWidget(cpuWidget);
    setWindowTitle("System Information - CPU Information");
    cpuInfoAction->setEnabled(false);
    diskInfoAction->setEnabled(true);
    networkInfoAction->setEnabled(true);
    serviceInfoAction->setEnabled(true);
    usbInfoAction->setEnabled(true);
}

void MainWindow::showDiskInfo() {
    stackedWidget->setCurrentWidget(diskWidget);
    setWindowTitle("System Information - Hard Drive Information");
    cpuInfoAction->setEnabled(true);
    diskInfoAction->setEnabled(false);
    networkInfoAction->setEnabled(true);
    serviceInfoAction->setEnabled(true);
    usbInfoAction->setEnabled(true);
}

void MainWindow::showNetworkInfo() {
    stackedWidget->setCurrentWidget(networkWidget);
    setWindowTitle("System Information - Network Interfaces");
    cpuInfoAction->setEnabled(true);
    diskInfoAction->setEnabled(true);
    networkInfoAction->setEnabled(false);
    serviceInfoAction->setEnabled(true);
    usbInfoAction->setEnabled(true);
}

void MainWindow::showServiceInfo() {
    stackedWidget->setCurrentWidget(serviceWidget);
    setWindowTitle("System Information - System Services");
    cpuInfoAction->setEnabled(true);
    diskInfoAction->setEnabled(true);
    networkInfoAction->setEnabled(true);
    serviceInfoAction->setEnabled(false);
    usbInfoAction->setEnabled(true);
}

void MainWindow::showUSBInfo() {
    stackedWidget->setCurrentWidget(usbWidget);
    setWindowTitle("System Information - USB Devices");
    cpuInfoAction->setEnabled(true);
    diskInfoAction->setEnabled(true);
    networkInfoAction->setEnabled(true);
    serviceInfoAction->setEnabled(true);
    usbInfoAction->setEnabled(false);
}

void MainWindow::showMemoryInfo() {
    stackedWidget->setCurrentWidget(memoryWidget);
    setWindowTitle("System Information - Memory Information");
    cpuInfoAction->setEnabled(true);
    diskInfoAction->setEnabled(true);
    networkInfoAction->setEnabled(true);
    serviceInfoAction->setEnabled(true);
    usbInfoAction->setEnabled(true);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About System Info"),
        tr("<h2>System Info</h2>"
           "<p>Version 1.0</p>"
           "<p>A simple application to display system information "
           "including CPU details and hard drive statistics.</p>"));
}

void MainWindow::toggleTheme() {
    ThemeManager::instance().toggleTheme();

    // Update the icon based on the new theme
    if (ThemeManager::instance().currentTheme() == ThemeManager::DarkTheme) {
        toggleThemeAction->setIcon(QIcon::fromTheme("weather-clear", QIcon(":/icons/light_mode.png")));

        // Force update of all widgets to ensure dark mode is applied
        for (QWidget* widget : QApplication::allWidgets()) {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    } else {
        toggleThemeAction->setIcon(QIcon::fromTheme("weather-clear-night", QIcon(":/icons/dark_mode.png")));

        // Force update of all widgets to ensure light mode is applied
        for (QWidget* widget : QApplication::allWidgets()) {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    }
}