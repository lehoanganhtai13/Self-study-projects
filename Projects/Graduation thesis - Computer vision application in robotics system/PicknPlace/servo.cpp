#include "servo.h"

servo::servo(QObject *parent)
{
    serial = new QSerialPort(this);
    serial->setPortName("COM4");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    timer = new QTimer(this);
}

servo::~servo(){
    delete serial;
    delete timer;
}

void servo::connect_UART(){
    if(!serial->open(QIODevice::ReadWrite)) {
        // handle error
        qDebug() << "COM Error";
    }

    connect(serial, SIGNAL(readyRead()), this, SLOT(read_data()));
    connect(timer, SIGNAL(timeout()), this, SLOT(ping()));
    timer->start(PING_INTERVAL);
}

void servo::disconnect_UART(){
    disconnect(serial, SIGNAL(readyRead()), this, SLOT(read_data()));
    disconnect(timer, SIGNAL(timeout()), this, SLOT(ping()));
    serial->close();

}

void servo::grab(){
    if(timer->isActive()){
        timer->stop();
    }

    uint16_t Header = 36055;
    uint8_t Command = GRAB;   // Command = 1: Servo Grab
    uint16_t CRC = (uint16_t)(Header + Command);

    qDebug() << "GRAB";

    // Prepare data to send
    tx_buff[0] = (uint8_t)(Header & 0x00FF);            // Header Low byte
    tx_buff[1] = (uint8_t)((Header >> 8) & 0x00FF);     // Header High byte
    tx_buff[2] = (uint8_t)(Command);
    tx_buff[3] = (uint8_t)(CRC & 0x00FF);               // CRC Low byte
    tx_buff[4] = (uint8_t)((CRC >> 8) & 0x00FF);        // CRC High byte

    QByteArray byteArray(reinterpret_cast<const char*>(tx_buff), sizeof(tx_buff));

    qDebug() << "Sent: " << byteArray;

    if (serial->write(byteArray) == -1) {
        // handle error
        qDebug() << "Failed to write data";
        return;
    }
    timer->start(PING_INTERVAL);
}

void servo::release(){
    if(timer->isActive()){
        timer->stop();
    }

    uint16_t Header = 36055;
    uint8_t Command = RELEASE;   // Command = 0: Release servo
    uint16_t CRC = (uint16_t)(Header + Command);

    qDebug() << "RELEASE";

    // Prepare data to send
    tx_buff[0] = (uint8_t)(Header & 0x00FF);            // Header Low byte
    tx_buff[1] = (uint8_t)((Header >> 8) & 0x00FF);     // Header High byte
    tx_buff[2] = (uint8_t)(Command);
    tx_buff[3] = (uint8_t)(CRC & 0x00FF);               // CRC Low byte
    tx_buff[4] = (uint8_t)((CRC >> 8) & 0x00FF);        // CRC High byte

    QByteArray byteArray(reinterpret_cast<const char*>(tx_buff), sizeof(tx_buff));
    qDebug () << "Sent: " << byteArray;

    if (serial->write(byteArray) == -1){
        // handle error
        qDebug() << "Failed to write data";
        return;
    }
    timer->start(PING_INTERVAL);
}

void servo::stopPing(){
    if(timer->isActive())
        timer->stop();
}

void servo::read_data(){
    QByteArray byteArray;
    if(serial->bytesAvailable() == sizeof(rx_buff)){
        byteArray = serial->readAll();
        memcpy(rx_buff, byteArray.constData(), sizeof(rx_buff));

//        qDebug () << "Size: " << byteArray.size();

//        qDebug () << "Received: " << byteArray;

        uint16_t Header_rx   = (rx_buff[1] << 8) | (rx_buff[0]);
        uint16_t Command_rx   = rx_buff[2];
        uint16_t Error_rx = rx_buff[3];
        uint16_t CRC_rx		= (rx_buff[5] << 8) | (rx_buff[4]);
        uint16_t CRC_check 	= Header_rx + Command_rx;

        if(Command_rx == PING){
            ping_count_rx++;
        } else if(Command_rx == GRAB){
            grab_count_rx++;
        } else if(Command_rx == RELEASE){
            release_count_rx++;
        }
    }
}

void servo::ping(){
    if (ping_count_tx != ping_count_rx){
        ping_count_tx = 0;
        ping_count_rx = 0;
        grab_count_tx = 0;
        grab_count_rx = 0;
        grab_count_tx = 0;
        grab_count_rx = 0;
        release_count_tx = 0;
        release_count_rx = 0;

        stopPing();
        emit pingFail();
        return;
    }

    uint16_t Header = 36055;
    uint8_t Command = PING;   // Command = 2: Ping
    uint16_t CRC = (uint16_t)(Header + Command);

    // Prepare data to send
    tx_buff[0] = (uint8_t)(Header & 0x00FF);            // Header Low byte
    tx_buff[1] = (uint8_t)((Header >> 8) & 0x00FF);     // Header High byte
    tx_buff[2] = (uint8_t)(Command);
    tx_buff[3] = (uint8_t)(CRC & 0x00FF);               // CRC Low byte
    tx_buff[4] = (uint8_t)((CRC >> 8) & 0x00FF);        // CRC High byte

    QByteArray byteArray(reinterpret_cast<const char*>(tx_buff), sizeof(tx_buff));
//    qDebug () << "Sent: ";
//    for (int i = 0; i < TX_BUFF_SIZE; i++){
//        qDebug() << tx_buff[i];
//    }

    if (serial->write(byteArray) == -1){
        // handle error
        qDebug() << "Failed to write data";
    }

    ping_count_tx++;
}
