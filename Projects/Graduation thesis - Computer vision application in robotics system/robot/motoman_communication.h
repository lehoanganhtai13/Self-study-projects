//#ifndef MOTOMAN_COMMUNICATION_H
//#define MOTOMAN_COMMUNICATION_H

//#include <QObject>

//class motoman_communication : public QObject
//{
//    Q_OBJECT
//public:
//    explicit motoman_communication(QObject *parent = nullptr);

//signals:

//};

//#endif // MOTOMAN_COMMUNICATION_H

#ifndef MOTOMAN_COMMUNICATION_H
#define MOTOMAN_COMMUNICATION_H

#include <QObject>
#include <udp.h>
#include <motoman_command.h>
#include <QVector>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QMatrix4x4>
#include <QtMath>
#include <QTime>
#include <QApplication>
#include <QDir>
#include <fileapi.h>


class motoman_communication : public QObject
{
    Q_OBJECT
public:
    explicit motoman_communication(QObject *parent = nullptr);
    ~motoman_communication();
    bool motomanConnect();
    void motomanDisconnect();
    bool motomanFIleCOntrolConnect();
    void motomanFileControlDisconnect();
    void motomanOnServo();
    bool motomanOnServoResponse(QByteArray);
    void motomanOffServo();
    bool motomanOffServoResponse(QByteArray);
    void motomanReadControlMode();
    quint8 motomanReadControlModeResponse(QByteArray);
    void motomanReadPosition();
    void motomanReadPositionResponse(QByteArray);
    void motomanReadPulse();
    void motomanReadPulseResponse(QByteArray);
    void motomanReadStatus();
    void motomanReadStatusResponse(QByteArray);
    void motomanSetConnection(QHostAddress, quint16, quint16);
    void motomanMoveCartesian(quint8, quint8, quint8, double, QVector<double> *);
    void motomanSelectJob(QString, quint32);
    bool motomanSelectJobResponse(QByteArray);
    void motomanStartJob();
    bool motomanStartJobResponse(QByteArray);
    void motomanReadJob(quint8, QString);
    void motomanReadJobResponse(QByteArray);
    void motomanTeachPosition(quint8, int, QVector<double> *);
    bool motomanTeachPositionResponse(QByteArray);
    void motomanWriteByte(int, quint8);
    bool motomanWriteByteResponse(QByteArray);
    void motomanTeachPluralPosition(quint8,int, int, QVector<double> *);
    bool motomanTeachPluralPositionResponse(QByteArray);
    void motomanReadIO();
    bool motomanReadIOResponse(QByteArray);
    void motomanSaveFile(QString);
    void motomanSaveFileResponse(QByteArray);
    void motomanLoadFile(QString);
    void motomanLoadFileResponse(QByteArray, quint32);
    void motomanListFile(QString file_extension);
    QStringList motomanListFileResponse(QByteArray);
    void motomanDeleteFile(QString);
    bool motomanDeleteFileResponse(QByteArray data);
    QMatrix4x4 motomanTransformationMatrix(double x, double y, double z, double Rx, double Ry, double Rz);
    QVector<double> motomanUpdatePosition();
    QVector<double> motomanUpdatePulse();
    QVector<double> motomanUpdateDegree();
    quint8 motomanUpdateStatus();
    quint8 motomanUpdateIO();
    QByteArray test;

signals:
    void dataRecieveUISignal(quint8, bool, QStringList);
    void writeByteSignal(int);
    void selectJobSignal(QString);
    void startJob();

public slots:
    void motomanDataCallback();

protected slots:
    void motomanFileCallback();

private:
    udp server;
    udp file_control;

    quint16 server_port;
    quint16 file_control_port;
    QHostAddress udp_address;

    quint8 request_id_index;
    quint8 response_id_index;
    char request_command[256];
    quint8 file_response_id_index;

    motoman_command motoman_command;

    int read_robot_position_type;
    int read_stauts_type;
    int turn_servo_mode;
    QVector<double> robot_position;
    QVector<double> robot_pulse;
    QVector<double> robot_degree;

    quint8 running_status;
    quint8 control_mode;

    quint8 io_signal;

    QString name;

    bool response_signal;

    quint32 load_file_block_num;
    QString str_file;
    bool load_file_status;

    QString job_information;

public:
    bool job_select_status_error;
    bool job_start_status_error;
    int byte;
    QString job;
    QString motoman_controller;
    QStringList list;
    QString name_info;
    quint32 info;

};

#endif // MOTOMAN_COMMUNICATION_H
