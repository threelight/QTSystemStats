#include "theme_manager.h"
#include <QStyleFactory>
#include <QDebug>
#include <QStyle>
#include <QWidget>
#include <QApplication>

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent), m_currentTheme(LightTheme)
{
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme)
        return;

    m_currentTheme = theme;

    if (theme == DarkTheme) {
        // Apply dark theme
        qApp->setPalette(createDarkPalette());
        qApp->setStyleSheet(getStyleSheet(theme));

        // Force update on all widgets
        foreach (QWidget *widget, QApplication::allWidgets()) {
            widget->update();
        }
    } else {
        // Restore default light theme by using the default Fusion style
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(QApplication::style()->standardPalette());
        qApp->setStyleSheet("");  // Clear any custom stylesheet

        // Force update on all widgets
        foreach (QWidget *widget, QApplication::allWidgets()) {
            widget->update();
        }
    }

    emit themeChanged(theme);
    qDebug() << "Theme changed to:" << (theme == DarkTheme ? "Dark" : "Light");
}

void ThemeManager::toggleTheme()
{
    setTheme(m_currentTheme == DarkTheme ? LightTheme : DarkTheme);
    saveTheme();
}

void ThemeManager::loadTheme()
{
    QSettings settings("SystemInfo", "SystemInfo");
    bool isDarkTheme = settings.value("darkTheme", false).toBool();
    setTheme(isDarkTheme ? DarkTheme : LightTheme);
}

void ThemeManager::saveTheme()
{
    QSettings settings("SystemInfo", "SystemInfo");
    settings.setValue("darkTheme", m_currentTheme == DarkTheme);
    settings.sync();
}

QPalette ThemeManager::createLightPalette() const
{
    // Return the default palette for light theme
    return QApplication::style()->standardPalette();
}

QPalette ThemeManager::createDarkPalette() const
{
    QPalette darkPalette;

    // Dark theme colors
    QColor darkColor(45, 45, 45);
    QColor disabledColor(127, 127, 127);
    QColor textColor(210, 210, 210);
    QColor highlightColor(42, 130, 218);
    QColor linkColor(42, 130, 218);

    // Base
    darkPalette.setColor(QPalette::WindowText, textColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::Light, darkColor.lighter(150));
    darkPalette.setColor(QPalette::Midlight, darkColor.lighter(125));
    darkPalette.setColor(QPalette::Dark, darkColor.darker(200));
    darkPalette.setColor(QPalette::Mid, darkColor.darker(150));
    darkPalette.setColor(QPalette::Text, textColor);
    darkPalette.setColor(QPalette::BrightText, Qt::white);
    darkPalette.setColor(QPalette::ButtonText, textColor);
    darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::Shadow, Qt::black);
    darkPalette.setColor(QPalette::Highlight, highlightColor);
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Link, linkColor);
    darkPalette.setColor(QPalette::LinkVisited, linkColor.darker());

    // Disabled
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

    return darkPalette;
}

