#include "service_window.h"
#include <QHeaderView>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>
#include <QSplitter>
#include <QApplication>
#include <QSortFilterProxyModel>

ServiceWindow::ServiceWindow(QWidget *parent) : QWidget(parent), selectedService("") {
    setupUI();
    updateTable();

    // Set up auto-refresh timer (every 10 seconds)
    autoRefreshTimer = new QTimer(this);
    connect(autoRefreshTimer, &QTimer::timeout, this, &ServiceWindow::refreshData);
    autoRefreshTimer->start(10000);
}

void ServiceWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFont titleFont = font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    // Create header
    QLabel *headerLabel = new QLabel("System Services", this);
    headerLabel->setFont(titleFont);
    mainLayout->addWidget(headerLabel);

    // Create filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();

    QLabel *filterLabel = new QLabel("Filter:", this);
    filterLayout->addWidget(filterLabel);

    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Enter service name to filter...");
    connect(filterEdit, &QLineEdit::textChanged, this, &ServiceWindow::filterServices);
    filterLayout->addWidget(filterEdit);

    refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &ServiceWindow::refreshData);
    filterLayout->addWidget(refreshButton);

    mainLayout->addLayout(filterLayout);

    // Create a splitter for table and details
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    // Create table for services
    serviceTable = new QTableWidget(this);
    serviceTable->setObjectName("serviceTable");
    serviceTable->setColumnCount(4);
    serviceTable->setHorizontalHeaderLabels(
        {"Service", "Description", "Status", "Sub-State"});
    serviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    serviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    serviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    serviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    serviceTable->setAlternatingRowColors(true);

    // Enable sorting
    serviceTable->setSortingEnabled(true);

    connect(serviceTable, &QTableWidget::cellClicked, this, &ServiceWindow::serviceSelected);
    splitter->addWidget(serviceTable);

    // Create details group
    QGroupBox *detailsGroup = new QGroupBox("Service Details", this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);

    detailsTextEdit = new QTextEdit(this);
    detailsTextEdit->setReadOnly(true);
    detailsLayout->addWidget(detailsTextEdit);

    // Create control buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();

    startButton = new QPushButton("Start", this);
    startButton->setEnabled(false);
    connect(startButton, &QPushButton::clicked, this, &ServiceWindow::startSelectedService);
    controlLayout->addWidget(startButton);

    stopButton = new QPushButton("Stop", this);
    stopButton->setEnabled(false);
    connect(stopButton, &QPushButton::clicked, this, &ServiceWindow::stopSelectedService);
    controlLayout->addWidget(stopButton);

    restartButton = new QPushButton("Restart", this);
    restartButton->setEnabled(false);
    connect(restartButton, &QPushButton::clicked, this, &ServiceWindow::restartSelectedService);
    controlLayout->addWidget(restartButton);

    controlLayout->addStretch();

    enableButton = new QPushButton("Enable", this);
    enableButton->setEnabled(false);
    connect(enableButton, &QPushButton::clicked, this, &ServiceWindow::enableSelectedService);
    controlLayout->addWidget(enableButton);

    disableButton = new QPushButton("Disable", this);
    disableButton->setEnabled(false);
    connect(disableButton, &QPushButton::clicked, this, &ServiceWindow::disableSelectedService);
    controlLayout->addWidget(disableButton);

    detailsLayout->addLayout(controlLayout);

    splitter->addWidget(detailsGroup);
    mainLayout->addWidget(splitter);

    // Set initial splitter sizes
    QList<int> sizes;
    sizes << 300 << 200;
    splitter->setSizes(sizes);

    // Create status bar
    QHBoxLayout *statusLayout = new QHBoxLayout();

    // Auto-refresh checkbox
    QCheckBox *autoRefreshCheckbox = new QCheckBox("Auto-refresh (10s)", this);
    autoRefreshCheckbox->setChecked(true);
    connect(autoRefreshCheckbox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            autoRefreshTimer->start(10000);
        } else {
            autoRefreshTimer->stop();
        }
    });
    statusLayout->addWidget(autoRefreshCheckbox);

    statusLayout->addStretch();

    statusLabel = new QLabel(this);
    statusLayout->addWidget(statusLabel);

    lastUpdateLabel = new QLabel(this);
    statusLayout->addWidget(lastUpdateLabel);

    mainLayout->addLayout(statusLayout);
}

