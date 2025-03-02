#include "simple_menu_window.h"
#include <QApplication>
#include <QStyleFactory>
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        // Set environment variables for Linux to ensure menus are shown
        #if defined(__linux__)
            qputenv("QT_QPA_PLATFORMTHEME", "gtk2");
            qputenv("QT_STYLE_OVERRIDE", "fusion");
            qputenv("QT_QPA_PLATFORM", "xcb");
        #endif

        // Set these before creating QApplication
        QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);

        QApplication app(argc, argv);
        app.setApplicationName("Simple Menu Example");
        app.setApplicationVersion("1.0");

        // Set application style
        app.setStyle(QStyleFactory::create("Fusion"));

        SimpleMenuWindow mainWindow;
        mainWindow.show();

        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}