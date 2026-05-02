#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

MainWindow::MainWindow() {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* startBtn = new QPushButton("Start");
    auto* stopBtn = new QPushButton("Stop");

    isWifi_ = Worker::isWifiConnected();

    auto hlIp = new QHBoxLayout;
    hlIp->addWidget(new QLabel("IP"));
    leIP_ = new QLineEdit;
    leIP_->setPlaceholderText("192.168.1.1");
    leIP_->setText("192.168.1.1");
    hlIp->addWidget(leIP_);
    layout->addLayout(hlIp);

    auto hl = new QHBoxLayout;

    hl->addWidget(startBtn);
    hl->addWidget(stopBtn);
    layout->addLayout(hl);

    chart_ = new QChart();
    pingSeries_ = new QLineSeries();
    pingSeries_->setName("Ping (ms)");
    chart_->addSeries(pingSeries_);
    chart_->createDefaultAxes();
    view_ = new QChartView(chart_);
    layout->addWidget(view_);

    if(isWifi_){
        chartS_ = new QChart();
        signalSeries_ = new QLineSeries();
        signalSeries_->setName("Wifi Signal (dB)");
        chartS_->addSeries(signalSeries_);
        chartS_->createDefaultAxes();
        viewS_ = new QChartView(chartS_);
        layout->addWidget(viewS_);
    }

    setCentralWidget(central);

    pings_.push_back(0);
    signals_.push_back(0);

    connect(startBtn, &QPushButton::clicked, this, &MainWindow::start);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stop);
}

MainWindow::~MainWindow() {
    stop();
}

void MainWindow::start() {
    worker_ = new Worker();
    worker_->moveToThread(&workerThread_);

    worker_->setIp(leIP_->text());

    connect(&workerThread_, &QThread::started, worker_, &Worker::run);
    connect(worker_, &Worker::newData, this, &MainWindow::onData);
    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);

    workerThread_.start();
}

void MainWindow::stop() {
    if (worker_) {
        worker_->stop();
        workerThread_.quit();
        workerThread_.wait();
        worker_ = nullptr;
    }
}

void MainWindow::onData(double t, double ping, double signal) {
    times_.push_back(t);
    pings_.push_back(ping);
    if(isWifi_)
        signals_.push_back(signal);

    if (times_.size() > 60) {
        times_.pop_front();
        pings_.pop_front();
        if(isWifi_)
            signals_.pop_front();
    }

    qDebug("ping %.2f, signal %.2f", ping, signal);

    {
        pingSeries_->clear();
        double min = ping, max = ping;
        for (size_t i = 0; i < times_.size(); i++) {
            pingSeries_->append(times_[i], pings_[i]);
            min = std::min(min, pings_[i]);
            max = std::max(max, pings_[i]);
        }
        chart_->axes(Qt::Horizontal).first()->setRange(times_.front(), times_.back());
        chart_->axes(Qt::Vertical).first()->setRange(min, max);
        chart_->update();
        chart_->zoomReset();
        view_->repaint();
    }

    if(isWifi_){
        signalSeries_->clear();
        double min = signal, max = signal;
        for (size_t i = 0; i < times_.size(); i++) {
            signalSeries_->append(times_[i], signals_[i]);
            min = std::min(min, signals_[i]);
            max = std::max(max, signals_[i]);
        }
        chartS_->axes(Qt::Horizontal).first()->setRange(times_.front(), times_.back());
        chartS_->axes(Qt::Vertical).first()->setRange(min, max + 1);

        chartS_->update();
        chartS_->zoomReset();
        viewS_->repaint();
    }
}