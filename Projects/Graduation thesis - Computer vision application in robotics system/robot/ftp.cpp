#include "ftp.h"

ftp::ftp(QObject *parent)
    : QObject{parent}
{

}

void ftp::ftpConnect(QString host, QString username, QString password, int port)
{
    url.setScheme("ftp");
    url.setHost(host);
    url.setUserName(username);
    url.setPassword(password);
    url.setPort(port);
    return;
}

void ftp::ftpLoad(QString dir, QString filename)
{
    qDebug() << "load";
    qDebug() << dir;
    file.setFileName(dir);

    QString name;
    name.append("/JOB/");
    name.append(filename);
    qDebug() << name;
    url.setPath(name);
    QNetworkRequest request(url);
    QByteArray byte;

    if(file.open(QIODevice::ReadOnly)){
        qDebug() << "opened";
        byte = file.readAll();
        reply = manager->put(request, byte);
    }

    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(loadError(QNetworkReply::NetworkError)));
}

void ftp::test()
{
    QString filename = "A.JBI";
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QUrl url("ftp://192.168.255.1/JOB/");
    qDebug() << "ftp://192.168.255.1/JOB";
    url.setPassword("99999999");
    url.setUserName("rcmaster");
    url.setPort(21);

    QNetworkAccessManager *man = new QNetworkAccessManager(this);
    QNetworkRequest req(url);
    reply = man->put(req, &file);
    file.close();
    connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(loadError(QNetworkReply::NetworkError)));
}

void ftp::uploadProgress(qint64 byte_sent, qint64 total_byte)
{
    qDebug() << "Uploaded" << byte_sent << "of" << total_byte << "bytes";
}

void ftp::replyFinished(QNetworkReply *)
{
    if(reply->error() == QNetworkReply::NoError){
        qDebug() << "Transfered succesfully";
    } else{
        qDebug() << "Failed to transfer";
    }

    reply->deleteLater();
    file.flush();
    file.close();
    file.deleteLater();
}

void ftp::loadError(QNetworkReply::NetworkError error)
{
    qDebug() << "Error: " << error;
}
