#include "cpu_info_widget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QLineEdit>
#include <QFont>
#include <QDateTime>

CPUInfoWidget::CPUInfoWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void CPUInfoWidget::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Create a tab widget to organize CPU information
    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);

    // Add tabs for different CPU sections
    tabWidget->addTab(createBasicInfoSection(), "Basic Info");
    tabWidget->addTab(createFrequencySection(), "Frequency");
    tabWidget->addTab(createCacheSection(), "Cache");
    tabWidget->addTab(createFlagsSection(), "CPU Flags");
}

QWidget* CPUInfoWidget::createBasicInfoSection() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QFont titleFont = widget->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    QFont valueFont = widget->font();
    valueFont.setPointSize(10);

    // Create a grid layout for the basic info
    QGridLayout *gridLayout = new QGridLayout();
    layout->addLayout(gridLayout);

    // Row 0: Processor Name
    QLabel *processorLabel = new QLabel("Processor:", widget);
    processorLabel->setFont(titleFont);
    gridLayout->addWidget(processorLabel, 0, 0);

    QLabel *processorValue = new QLabel(QString::fromStdString(cpuInfo.getProcessorName()), widget);
    processorValue->setFont(valueFont);
    processorValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    gridLayout->addWidget(processorValue, 0, 1);

    // Row 1: Vendor ID
    QLabel *vendorLabel = new QLabel("Vendor:", widget);
    vendorLabel->setFont(titleFont);
    gridLayout->addWidget(vendorLabel, 1, 0);

    QLabel *vendorValue = new QLabel(QString::fromStdString(cpuInfo.getVendorId()), widget);
    vendorValue->setFont(valueFont);
    vendorValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    gridLayout->addWidget(vendorValue, 1, 1);

    // Row 2: Physical Cores
    QLabel *physicalCoresLabel = new QLabel("Physical Cores:", widget);
    physicalCoresLabel->setFont(titleFont);
    gridLayout->addWidget(physicalCoresLabel, 2, 0);

    QLabel *physicalCoresValue = new QLabel(QString::number(cpuInfo.getPhysicalCores()), widget);
    physicalCoresValue->setFont(valueFont);
    gridLayout->addWidget(physicalCoresValue, 2, 1);

    // Row 3: Logical Cores
    QLabel *logicalCoresLabel = new QLabel("Logical Cores:", widget);
    logicalCoresLabel->setFont(titleFont);
    gridLayout->addWidget(logicalCoresLabel, 3, 0);

    QLabel *logicalCoresValue = new QLabel(QString::number(cpuInfo.getLogicalCores()), widget);
    logicalCoresValue->setFont(valueFont);
    gridLayout->addWidget(logicalCoresValue, 3, 1);

    // Set column stretch to make the value column expand
    gridLayout->setColumnStretch(1, 1);

    // Add stretch to push everything to the top
    layout->addStretch();

    return widget;
}

