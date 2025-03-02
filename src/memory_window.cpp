#include "memory_window.h"
#include <QDateTime>
#include <QHeaderView>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QProcess>
#include <QRegularExpression>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

MemoryWindow::MemoryWindow(QWidget *parent) : QWidget(parent) {
    setupUI();

    // Initialize with current data
    refreshData();

    // Set up auto-refresh timer (every 10 seconds)
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MemoryWindow::refreshData);
    refreshTimer->start(10000); // 10 seconds
}

MemoryWindow::~MemoryWindow() {
    if (refreshTimer->isActive()) {
        refreshTimer->stop();
    }
}

void MemoryWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    QFont titleFont = font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    QLabel *titleLabel = new QLabel("Memory Information", this);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // RAM and Swap usage bars
    QGroupBox *usageGroup = new QGroupBox("Memory Usage", this);
    QVBoxLayout *usageLayout = new QVBoxLayout(usageGroup);

    QHBoxLayout *ramLayout = new QHBoxLayout();
    QLabel *ramLabel = new QLabel("RAM:", this);
    ramLabel->setMinimumWidth(80);
    ramProgressBar = new QProgressBar(this);
    ramProgressBar->setRange(0, 100);
    ramProgressBar->setTextVisible(true);
    ramLayout->addWidget(ramLabel);
    ramLayout->addWidget(ramProgressBar);

    QHBoxLayout *swapLayout = new QHBoxLayout();
    QLabel *swapLabel = new QLabel("Swap:", this);
    swapLabel->setMinimumWidth(80);
    swapProgressBar = new QProgressBar(this);
    swapProgressBar->setRange(0, 100);
    swapProgressBar->setTextVisible(true);
    swapLayout->addWidget(swapLabel);
    swapLayout->addWidget(swapProgressBar);

    usageLayout->addLayout(ramLayout);
    usageLayout->addLayout(swapLayout);

    mainLayout->addWidget(usageGroup);

    // Detailed memory information table
    QGroupBox *detailsGroup = new QGroupBox("Memory Details", this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsGroup);

    memoryTable = new QTableWidget(0, 2, this);
    memoryTable->setHorizontalHeaderLabels({"Metric", "Value"});
    memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    memoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    memoryTable->setAlternatingRowColors(true);
    memoryTable->verticalHeader()->setVisible(false);

    detailsLayout->addWidget(memoryTable);
    mainLayout->addWidget(detailsGroup);

    // Charts for memory usage over time
    QGroupBox *chartsGroup = new QGroupBox("Memory Usage Over Time", this);
    QVBoxLayout *chartsLayout = new QVBoxLayout(chartsGroup);

    // RAM usage chart
    ramChart = new QChart();
    ramChart->setTitle("RAM Usage History");
    ramChart->legend()->setVisible(true);
    ramChart->legend()->setAlignment(Qt::AlignBottom);

    ramUsageSeries = new QLineSeries();
    ramUsageSeries->setName("Used RAM");

    ramCachedSeries = new QLineSeries();
    ramCachedSeries->setName("Cached");

    ramBuffersSeries = new QLineSeries();
    ramBuffersSeries->setName("Buffers");

    ramChart->addSeries(ramUsageSeries);
    ramChart->addSeries(ramCachedSeries);
    ramChart->addSeries(ramBuffersSeries);

    QDateTimeAxis *ramTimeAxis = new QDateTimeAxis;
    ramTimeAxis->setFormat("hh:mm:ss");
    ramTimeAxis->setTitleText("Time");
    ramChart->addAxis(ramTimeAxis, Qt::AlignBottom);

    QValueAxis *ramValueAxis = new QValueAxis;
    ramValueAxis->setTitleText("Memory (GB)");
    ramValueAxis->setLabelFormat("%.2f");
    ramChart->addAxis(ramValueAxis, Qt::AlignLeft);

    ramUsageSeries->attachAxis(ramTimeAxis);
    ramUsageSeries->attachAxis(ramValueAxis);
    ramCachedSeries->attachAxis(ramTimeAxis);
    ramCachedSeries->attachAxis(ramValueAxis);
    ramBuffersSeries->attachAxis(ramTimeAxis);
    ramBuffersSeries->attachAxis(ramValueAxis);

    ramChartView = new QChartView(ramChart);
    ramChartView->setRenderHint(QPainter::Antialiasing);

    // Swap usage chart
    swapChart = new QChart();
    swapChart->setTitle("Swap Usage History");
    swapChart->legend()->setVisible(true);
    swapChart->legend()->setAlignment(Qt::AlignBottom);

    swapUsageSeries = new QLineSeries();
    swapUsageSeries->setName("Used Swap");

    swapChart->addSeries(swapUsageSeries);

    QDateTimeAxis *swapTimeAxis = new QDateTimeAxis;
    swapTimeAxis->setFormat("hh:mm:ss");
    swapTimeAxis->setTitleText("Time");
    swapChart->addAxis(swapTimeAxis, Qt::AlignBottom);

    QValueAxis *swapValueAxis = new QValueAxis;
    swapValueAxis->setTitleText("Memory (GB)");
    swapValueAxis->setLabelFormat("%.2f");
    swapChart->addAxis(swapValueAxis, Qt::AlignLeft);

    swapUsageSeries->attachAxis(swapTimeAxis);
    swapUsageSeries->attachAxis(swapValueAxis);

    swapChartView = new QChartView(swapChart);
    swapChartView->setRenderHint(QPainter::Antialiasing);

    // Add charts to layout
    QHBoxLayout *chartViewsLayout = new QHBoxLayout();
    chartViewsLayout->addWidget(ramChartView);
    chartViewsLayout->addWidget(swapChartView);
    chartsLayout->addLayout(chartViewsLayout);

    mainLayout->addWidget(chartsGroup);

    // Controls at the bottom
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    QCheckBox *autoRefreshCheckbox = new QCheckBox("Auto-refresh (10s)", this);
    autoRefreshCheckbox->setChecked(true);
    connect(autoRefreshCheckbox, &QCheckBox::toggled, this, &MemoryWindow::toggleAutoRefresh);

    lastUpdateLabel = new QLabel("Last update: Never", this);

    refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &MemoryWindow::refreshData);

    controlsLayout->addWidget(autoRefreshCheckbox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(lastUpdateLabel);
    controlsLayout->addWidget(refreshButton);

    mainLayout->addLayout(controlsLayout);
}

