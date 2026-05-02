#pragma once

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QLineEdit>
#include <deque>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include "worker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private slots:
    void onData(double t, double ping, double signal);
    void start();
    void stop();

private:
    QThread workerThread_;
    Worker* worker_ = nullptr;
    bool isWifi_{};

    QLineEdit* leIP_{};

    std::deque<double> times_;
    std::deque<double> pings_;
    std::deque<double> signals_;

    QLineSeries* pingSeries_{};
    QLineSeries* signalSeries_{};

    QChart* chart_{};
    QChart* chartS_{};
    QChartView *view_{};
    QChartView *viewS_{};
};