QWidget* CPUInfoWidget::createFrequencySection() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QFont titleFont = widget->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    QFont valueFont = widget->font();
    valueFont.setPointSize(10);

    // Create a grid layout for the frequency info
    QGridLayout *gridLayout = new QGridLayout();
    layout->addLayout(gridLayout);

    // Row 0: Current Frequency
    QLabel *currentFreqLabel = new QLabel("Current Frequency:", widget);
    currentFreqLabel->setFont(titleFont);
    gridLayout->addWidget(currentFreqLabel, 0, 0);

    QString currentFreqText = cpuInfo.getCurrentFrequencyMHz() > 0
        ? QString::number(cpuInfo.getCurrentFrequencyMHz()) + " MHz"
        : "Not available";
    QLabel *currentFreqValue = new QLabel(currentFreqText, widget);
    currentFreqValue->setFont(valueFont);
    gridLayout->addWidget(currentFreqValue, 0, 1);

    // Row 1: Minimum Frequency
    QLabel *minFreqLabel = new QLabel("Minimum Frequency:", widget);
    minFreqLabel->setFont(titleFont);
    gridLayout->addWidget(minFreqLabel, 1, 0);

    QString minFreqText = cpuInfo.getMinFrequencyMHz() > 0
        ? QString::number(cpuInfo.getMinFrequencyMHz()) + " MHz"
        : "Not available";
    QLabel *minFreqValue = new QLabel(minFreqText, widget);
    minFreqValue->setFont(valueFont);
    gridLayout->addWidget(minFreqValue, 1, 1);

    // Row 2: Maximum Frequency
    QLabel *maxFreqLabel = new QLabel("Maximum Frequency:", widget);
    maxFreqLabel->setFont(titleFont);
    gridLayout->addWidget(maxFreqLabel, 2, 0);

    QString maxFreqText = cpuInfo.getMaxFrequencyMHz() > 0
        ? QString::number(cpuInfo.getMaxFrequencyMHz()) + " MHz"
        : "Not available";
    QLabel *maxFreqValue = new QLabel(maxFreqText, widget);
    maxFreqValue->setFont(valueFont);
    gridLayout->addWidget(maxFreqValue, 2, 1);

    // Set column stretch to make the value column expand
    gridLayout->setColumnStretch(1, 1);

    // Add stretch to push everything to the top
    layout->addStretch();

    return widget;
}

QWidget* CPUInfoWidget::createCacheSection() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QFont titleFont = widget->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    // Create a table for cache information
    QTableWidget *cacheTable = new QTableWidget(widget);
    cacheTable->setColumnCount(2);
    cacheTable->setHorizontalHeaderLabels(QStringList() << "Cache Type" << "Size");
    cacheTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    cacheTable->verticalHeader()->setVisible(false);
    cacheTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cacheTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Get cache information
    std::map<std::string, size_t> cacheSizes = cpuInfo.getCacheSizes();
    cacheTable->setRowCount(cacheSizes.size());

    int row = 0;
    for (const auto& [cacheType, size] : cacheSizes) {
        // Cache type
        QTableWidgetItem *typeItem = new QTableWidgetItem(QString::fromStdString(cacheType));
        cacheTable->setItem(row, 0, typeItem);

        // Cache size
        QString sizeText;
        if (size >= 1024 * 1024) {
            sizeText = QString::number(size / (1024 * 1024)) + " MB";
        } else if (size >= 1024) {
            sizeText = QString::number(size / 1024) + " KB";
        } else {
            sizeText = QString::number(size) + " bytes";
        }

        QTableWidgetItem *sizeItem = new QTableWidgetItem(sizeText);
        cacheTable->setItem(row, 1, sizeItem);

        row++;
    }

    layout->addWidget(cacheTable);

    return widget;
}

QWidget* CPUInfoWidget::createFlagsSection() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QFont titleFont = widget->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);

    // Create a list widget for CPU flags
    QListWidget *flagsList = new QListWidget(widget);
    flagsList->setSelectionMode(QAbstractItemView::SingleSelection);
    flagsList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Get CPU flags
    std::vector<std::string> flags = cpuInfo.getCPUFlags();
    for (const auto& flag : flags) {
        flagsList->addItem(QString::fromStdString(flag));
    }

    // Add a search box for flags
    QLabel *searchLabel = new QLabel("Search Flags:", widget);
    searchLabel->setFont(titleFont);
    QLineEdit *searchBox = new QLineEdit(widget);

    // Connect search box to filter the list
    QObject::connect(searchBox, &QLineEdit::textChanged, [flagsList, flags](const QString &text) {
        flagsList->clear();
        for (const auto& flag : flags) {
            if (text.isEmpty() || QString::fromStdString(flag).contains(text, Qt::CaseInsensitive)) {
                flagsList->addItem(QString::fromStdString(flag));
            }
        }
    });

    layout->addWidget(searchLabel);
    layout->addWidget(searchBox);
    layout->addWidget(flagsList);

    return widget;
}

void CPUInfoWidget::refreshData() {
    // Update CPU information
    cpuInfo.refresh();

    // Update UI with new data
    updateCPUInfo();
    updateCacheInfo();
    updateFlagsInfo();

    // Update last refresh time
    QDateTime now = QDateTime::currentDateTime();
    lastUpdateLabel->setText("Last updated: " + now.toString("yyyy-MM-dd hh:mm:ss"));
}