void MemoryWindow::refreshData() {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Get current memory info
    MemoryInfo info = getMemoryInfo();

    // Add to history
    memoryHistory.push_back(info);
    if (memoryHistory.size() > MAX_HISTORY_SIZE) {
        memoryHistory.pop_front();
    }

    // Update UI
    updateUI();

    // Update last update time
    lastUpdateLabel->setText("Last update: " + QDateTime::currentDateTime().toString("hh:mm:ss"));

    QApplication::restoreOverrideCursor();
}

void MemoryWindow::updateUI() {
    if (memoryHistory.empty()) {
        return;
    }

    const MemoryInfo &info = memoryHistory.back();

    // Update progress bars
    int ramUsagePercent = static_cast<int>((static_cast<double>(info.usedRam) / info.totalRam) * 100);
    ramProgressBar->setValue(ramUsagePercent);
    ramProgressBar->setFormat(QString("%1% (%2 / %3)")
                             .arg(ramUsagePercent)
                             .arg(formatSize(info.usedRam))
                             .arg(formatSize(info.totalRam)));

    // Set color based on usage
    QString ramStyle = "QProgressBar { text-align: center; } QProgressBar::chunk { ";
    if (ramUsagePercent < 60) {
        ramStyle += "background-color: #4CAF50; }"; // Green
    } else if (ramUsagePercent < 85) {
        ramStyle += "background-color: #FF9800; }"; // Orange
    } else {
        ramStyle += "background-color: #F44336; }"; // Red
    }
    ramProgressBar->setStyleSheet(ramStyle);

    int swapUsagePercent = 0;
    if (info.totalSwap > 0) {
        swapUsagePercent = static_cast<int>((static_cast<double>(info.usedSwap) / info.totalSwap) * 100);
    }
    swapProgressBar->setValue(swapUsagePercent);
    swapProgressBar->setFormat(QString("%1% (%2 / %3)")
                              .arg(swapUsagePercent)
                              .arg(formatSize(info.usedSwap))
                              .arg(formatSize(info.totalSwap)));

    // Set color based on usage
    QString swapStyle = "QProgressBar { text-align: center; } QProgressBar::chunk { ";
    if (swapUsagePercent < 40) {
        swapStyle += "background-color: #4CAF50; }"; // Green
    } else if (swapUsagePercent < 70) {
        swapStyle += "background-color: #FF9800; }"; // Orange
    } else {
        swapStyle += "background-color: #F44336; }"; // Red
    }
    swapProgressBar->setStyleSheet(swapStyle);

    // Update table
    memoryTable->setRowCount(0);

    auto addRow = [this](const QString &metric, const QString &value) {
        int row = memoryTable->rowCount();
        memoryTable->insertRow(row);
        memoryTable->setItem(row, 0, new QTableWidgetItem(metric));
        memoryTable->setItem(row, 1, new QTableWidgetItem(value));
    };

    addRow("Total RAM", formatSize(info.totalRam));
    addRow("Used RAM", formatSize(info.usedRam));
    addRow("Free RAM", formatSize(info.freeRam));
    addRow("Available RAM", formatSize(info.availableRam));
    addRow("Buffers", formatSize(info.buffers));
    addRow("Cached", formatSize(info.cached));
    addRow("Shared Memory", formatSize(info.shmem));
    addRow("Total Swap", formatSize(info.totalSwap));
    addRow("Used Swap", formatSize(info.usedSwap));
    addRow("Free Swap", formatSize(info.freeSwap));
    addRow("RAM Usage", QString("%1%").arg(ramUsagePercent));
    addRow("Swap Usage", QString("%1%").arg(swapUsagePercent));

    // Update charts
    ramUsageSeries->clear();
    ramCachedSeries->clear();
    ramBuffersSeries->clear();
    swapUsageSeries->clear();

    // Convert to GB for better readability in charts
    const double GB = 1024.0 * 1024.0 * 1024.0;

    for (const auto &historyItem : memoryHistory) {
        qint64 msecsSinceEpoch = historyItem.timestamp.toMSecsSinceEpoch();

        // RAM chart
        double usedRamGB = historyItem.usedRam / GB;
        double cachedGB = historyItem.cached / GB;
        double buffersGB = historyItem.buffers / GB;

        ramUsageSeries->append(msecsSinceEpoch, usedRamGB);
        ramCachedSeries->append(msecsSinceEpoch, cachedGB);
        ramBuffersSeries->append(msecsSinceEpoch, buffersGB);

        // Swap chart
        double usedSwapGB = historyItem.usedSwap / GB;
        swapUsageSeries->append(msecsSinceEpoch, usedSwapGB);
    }

    // Update chart axes
    if (!memoryHistory.empty()) {
        QDateTime firstTime = memoryHistory.front().timestamp;
        QDateTime lastTime = memoryHistory.back().timestamp;

        // RAM chart
        QDateTimeAxis *ramTimeAxis = qobject_cast<QDateTimeAxis*>(ramChart->axes(Qt::Horizontal).first());
        ramTimeAxis->setRange(firstTime, lastTime);

        QValueAxis *ramValueAxis = qobject_cast<QValueAxis*>(ramChart->axes(Qt::Vertical).first());
        double maxRamValue = info.totalRam / GB;
        ramValueAxis->setRange(0, maxRamValue);

        // Swap chart
        QDateTimeAxis *swapTimeAxis = qobject_cast<QDateTimeAxis*>(swapChart->axes(Qt::Horizontal).first());
        swapTimeAxis->setRange(firstTime, lastTime);

        QValueAxis *swapValueAxis = qobject_cast<QValueAxis*>(swapChart->axes(Qt::Vertical).first());
        double maxSwapValue = std::max(1.0, info.totalSwap / GB); // At least 1 GB for visibility
        swapValueAxis->setRange(0, maxSwapValue);
    }
}

