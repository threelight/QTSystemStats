#include "disk_window.h"

DiskWindow::DiskWindow(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void DiskWindow::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QFont titleFont = font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    QFont valueFont = font();
    valueFont.setPointSize(10);

    // Get disk information
    std::vector<DiskInfo> disks = diskInfo.getDisks();

    if (disks.empty()) {
        QLabel *noDisksLabel = new QLabel("No disk information available", this);
        noDisksLabel->setFont(titleFont);
        layout->addWidget(noDisksLabel);
        layout->addStretch();
        return;
    }

    // Create a scroll area for many disks
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget(scrollArea);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);

    // Format size helper function
    auto formatSize = [](uint64_t size) -> QString {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double formattedSize = static_cast<double>(size);

        while (formattedSize >= 1024.0 && unitIndex < 4) {
            formattedSize /= 1024.0;
            unitIndex++;
        }

        return QString("%1 %2").arg(formattedSize, 0, 'f', 2).arg(units[unitIndex]);
    };

    // Add each disk as a group box
    for (const auto& disk : disks) {
        QGroupBox *diskGroup = new QGroupBox(QString::fromStdString(disk.device), scrollContent);
        QVBoxLayout *diskLayout = new QVBoxLayout(diskGroup);

        // Create a grid layout for disk details
        QGridLayout *gridLayout = new QGridLayout();
        diskLayout->addLayout(gridLayout);

        // Mount point
        QLabel *mountLabel = new QLabel("Mount Point:", diskGroup);
        mountLabel->setFont(titleFont);
        gridLayout->addWidget(mountLabel, 0, 0);

        QLabel *mountValue = new QLabel(QString::fromStdString(disk.mountPoint), diskGroup);
        mountValue->setFont(valueFont);
        mountValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
        gridLayout->addWidget(mountValue, 0, 1);

        // File system
        QLabel *fsLabel = new QLabel("File System:", diskGroup);
        fsLabel->setFont(titleFont);
        gridLayout->addWidget(fsLabel, 1, 0);

        QLabel *fsValue = new QLabel(QString::fromStdString(disk.fsType), diskGroup);
        fsValue->setFont(valueFont);
        gridLayout->addWidget(fsValue, 1, 1);

        // Total size
        QLabel *totalLabel = new QLabel("Total Size:", diskGroup);
        totalLabel->setFont(titleFont);
        gridLayout->addWidget(totalLabel, 2, 0);

        QLabel *totalValue = new QLabel(formatSize(disk.totalSize), diskGroup);
        totalValue->setFont(valueFont);
        gridLayout->addWidget(totalValue, 2, 1);

        // Free space
        QLabel *freeLabel = new QLabel("Free Space:", diskGroup);
        freeLabel->setFont(titleFont);
        gridLayout->addWidget(freeLabel, 3, 0);

        QLabel *freeValue = new QLabel(formatSize(disk.freeSpace), diskGroup);
        freeValue->setFont(valueFont);
        gridLayout->addWidget(freeValue, 3, 1);

        // Available space
        QLabel *availLabel = new QLabel("Available Space:", diskGroup);
        availLabel->setFont(titleFont);
        gridLayout->addWidget(availLabel, 4, 0);

        QLabel *availValue = new QLabel(formatSize(disk.availableSpace), diskGroup);
        availValue->setFont(valueFont);
        gridLayout->addWidget(availValue, 4, 1);

        // Used space
        QLabel *usedLabel = new QLabel("Used Space:", diskGroup);
        usedLabel->setFont(titleFont);
        gridLayout->addWidget(usedLabel, 5, 0);

        QLabel *usedValue = new QLabel(formatSize(disk.totalSize - disk.freeSpace), diskGroup);
        usedValue->setFont(valueFont);
        gridLayout->addWidget(usedValue, 5, 1);

        // Usage percentage
        double usagePercent = static_cast<double>(disk.totalSize - disk.freeSpace) / disk.totalSize * 100.0;

        QLabel *usageLabel = new QLabel("Usage:", diskGroup);
        usageLabel->setFont(titleFont);
        gridLayout->addWidget(usageLabel, 6, 0);

        QLabel *usageValue = new QLabel(QString("%1%").arg(usagePercent, 0, 'f', 2), diskGroup);
        usageValue->setFont(valueFont);
        gridLayout->addWidget(usageValue, 6, 1);

        // Add a progress bar for visual representation
        QProgressBar *usageBar = new QProgressBar(diskGroup);
        usageBar->setMinimum(0);
        usageBar->setMaximum(100);
        usageBar->setValue(static_cast<int>(usagePercent));

        // Set color based on usage
        QString styleSheet;
        if (usagePercent < 70) {
            styleSheet = "QProgressBar::chunk { background-color: green; }";
        } else if (usagePercent < 90) {
            styleSheet = "QProgressBar::chunk { background-color: orange; }";
        } else {
            styleSheet = "QProgressBar::chunk { background-color: red; }";
        }
        usageBar->setStyleSheet(styleSheet);

        diskLayout->addWidget(usageBar);

        // Set column stretch to make the value column expand
        gridLayout->setColumnStretch(1, 1);

        // Add the disk group to the scroll layout
        scrollLayout->addWidget(diskGroup);
    }

    // Add stretch to push everything to the top
    scrollLayout->addStretch();

    // Set the scroll content and add to layout
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea);
}