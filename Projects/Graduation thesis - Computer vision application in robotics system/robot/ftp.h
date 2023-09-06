#ifndef FTP_H
#define FTP_H

#include <QObject>
#include <QFile>
#include <QUrl>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QDebug>

class ftp : public QObject
{
    Q_OBJECT
public:
    explicit ftp(QObject *parent = nullptr);

    void ftpConnect(QString host, QString username, QString password, int port);
    void ftpLoad(QString dir, QString filename);
    void ftprSave(QString dir, QString filename);
    void test();

private:
    QFile file;
    QUrl url;
    QNetworkReply *reply;
    QNetworkAccessManager *manager;
    QString ftp_save_dir;

public slots:
    void uploadProgress(qint64 byte_sent,qint64 total_byte);
    void replyFinished(QNetworkReply*);
    void loadError(QNetworkReply::NetworkError error);

signals:

};

#endif // FTP_H
