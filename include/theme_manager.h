#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>
#include <QSettings>

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum Theme {
        LightTheme,
        DarkTheme
    };

    static ThemeManager& instance() {
        static ThemeManager instance;
        return instance;
    }

    void setTheme(Theme theme);
    Theme currentTheme() const { return m_currentTheme; }
    void toggleTheme();
    void loadTheme();
    void saveTheme();

signals:
    void themeChanged(Theme newTheme);

private:
    ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    QPalette createLightPalette() const;
    QPalette createDarkPalette() const;
    QString getStyleSheet(Theme theme) const;

    Theme m_currentTheme;
};

#endif // THEME_MANAGER_H