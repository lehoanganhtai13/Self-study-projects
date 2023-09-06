#ifndef SERVO_H
#define SERVO_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QDebug>

#define TX_BUFF_SIZE    5
#define RX_BUFF_SIZE    6
#define PING_INTERVAL   2000

#define RELEASE 0
#define GRAB 1
#define PING 2

class servo : public QObject
{
Q_OBJECT
public:
    servo(QObject *parent = nullptr);
    ~servo();

    void connect_UART();
    void disconnect_UART();
    void grab();
    void release();
    void stopPing();
public slots:
    void ping();
    void read_data();

signals:
    void pingFail();

private:
    QSerialPort* serial;
    QTimer* timer;

    uint32_t ping_count_tx = 0;
    uint32_t ping_count_rx = 0;
    uint32_t grab_count_tx = 0;
    uint32_t grab_count_rx = 0;
    uint32_t release_count_tx = 0;
    uint32_t release_count_rx = 0;

    uint8_t tx_buff[TX_BUFF_SIZE];
    uint8_t rx_buff[RX_BUFF_SIZE];  //Header, Command, CRC
};

#endif // SERVO_H
