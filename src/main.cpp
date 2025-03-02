#include "cpu_info.h"
#include "disk_info.h"
#include "network_info.h"
#include "service_info.h"
#include <iostream>

// Check if Qt is available at compile time
#if defined(USE_QT_GUI)
#include "main_window.h"
#include <QApplication>
#include <QStyleFactory>
#include <QSettings>
#endif

int main(int argc, char *argv[]) {
    try {
        #if defined(USE_QT_GUI)
            // Set environment variables for Linux to ensure menus are shown
            #if defined(__linux__)
                qputenv("QT_QPA_PLATFORMTHEME", "gtk2");  // Try gtk2 instead of generic
                qputenv("QT_STYLE_OVERRIDE", "fusion");   // Force fusion style
                qputenv("QT_QPA_PLATFORM", "xcb");        // Use X11 backend
            #endif

            // Set these before creating QApplication
            QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);

            QApplication app(argc, argv);
            app.setApplicationName("SystemInfo");
            app.setOrganizationName("SystemInfo");
            app.setApplicationVersion("1.0");

            // Add this to ensure settings are properly initialized
            QSettings::setDefaultFormat(QSettings::NativeFormat);

            // Set application style
            app.setStyle(QStyleFactory::create("Fusion"));

            // Initialize theme manager (it will load the theme from settings)
            ThemeManager::instance().loadTheme();

            MainWindow mainWindow;
            mainWindow.show();
            return app.exec();
        #else
            // Fallback to console mode
            std::cout << "System Info v1.0" << std::endl;
            std::cout << "================" << std::endl << std::endl;

            CPUInfo cpuInfo;
            cpuInfo.printAllInfo();

            std::cout << "\n\n";

            DiskInfoCollector diskInfo;
            diskInfo.printAllInfo();

            std::cout << "\n\n";

            NetworkInfoCollector networkInfo;
            networkInfo.printAllInfo();

            std::cout << "\n\n";

            ServiceInfoCollector serviceInfo;
            serviceInfo.printAllInfo();
            return 0;
        #endif
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}