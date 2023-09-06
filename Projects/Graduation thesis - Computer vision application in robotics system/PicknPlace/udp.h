#ifndef UDP_H
#define UDP_H

#include <QObject>
#include <QUdpSocket>
#include <QIODevice>

class udp : public QObject
{
    Q_OBJECT
public:
    explicit udp(QObject *parent = nullptr);
    bool udpConnect(QHostAddress, quint16);
    void udpDisConnect();
    void sendData(QHostAddress, quint16, QByteArray);
    QByteArray getUdpData();

signals:
    void dataRecieveSignal();

public slots:
    void dataRecieveCallback();

private:
    QByteArray buffer;

public:
    QUdpSocket *socket;
};

#endif // UDP_H
