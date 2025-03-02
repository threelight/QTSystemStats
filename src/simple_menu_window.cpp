#include "simple_menu_window.h"
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

SimpleMenuWindow::SimpleMenuWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Simple Menu Example");
    resize(400, 300);

    // Create a central widget with a simple label
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    QLabel *label = new QLabel("This is a simple Qt application to test menu functionality.\n"
                              "Try using the menu bar at the top of the window.", centralWidget);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    // Force the menu bar to be visible and not use native menu bar
    menuBar()->setNativeMenuBar(false);
    menuBar()->setVisible(true);

    createMenus();

    // Show status bar
    statusBar()->showMessage("Ready");
}

void SimpleMenuWindow::createMenus() {
    // Create File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    // Action 1
    action1 = new QAction(tr("&Action 1"), this);
    action1->setStatusTip(tr("Execute Action 1"));
    connect(action1, &QAction::triggered, this, &SimpleMenuWindow::showMessage);
    fileMenu->addAction(action1);

    // Action 2
    action2 = new QAction(tr("&Action 2"), this);
    action2->setStatusTip(tr("Execute Action 2"));
    connect(action2, &QAction::triggered, this, &SimpleMenuWindow::showMessage);
    fileMenu->addAction(action2);

    fileMenu->addSeparator();

    // Exit action
    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    fileMenu->addAction(exitAction);

    // Create Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    // About action
    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, &QAction::triggered, this, &SimpleMenuWindow::about);
    helpMenu->addAction(aboutAction);
}

void SimpleMenuWindow::showMessage() {
    // Implementation of showMessage
    QMessageBox::information(this, "Message", "This is a simple message");
}

void SimpleMenuWindow::about() {
    // Implementation of about
    QMessageBox::about(this, "About", "Simple Menu Example\nVersion 1.0");
}