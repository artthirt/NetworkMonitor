#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <atomic>
#include <vector>

struct NetSample {
    double time;
    double ping;
    double signal;
};

class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject* parent = nullptr);
    void stop();

    void setIp(const QString &ip) { mIp = ip; }

signals:
    void newData(double t, double ping, double signal);

public slots:
    void run();

private:
    QString mIp{"192.168.1.1"};
    std::atomic<bool> running{true};
    double pingHost(const char* ip);
    double getWifiSignal();
};

#endif