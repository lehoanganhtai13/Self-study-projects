#ifndef LICENSE_SERVER_H
#define LICENSE_SERVER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QSslConfiguration>
#include <QDate>
#include <QDir>
#include <fileapi.h>
#include <QMessageBox>
#include <QTime>
#include <QDateTime>

class license_server : public QObject
{
    Q_OBJECT
public:
    explicit license_server(QObject *parent = nullptr);
    void getIp();
    void login(QString email, QString password);
    int licenseProcess(QString key);
    bool checkTimerCheating();
    void unlockConsider();

private:
    QString ip;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    bool change;
    bool consider_key;

public slots:
    void uploadProgress(qint64 byte_sent,qint64 total_byte);
    void finishReply(QNetworkReply*);
    void showError(QNetworkReply::NetworkError error);
    void downloadProgress(qint64 byte_received,qint64 total_byte);
    void readCallback();
    void finishReplydown(QNetworkReply*);
    void showErrordown(QNetworkReply::NetworkError error);

signals:
    void confirmAutossManagement(bool);

};

#endif // LICENSE_SERVER_H
