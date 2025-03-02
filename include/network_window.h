#ifndef NETWORK_WINDOW_H
#define NETWORK_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include "network_info.h"

class NetworkWindow : public QWidget {
    Q_OBJECT

public:
    explicit NetworkWindow(QWidget *parent = nullptr);
    ~NetworkWindow() = default;

private slots:
    void refreshData();

private:
    void setupUI();
    void updateTable();

    NetworkInfoCollector networkInfo;
    QTableWidget *interfaceTable;
    QPushButton *refreshButton;
    QLabel *lastUpdateLabel;
    QTimer *autoRefreshTimer;
};

#endif // NETWORK_WINDOW_H