#ifndef SERVICE_WINDOW_H
#define SERVICE_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QTimer>
#include <QComboBox>
#include <QTextEdit>
#include "service_info.h"

class ServiceWindow : public QWidget {
    Q_OBJECT

public:
    explicit ServiceWindow(QWidget *parent = nullptr);
    ~ServiceWindow() = default;

private slots:
    void refreshData();
    void filterServices(const QString& text);
    void serviceSelected(int row, int column);
    void startSelectedService();
    void stopSelectedService();
    void restartSelectedService();
    void enableSelectedService();
    void disableSelectedService();

private:
    void setupUI();
    void updateTable();
    void updateServiceDetails(const ServiceInfo& service);
    void showMessage(const QString& message, bool isError = false);

    ServiceInfoCollector serviceInfo;
    QTableWidget *serviceTable;
    QLineEdit *filterEdit;
    QPushButton *refreshButton;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *restartButton;
    QPushButton *enableButton;
    QPushButton *disableButton;
    QLabel *statusLabel;
    QLabel *lastUpdateLabel;
    QTimer *autoRefreshTimer;
    QTextEdit *detailsTextEdit;

    std::string selectedService;
};

#endif // SERVICE_WINDOW_H