QString ThemeManager::getStyleSheet(Theme theme) const
{
    if (theme == DarkTheme) {
        return QString(
            // General UI elements
            "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"

            // Main application tabs (the ones at the very top)
            "QMainWindow > QWidget > QTabBar::tab { color: #FFFFFF; background-color: #353535; "
            "  padding: 10px 20px; margin: 2px; border: 1px solid #555555; border-bottom: none; font-weight: bold; }"
            "QMainWindow > QWidget > QTabBar::tab:selected { background-color: #4A4A4A; border-bottom: 3px solid #2a82da; }"
            "QMainWindow > QWidget > QTabBar::tab:!selected { background-color: #252525; }"
            "QMainWindow > QWidget > QTabBar::tab:hover { background-color: #454545; }"

            // Inner tabs (like in CPU Info)
            "QTabWidget::pane { border: 1px solid #3A3A3A; background-color: #2D2D2D; }"
            "QTabWidget QTabBar::tab { color: #DDDDDD; background-color: #353535; padding: 8px 12px; margin: 2px; border: 1px solid #555555; border-bottom: none; }"
            "QTabWidget QTabBar::tab:selected { background-color: #4A4A4A; border-bottom: 3px solid #2a82da; font-weight: bold; }"
            "QTabWidget QTabBar::tab:!selected { background-color: #252525; }"
            "QTabWidget QTabBar::tab:hover { background-color: #454545; }"

            // Tables
            "QTableView { background-color: #1E1E1E; color: #DDDDDD; gridline-color: #3A3A3A; "
            "  selection-background-color: #2A82DA; selection-color: white; alternate-background-color: #252525; }"
            "QTableView::item { padding: 5px; }"
            "QTableView::item:selected { background-color: #2A82DA; color: white; }"
            "QTableView::item:hover { background-color: #353535; }"

            // Headers
            "QHeaderView { background-color: #252525; }"
            "QHeaderView::section { background-color: #353535; color: #DDDDDD; padding: 6px; border: 1px solid #555555; font-weight: bold; }"
            "QHeaderView::section:hover { background-color: #454545; }"

            // Input elements
            "QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QComboBox, QDateEdit, QTimeEdit, QDateTimeEdit {"
            "  background-color: #1E1E1E; color: #DDDDDD; border: 1px solid #555555; padding: 4px; border-radius: 3px; }"

            "QPushButton {"
                "  background-color: #3A3A3A; "
                "  color: #FFFFFF; "
                "  border: 1px solid #555555; "
                "  padding: 6px 12px; "
                "  border-radius: 4px; "
                "  font-weight: bold; "
                "}"

            "QPushButton:hover { background-color: #4A4A4A; border: 1px solid #666666; }"
            "QPushButton:pressed { background-color: #2A82DA; color: white; }"
            "QPushButton:disabled { color: #777777; background-color: #2A2A2A; border: 1px solid #3A3A3A; }"
            "QPushButton:checked { background-color: #2A82DA; color: white; }"

            // Dropdown menus
            "QComboBox { background-color: #1E1E1E; color: #DDDDDD; border: 1px solid #555555; padding: 4px; border-radius: 3px; }"
            "QComboBox::drop-down { border-left: 1px solid #555555; width: 20px; }"
            "QComboBox::down-arrow { image: url(:/icons/dropdown_arrow.png); width: 12px; height: 12px; }"
            "QComboBox QAbstractItemView { background-color: #1E1E1E; color: #DDDDDD; selection-background-color: #2A82DA; border: 1px solid #555555; }"

            // Scrollbars
            "QScrollBar:vertical { background-color: #1E1E1E; width: 16px; margin: 0px; }"
            "QScrollBar::handle:vertical { background-color: #5A5A5A; min-height: 20px; border-radius: 3px; margin: 3px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
            "QScrollBar:horizontal { background-color: #1E1E1E; height: 16px; margin: 0px; }"
            "QScrollBar::handle:horizontal { background-color: #5A5A5A; min-width: 20px; border-radius: 3px; margin: 3px; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"

            // Group boxes
            "QGroupBox { border: 1px solid #555555; margin-top: 1.5em; padding-top: 0.8em; border-radius: 3px; }"
            "QGroupBox::title { color: #DDDDDD; subcontrol-origin: margin; left: 10px; padding: 0 5px; font-weight: bold; }"

            // Main navigation buttons (CPU, Disk, Network, Services)
            "QWidget[isMenuWidget=\"true\"] { "
            "  background-color: #252525; "
            "  border-radius: 5px; "
            "  padding: 8px; "
            "  border: 1px solid #555555; "
            "}"
            "QWidget[isMenuWidget=\"true\"] QPushButton { "
            "  padding: 10px 20px; "
            "  background-color: #3A3A3A; "
            "  color: #FFFFFF; "
            "  border: 1px solid #555555; "
            "  border-radius: 4px; "
            "  font-weight: bold; "
            "  min-width: 120px; "
            "}"
            "QWidget[isMenuWidget=\"true\"] QPushButton:hover { "
            "  background-color: #4A4A4A; "
            "  border: 1px solid #666666; "
            "}"
            "QWidget[isMenuWidget=\"true\"] QPushButton:pressed { "
            "  background-color: #2A82DA; "
            "  color: white; "
            "}"
            "QWidget[isMenuWidget=\"true\"] QPushButton:checked, "
            "QWidget[isMenuWidget=\"true\"] QPushButton:disabled { "
            "  background-color: #2A82DA; "
            "  color: white; "
            "  border: 1px solid #2A82DA; "
            "}"

            // Status bar
            "QStatusBar { background-color: #252525; color: #DDDDDD; border-top: 1px solid #555555; }"
            "QStatusBar QLabel { color: #DDDDDD; }"

            // Menu styling
            "QMenuBar { background-color: #252525; color: #DDDDDD; border-bottom: 1px solid #555555; }"
            "QMenuBar::item { background-color: transparent; padding: 6px 10px; }"
            "QMenuBar::item:selected { background-color: #3A3A3A; }"
            "QMenuBar::item:pressed { background-color: #2A82DA; color: white; }"
            "QMenu { background-color: #252525; color: #DDDDDD; border: 1px solid #555555; }"
            "QMenu::item { padding: 6px 20px 6px 20px; }"
            "QMenu::item:selected { background-color: #2A82DA; color: white; }"
            "QMenu::separator { height: 1px; background-color: #555555; margin: 5px 0px 5px 0px; }"

            // Labels
            "QLabel { color: #DDDDDD; }"

            // Frames and separators
            "QFrame[frameShape=\"4\"], QFrame[frameShape=\"5\"] { color: #555555; }" // HLine and VLine

            // Service status colors in the service table
            "QTableView#serviceTable QTableWidgetItem[status=\"active\"] { color: #33CC33; }"
            "QTableView#serviceTable QTableWidgetItem[status=\"inactive\"] { color: #AAAAAA; }"
            "QTableView#serviceTable QTableWidgetItem[status=\"failed\"] { color: #FF3333; }"

            // Network table specific styling
            "QTableWidget#networkTable { background-color: #1E1E1E; color: #DDDDDD; }"
            "QTableWidget#networkTable QTableWidgetItem { color: #DDDDDD; }"
            "QTableWidget#networkTable::item:alternate { background-color: #252525; }"
            "QTableWidget#networkTable::item { background-color: #1E1E1E; }"

            // Status colors in network table
            "QTableWidget#networkTable QTableWidgetItem[status=\"UP\"] { color: #33CC33; }"
            "QTableWidget#networkTable QTableWidgetItem[status=\"DOWN\"] { color: #FF3333; }"

            // USB table specific styling
            "QTableWidget#usbTable { background-color: #1E1E1E; color: #DDDDDD; }\n"
            "QTableWidget#usbTable QTableWidgetItem { color: #DDDDDD; }\n"
            "QTableWidget#usbTable::item:alternate { background-color: #252525; }\n"
            "QTableWidget#usbTable::item { background-color: #1E1E1E; }\n"
            "\n"
            // Status colors in USB table
            "QTableWidget#usbTable QTableWidgetItem[status=\"connected\"] { color: #33CC33; }\n"
            "QTableWidget#usbTable QTableWidgetItem[status=\"disconnected\"] { color: #FF3333; }\n"

            // Memory window specific styling
            "QProgressBar { border: 1px solid #555555; border-radius: 3px; background-color: #2A2A2A; text-align: center; }\n"
            "QProgressBar::chunk { background-color: #2A82DA; }\n"
        );
    } else {
        return QString(); // Use default style for light theme
    }
}