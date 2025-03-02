#include "network_window.h"
#include <QHeaderView>
#include <QDateTime>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

NetworkWindow::NetworkWindow(QWidget *parent) : QWidget(parent) {
    setupUI();
    updateTable();

    // Set up auto-refresh timer (every 5 seconds)
    autoRefreshTimer = new QTimer(this);
    connect(autoRefreshTimer, &QTimer::timeout, this, &NetworkWindow::refreshData);
    autoRefreshTimer->start(5000);
}

void NetworkWindow::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QFont titleFont = font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    // Create header
    QLabel *headerLabel = new QLabel("Network Interfaces", this);
    headerLabel->setFont(titleFont);
    layout->addWidget(headerLabel);

    // Create table for network interfaces
    interfaceTable = new QTableWidget(this);
    interfaceTable->setObjectName("networkTable");
    interfaceTable->setColumnCount(7);
    interfaceTable->setHorizontalHeaderLabels(
        {"Interface", "Status", "IP Address", "MAC Address", "Type", "RX", "TX"});
    interfaceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    interfaceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    interfaceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    interfaceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    interfaceTable->setAlternatingRowColors(true);
    layout->addWidget(interfaceTable);

    // Create controls at the bottom
    QHBoxLayout *controlLayout = new QHBoxLayout();

    // Auto-refresh checkbox
    QCheckBox *autoRefreshCheckbox = new QCheckBox("Auto-refresh (5s)", this);
    autoRefreshCheckbox->setChecked(true);
    connect(autoRefreshCheckbox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            autoRefreshTimer->start(5000);
        } else {
            autoRefreshTimer->stop();
        }
    });
    controlLayout->addWidget(autoRefreshCheckbox);

    controlLayout->addStretch();

    // Last update label
    lastUpdateLabel = new QLabel(this);
    controlLayout->addWidget(lastUpdateLabel);

    // Refresh button
    refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &NetworkWindow::refreshData);
    controlLayout->addWidget(refreshButton);

    layout->addLayout(controlLayout);

    // Add detailed statistics in a collapsible group
    QGroupBox *statsGroup = new QGroupBox("Detailed Statistics", this);
    statsGroup->setCheckable(true);
    statsGroup->setChecked(false);

    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);

    QTableWidget *statsTable = new QTableWidget(this);
    statsTable->setColumnCount(5);
    statsTable->setHorizontalHeaderLabels(
        {"Interface", "RX Packets", "TX Packets", "RX Errors", "TX Errors"});
    statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setAlternatingRowColors(true);

    // Populate statistics table
    auto interfaces = networkInfo.getInterfaces();
    statsTable->setRowCount(interfaces.size());

    for (size_t i = 0; i < interfaces.size(); ++i) {
        const auto& iface = interfaces[i];

        statsTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(iface.name)));
        statsTable->setItem(i, 1, new QTableWidgetItem(QString::number(iface.rxPackets)));
        statsTable->setItem(i, 2, new QTableWidgetItem(QString::number(iface.txPackets)));
        statsTable->setItem(i, 3, new QTableWidgetItem(QString::number(iface.rxErrors)));
        statsTable->setItem(i, 4, new QTableWidgetItem(QString::number(iface.txErrors)));
    }

    statsLayout->addWidget(statsTable);
    layout->addWidget(statsGroup);
}

void NetworkWindow::updateTable() {
    auto interfaces = networkInfo.getInterfaces();
    interfaceTable->setRowCount(interfaces.size());

    for (size_t i = 0; i < interfaces.size(); ++i) {
        const auto& iface = interfaces[i];

        // Interface name
        interfaceTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(iface.name)));

        // Status
        QString status = iface.isUp ? "UP" : "DOWN";
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(iface.isUp ? Qt::darkGreen : Qt::red);
        interfaceTable->setItem(i, 1, statusItem);

        // IP Address
        interfaceTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(iface.ipAddress)));

        // MAC Address
        interfaceTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(iface.macAddress)));

        // Type
        QString type;
        if (iface.isLoopback) {
            type = "Loopback";
        } else if (iface.isWireless) {
            type = "Wireless";
        } else {
            type = "Wired";
        }
        interfaceTable->setItem(i, 4, new QTableWidgetItem(type));

        // RX Bytes
        QString rxText = QString::fromStdString(networkInfo.formatBytes(iface.rxBytes));
        interfaceTable->setItem(i, 5, new QTableWidgetItem(rxText));

        // TX Bytes
        QString txText = QString::fromStdString(networkInfo.formatBytes(iface.txBytes));
        interfaceTable->setItem(i, 6, new QTableWidgetItem(txText));
    }

    // Update last refresh time
    QDateTime now = QDateTime::currentDateTime();
    lastUpdateLabel->setText("Last updated: " + now.toString("hh:mm:ss"));
}

void NetworkWindow::refreshData() {
    networkInfo.refresh();
    updateTable();
}