#ifndef MEMORY_WINDOW_H
#define MEMORY_WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>

#ifdef USE_QT_CHARTS
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
QT_CHARTS_USE_NAMESPACE
#endif

#include <deque>

struct MemoryInfo {
    // RAM info
    uint64_t totalRam;
    uint64_t freeRam;
    uint64_t availableRam;
    uint64_t buffers;
    uint64_t cached;
    uint64_t shmem;

    // Swap info
    uint64_t totalSwap;
    uint64_t freeSwap;

    // Calculated values
    uint64_t usedRam;
    uint64_t usedSwap;

    // Timestamps for graphs
    QDateTime timestamp;
};

class MemoryWindow : public QWidget {
    Q_OBJECT

public:
    explicit MemoryWindow(QWidget *parent = nullptr);
    ~MemoryWindow();

private slots:
    void refreshData();
    void updateUI();
    void toggleAutoRefresh(bool enabled);

private:
    void setupUI();
    MemoryInfo getMemoryInfo();
    QString formatSize(uint64_t bytes) const;

    // UI components
    QLabel *lastUpdateLabel;
    QProgressBar *ramProgressBar;
    QProgressBar *swapProgressBar;
    QTableWidget *memoryTable;
    QPushButton *refreshButton;
    QTimer *refreshTimer;

#ifdef USE_QT_CHARTS
    // Chart components
    QChart *ramChart;
    QChartView *ramChartView;
    QLineSeries *ramUsageSeries;
    QLineSeries *ramCachedSeries;
    QLineSeries *ramBuffersSeries;

    QChart *swapChart;
    QChartView *swapChartView;
    QLineSeries *swapUsageSeries;
#endif

    // Data storage
    std::deque<MemoryInfo> memoryHistory;
    const int MAX_HISTORY_SIZE = 60; // Store 60 data points (10 minutes at 10s intervals)
};

#endif // MEMORY_WINDOW_H