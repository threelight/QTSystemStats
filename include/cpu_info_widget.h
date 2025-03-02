#ifndef CPU_INFO_WIDGET_H
#define CPU_INFO_WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include "cpu_info.h"

class CPUInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit CPUInfoWidget(QWidget *parent = nullptr);
    ~CPUInfoWidget() = default;

private slots:
    void refreshData();

private:
    void setupUI();
    void updateCPUInfo();
    void updateCacheInfo();
    void updateFlagsInfo();

    // Helper methods to create different sections
    QWidget* createBasicInfoSection();
    QWidget* createFrequencySection();
    QWidget* createCacheSection();
    QWidget* createFlagsSection();

    CPUInfo cpuInfo;
    QTabWidget *tabWidget;
    QTableWidget *cpuInfoTable;
    QTableWidget *cacheInfoTable;
    QTableWidget *flagsTable;
    QLabel *lastUpdateLabel;
};

#endif // CPU_INFO_WIDGET_H