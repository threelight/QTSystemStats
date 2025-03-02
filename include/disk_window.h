#ifndef DISK_WINDOW_H
#define DISK_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QGroupBox>
#include <QGridLayout>
#include "disk_info.h"

class DiskWindow : public QWidget {
    Q_OBJECT

public:
    explicit DiskWindow(QWidget *parent = nullptr);
    ~DiskWindow() = default;

private:
    void setupUI();

    DiskInfoCollector diskInfo;
};

#endif // DISK_WINDOW_H