void CPUInfoWidget::updateCPUInfo() {
    // Update basic CPU information in the UI
    QWidget* basicInfoTab = tabWidget->widget(0); // Get the Basic Info tab

    // Find and update the processor name label
    QList<QLabel*> labels = basicInfoTab->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text().contains("Processor:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::fromStdString(cpuInfo.getProcessorName()));
                    }
                }
            }
        } else if (label->text().contains("Vendor:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::fromStdString(cpuInfo.getVendorId()));
                    }
                }
            }
        } else if (label->text().contains("Physical Cores:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::number(cpuInfo.getPhysicalCores()));
                    }
                }
            }
        } else if (label->text().contains("Logical Cores:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::number(cpuInfo.getLogicalCores()));
                    }
                }
            }
        }
    }

    // Update frequency information in the Frequency tab
    QWidget* freqTab = tabWidget->widget(1); // Get the Frequency tab
    labels = freqTab->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text().contains("Current Frequency:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::number(cpuInfo.getCurrentFrequencyMHz(), 'f', 2) + " MHz");
                    }
                }
            }
        } else if (label->text().contains("Minimum Frequency:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::number(cpuInfo.getMinFrequencyMHz(), 'f', 2) + " MHz");
                    }
                }
            }
        } else if (label->text().contains("Maximum Frequency:")) {
            // Find the value label (next in the grid layout)
            QGridLayout* grid = qobject_cast<QGridLayout*>(label->parentWidget()->layout());
            if (grid) {
                int row, column, rowSpan, columnSpan;
                grid->getItemPosition(grid->indexOf(label), &row, &column, &rowSpan, &columnSpan);
                QLayoutItem* item = grid->itemAtPosition(row, column + 1);
                if (item && item->widget()) {
                    QLabel* valueLabel = qobject_cast<QLabel*>(item->widget());
                    if (valueLabel) {
                        valueLabel->setText(QString::number(cpuInfo.getMaxFrequencyMHz(), 'f', 2) + " MHz");
                    }
                }
            }
        }
    }
}

void CPUInfoWidget::updateCacheInfo() {
    // Update cache information in the Cache tab
    cacheInfoTable->clearContents();

    // Get cache information
    std::map<std::string, size_t> cacheSizes = cpuInfo.getCacheSizes();
    cacheInfoTable->setRowCount(cacheSizes.size());

    int row = 0;
    for (const auto& [cacheType, size] : cacheSizes) {
        // Cache type
        QTableWidgetItem *typeItem = new QTableWidgetItem(QString::fromStdString(cacheType));
        cacheInfoTable->setItem(row, 0, typeItem);

        // Cache size
        QString sizeText;
        if (size >= 1024 * 1024) {
            sizeText = QString::number(size / (1024 * 1024)) + " MB";
        } else if (size >= 1024) {
            sizeText = QString::number(size / 1024) + " KB";
        } else {
            sizeText = QString::number(size) + " bytes";
        }

        QTableWidgetItem *sizeItem = new QTableWidgetItem(sizeText);
        cacheInfoTable->setItem(row, 1, sizeItem);

        row++;
    }
}

void CPUInfoWidget::updateFlagsInfo() {
    // Update CPU flags in the Flags tab
    flagsTable->clearContents();

    // Get CPU flags
    std::vector<std::string> flags = cpuInfo.getCPUFlags();

    // Calculate how many rows we need (assuming 4 columns)
    int numColumns = 4;
    int numRows = (flags.size() + numColumns - 1) / numColumns;

    flagsTable->setRowCount(numRows);

    for (size_t i = 0; i < flags.size(); ++i) {
        int row = i / numColumns;
        int col = i % numColumns;

        QTableWidgetItem *flagItem = new QTableWidgetItem(QString::fromStdString(flags[i]));
        flagsTable->setItem(row, col, flagItem);
    }
}