void ServiceWindow::updateTable() {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Temporarily disable sorting while updating the table
    bool wasSortingEnabled = serviceTable->isSortingEnabled();
    serviceTable->setSortingEnabled(false);

    auto services = serviceInfo.getAllServices();

    // Apply filter if any
    QString filter = filterEdit->text().toLower();
    std::vector<ServiceInfo> filteredServices;

    if (!filter.isEmpty()) {
        for (const auto& service : services) {
            QString serviceName = QString::fromStdString(service.name).toLower();
            QString description = QString::fromStdString(service.description).toLower();

            if (serviceName.contains(filter) || description.contains(filter)) {
                filteredServices.push_back(service);
            }
        }
    } else {
        filteredServices = services;
    }

    serviceTable->setRowCount(filteredServices.size());

    for (size_t i = 0; i < filteredServices.size(); ++i) {
        const auto& service = filteredServices[i];

        // Service name
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(service.name));
        nameItem->setData(Qt::UserRole, QString::fromStdString(service.name)); // Store original name for sorting
        serviceTable->setItem(i, 0, nameItem);

        // Description
        QTableWidgetItem *descItem = new QTableWidgetItem(QString::fromStdString(service.description));
        serviceTable->setItem(i, 1, descItem);

        // Status
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(service.activeState));
        // Store a numeric value for sorting (active=2, failed=0, inactive=1)
        int statusValue = 1; // Default for inactive
        if (service.status == ServiceStatus::ACTIVE) {
            statusItem->setForeground(Qt::darkGreen);
            statusValue = 2;
            statusItem->setData(Qt::UserRole + 1, "active"); // Add status attribute
        } else if (service.status == ServiceStatus::FAILED) {
            statusItem->setForeground(Qt::red);
            statusValue = 0;
            statusItem->setData(Qt::UserRole + 1, "failed"); // Add status attribute
        } else if (service.status == ServiceStatus::INACTIVE) {
            statusItem->setForeground(Qt::darkGray);
            statusValue = 1;
            statusItem->setData(Qt::UserRole + 1, "inactive"); // Add status attribute
        }
        statusItem->setData(Qt::UserRole, statusValue);
        serviceTable->setItem(i, 2, statusItem);

        // Sub-state
        QTableWidgetItem *subStateItem = new QTableWidgetItem(QString::fromStdString(service.subState));
        serviceTable->setItem(i, 3, subStateItem);
    }

    // Re-enable sorting if it was enabled before
    serviceTable->setSortingEnabled(wasSortingEnabled);

    // Update last refresh time
    QDateTime now = QDateTime::currentDateTime();
    lastUpdateLabel->setText("Last updated: " + now.toString("hh:mm:ss"));

    QApplication::restoreOverrideCursor();
}

void ServiceWindow::refreshData() {
    // Remember the current sort column and order
    int sortColumn = serviceTable->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder sortOrder = serviceTable->horizontalHeader()->sortIndicatorOrder();

    serviceInfo.refresh();
    updateTable();

    // Restore the sort order
    if (serviceTable->isSortingEnabled()) {
        serviceTable->sortItems(sortColumn, sortOrder);
    }

    // If a service was selected, update its details
    if (!selectedService.empty()) {
        updateServiceDetails(serviceInfo.getServiceDetails(selectedService));
    }
}

void ServiceWindow::filterServices(const QString& text) {
    updateTable();
}

void ServiceWindow::serviceSelected(int row, int column) {
    selectedService = serviceTable->item(row, 0)->text().toStdString();

    // Get detailed information about the selected service
    ServiceInfo details = serviceInfo.getServiceDetails(selectedService);
    updateServiceDetails(details);

    // Enable control buttons
    startButton->setEnabled(details.status != ServiceStatus::ACTIVE);
    stopButton->setEnabled(details.status == ServiceStatus::ACTIVE);
    restartButton->setEnabled(details.status == ServiceStatus::ACTIVE);
    enableButton->setEnabled(details.unitFileState != "enabled");
    disableButton->setEnabled(details.unitFileState == "enabled");
}

void ServiceWindow::updateServiceDetails(const ServiceInfo& service) {
    QString details;
    details += "<h3>" + QString::fromStdString(service.name) + "</h3>";
    details += "<p><b>Description:</b> " + QString::fromStdString(service.description) + "</p>";
    details += "<p><b>Status:</b> " + QString::fromStdString(service.activeState) + " (" +
               QString::fromStdString(service.subState) + ")</p>";
    details += "<p><b>Load State:</b> " + QString::fromStdString(service.loadState) + "</p>";
    details += "<p><b>Unit File State:</b> " + QString::fromStdString(service.unitFileState) + "</p>";

    detailsTextEdit->setHtml(details);
}

void ServiceWindow::startSelectedService() {
    if (selectedService.empty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = serviceInfo.startService(selectedService);
    QApplication::restoreOverrideCursor();

    if (success) {
        showMessage("Service started successfully");
        refreshData();
    } else {
        showMessage("Failed to start service. You may need administrative privileges.", true);
    }
}

void ServiceWindow::stopSelectedService() {
    if (selectedService.empty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = serviceInfo.stopService(selectedService);
    QApplication::restoreOverrideCursor();

    if (success) {
        showMessage("Service stopped successfully");
        refreshData();
    } else {
        showMessage("Failed to stop service. You may need administrative privileges.", true);
    }
}

void ServiceWindow::restartSelectedService() {
    if (selectedService.empty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = serviceInfo.restartService(selectedService);
    QApplication::restoreOverrideCursor();

    if (success) {
        showMessage("Service restarted successfully");
        refreshData();
    } else {
        showMessage("Failed to restart service. You may need administrative privileges.", true);
    }
}

void ServiceWindow::enableSelectedService() {
    if (selectedService.empty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = serviceInfo.enableService(selectedService);
    QApplication::restoreOverrideCursor();

    if (success) {
        showMessage("Service enabled successfully");
        refreshData();
    } else {
        showMessage("Failed to enable service. You may need administrative privileges.", true);
    }
}

void ServiceWindow::disableSelectedService() {
    if (selectedService.empty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = serviceInfo.disableService(selectedService);
    QApplication::restoreOverrideCursor();

    if (success) {
        showMessage("Service disabled successfully");
        refreshData();
    } else {
        showMessage("Failed to disable service. You may need administrative privileges.", true);
    }
}

void ServiceWindow::showMessage(const QString& message, bool isError) {
    statusLabel->setText(message);

    if (isError) {
        statusLabel->setStyleSheet("color: red;");
        QMessageBox::warning(this, "Service Operation", message);
    } else {
        statusLabel->setStyleSheet("color: green;");
    }

    // Clear the message after 5 seconds
    QTimer::singleShot(5000, [this]() {
        statusLabel->setText("");
        statusLabel->setStyleSheet("");
    });
}