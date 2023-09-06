#include "udp.h"

udp::udp(QObject *parent) : QObject(parent){
    socket = new QUdpSocket(this);
}

bool udp::udpConnect(QHostAddress address, quint16 port){
    socket = new QUdpSocket(this);
    bool status = socket->bind(QHostAddress::Any, port);
//    bool status = socket->bind(address, port);
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataRecieveCallback()));
    return status;
}

void udp::udpDisConnect(){
    socket->close();
    socket->disconnect();
    socket->disconnectFromHost();
    socket->deleteLater();
    socket = nullptr;
}

void udp::sendData(QHostAddress address, quint16 port, QByteArray data){
    socket->writeDatagram(data, address, port);
}

void udp::dataRecieveCallback(){
    QHostAddress sender;
    quint16 senderPort;
    buffer.resize(socket->pendingDatagramSize());
    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    emit dataRecieveSignal();
}

QByteArray udp::getUdpData(){
    return buffer;
}