MemoryInfo MemoryWindow::getMemoryInfo() {
    MemoryInfo info = {};
    info.timestamp = QDateTime::currentDateTime();

    // Read /proc/meminfo
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open /proc/meminfo";
        return info;
    }

    QTextStream in(&file);
    QString line;

    while (in.readLineInto(&line)) {
        QStringList parts = line.split(":");
        if (parts.size() < 2) continue;

        QString key = parts[0].trimmed();
        QString valueStr = parts[1].trimmed();

        // Remove " kB" suffix and convert to bytes
        valueStr.remove(" kB");
        uint64_t value = valueStr.toULongLong() * 1024; // Convert KB to bytes

        if (key == "MemTotal") {
            info.totalRam = value;
        } else if (key == "MemFree") {
            info.freeRam = value;
        } else if (key == "MemAvailable") {
            info.availableRam = value;
        } else if (key == "Buffers") {
            info.buffers = value;
        } else if (key == "Cached") {
            info.cached = value;
        } else if (key == "Shmem") {
            info.shmem = value;
        } else if (key == "SwapTotal") {
            info.totalSwap = value;
        } else if (key == "SwapFree") {
            info.freeSwap = value;
        }
    }

    file.close();

    // Calculate derived values
    info.usedRam = info.totalRam - info.freeRam - info.buffers - info.cached;
    info.usedSwap = info.totalSwap - info.freeSwap;

    return info;
}

QString MemoryWindow::formatSize(uint64_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

void MemoryWindow::toggleAutoRefresh(bool enabled) {
    if (enabled) {
        refreshTimer->start(10000);
    } else {
        refreshTimer->stop();
    }
}