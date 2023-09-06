#ifndef PING_H
#define PING_H

#include <QObject>
#include <QSysInfo>
#include <QProcess>
#include <QDebug>

class ping : public QObject
{
    Q_OBJECT
public:
    explicit ping(QObject *parent = nullptr);

    void setIpAddress(QString ip);

signals:
    void output(QString data);

public slots:
    void start();
    void stop();

private slots:
    void errorOccured(QProcess::ProcessError error);
    void finished(int exit_code, QProcess::ExitStatus exit_status);
    void readyReadStandardError();
    void readyReadStandardOutput();
    void started();
    void stateChanged(QProcess::ProcessState new_change);
    void readyRead();

private:
    QProcess process;
    QString ip_address;
    bool listening_status;

    QString getProcess();
    void startPing();
};

#endif // PING_H
