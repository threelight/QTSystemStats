#ifndef SIMPLE_MENU_WINDOW_H
#define SIMPLE_MENU_WINDOW_H

#include <QMainWindow>
#include <QAction>

class SimpleMenuWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SimpleMenuWindow(QWidget *parent = nullptr);
    ~SimpleMenuWindow() = default;

private slots:
    void showMessage();
    void about();

private:
    void createMenus();

    QAction *action1;
    QAction *action2;
    QAction *exitAction;
    QAction *aboutAction;
};

#endif // SIMPLE_MENU_WINDOW_H