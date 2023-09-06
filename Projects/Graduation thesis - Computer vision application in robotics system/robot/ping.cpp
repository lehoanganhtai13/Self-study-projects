#include "ping.h"

ping::ping(QObject *parent)
    : QObject{parent}
{
    connect(&process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(errorOccured(QProcess::ProcessError)));
    connect(&process, SIGNAL(readyReadStandardError()),this, SLOT(readyReadStandardError()));
    connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
//    connect(&process, SIGNAL(started()), this, SLOT(started()));
//    connect(&process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
    connect(&process, SIGNAL(readyRead()), this, SLOT(readyRead()));

    // finished signal is overloaded
    connect(&process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, &ping::finished);

    listening_status = false;
    ip_address = "";
}

void ping::setIpAddress(QString ip)
{
    ip_address = ip;
}

void ping::start(){
//    qInfo() << Q_FUNC_INFO;
    listening_status = true;
    QString cmd = "ping";
    QStringList args;
    args << "-w" << "3" << ip_address;
    process.start(cmd, args);
}

void ping::stop(){
//    qInfo() << Q_FUNC_INFO;
    listening_status = false;
    process.close();
}

void ping::errorOccured(QProcess::ProcessError error){
    if(!listening_status) return;
//    qInfo() << Q_FUNC_INFO << error;
    emit output("Error");
}

void ping::finished(int exit_code, QProcess::ExitStatus exit_status){
    if(!listening_status) return;
//    qInfo() << Q_FUNC_INFO;
//    Q_UNUSED(exit_code);
//    Q_UNUSED(exit_status);
    emit output("Finished");
}

void ping::readyReadStandardError(){
    if(!listening_status) return;
//    qInfo() << Q_FUNC_INFO;
    QString msg = "Standard errror: ";
    msg.append(process.readAllStandardError());
    emit output(msg);
}

void ping::readyReadStandardOutput(){
    if(!listening_status) return;
//    qInfo() << Q_FUNC_INFO;
    QByteArray data = process.readAllStandardOutput();
    emit output(data);
}

void ping::started(){
    // Do nothing
//    qInfo() << Q_FUNC_INFO;
}

void ping::stateChanged(QProcess::ProcessState new_change){
//    qInfo() << Q_FUNC_INFO;
    switch (new_change) {
    case QProcess::NotRunning:
        emit output("Not running..");
        break;
    case QProcess::Starting:
        emit output("Starting..");
        break;
    case QProcess::Running:
        emit output("Running..");
        break;
    }
}

void ping::readyRead(){
    if(!listening_status) return;
//    qInfo() << Q_FUNC_INFO;
    QByteArray data = process.readAll().trimmed();
//    qInfo() << data;
    emit output(data);
}
