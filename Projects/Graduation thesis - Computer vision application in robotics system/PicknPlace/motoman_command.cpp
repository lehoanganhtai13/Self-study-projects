#include "motoman_command.h"
#include <string>
#include <iostream>

motoman_command::motoman_command(QObject *parent) : QObject(parent)
{
    /*
     * Initializing the high-speed Ethernet server transmission function packet for "request" consists of
     * header part (32 Byte) and data part (479 Byte max) when the servo off.
     *
     * When adding new character to the array, for each component (), it will be arranged in reverse order (big-endian byte order)
     * if only one single byte is appended to n-Bytes part (n > 1)
     * Example: "0x00" - "0x20" -> "0x20" + "0x00"
     *
     * Data part size, Attribute, Instance, and Service will follow Command No.
    */
    header_part.append('\x59'); // Identifier: " YERC"
    header_part.append('\x45');
    header_part.append('\x52');
    header_part.append('\x43');

    header_part.append('\x20'); // Header part size: fixed to "0x20"
    header_part.append('\x00');

    header_part.append('\x04'); // Data part size: size for data is 4 Byte ("0x04")
    header_part.append('\x00');

    header_part.append('\x03'); // Reserve 1: fixed to "0x03"

    header_part.append('\x01'); // Processing division: robot control ("0x01") or  file control ("0x02")

    header_part.append('\x00'); // ACK: request ("0x00") or others ("0x01")

    header_part.append('\x01'); // Request ID

    header_part.append('\x00'); // Block No. : request ("0x00")
    header_part.append('\x00');
    header_part.append('\x00');
    header_part.append('\x00');

    header_part.append('\x39'); // Reserve 2: "999999999"
    header_part.append('\x39');
    header_part.append('\x39');
    header_part.append('\x39');
    header_part.append('\x39');
    header_part.append('\x39');
    header_part.append('\x39');
    header_part.append('\x39');

    header_part.append('\x83'); // Command No. : Servo off ("0x83")
    header_part.append('\x00');

    header_part.append('\x02'); // Instance: servo on ("0x02")
    header_part.append('\x00');

    header_part.append('\x01'); // Attribute: fixed to "0x01"

    header_part.append('\x10'); // Service: fixed to "0x10"

    header_part.append('\x00'); // Padding: fixed to "0x0000"
    header_part.append('\x00');

    data_part.append('\x00');   // Data content
    data_part.append('\x00');
    data_part.append('\x00');
    data_part.append('\x00');

    for(int i = 0; i < 104; i++){
        cartesian_move_data_part.append('\x00');
    }
    cartesian_move_data_part[DATA_MOVE_ROBOT_NUM - HEADER_SIZE] = '\x01';         // Default robot No. 1
    cartesian_move_data_part[DATA_MOVE_COORDINATE_NUM - HEADER_SIZE] = '\x01';     // Defualt coordinate No. 1

    for(int i = 0; i < 36; i++){
        job_data_part.append('\x00');
    }

    for(int i = 0; i < 52; i++){
        teach_position_data_part.append('\x00');
    }

    wrtie_byte_data_part.append("\x00");
}

QByteArray motoman_command::setServoOn(quint8 request_id_index){
    QByteArray cmd = header_part + data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_SERVO_ON;
    cmd[CMD_INSTANCE] = CMD_HEADER_SERVO_INSTANCE_ON;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_SERVO_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_SERVO_SERVICE;
    cmd[DATA_BYTE0] = CMD_DATA_SERVO_ON;
    return cmd;
}

QByteArray motoman_command::setServoOff(quint8 request_id_index){
    QByteArray cmd = header_part + data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_SERVO_ON;
    cmd[CMD_INSTANCE] = CMD_HEADER_SERVO_INSTANCE_ON;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_SERVO_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_SERVO_SERVICE;
    cmd[DATA_BYTE0] = CMD_DATA_SERVO_OFF;
    return cmd;
}

QByteArray motoman_command::readControlMode(quint8 request_id_index){
    /*
     * For this function there will be no data part
    */
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_STATUS_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_STATUS;
    cmd[CMD_INSTANCE] = CMD_HEADER_READ_STATUS_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_STATUS_ATTRIBUTE_DATA_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_STATUS_SERVICE_ALL;
    return cmd;
}

QByteArray motoman_command::readRobotPosition(quint8 request_id_index){
    /*
     * For this function there will be no data part
    */
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_POSITION_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_ROBOT_POSITION;
    cmd[CMD_INSTANCE] = CMD_HEADER_READ_POSITION_INSTANCE_CARTESIAN;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_POSITION_ATTRIBUTE_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_POSITION_SERVICE_ALL;
    return cmd;
}

QByteArray motoman_command::readRobotPulse(quint8 request_id_index){
    /*
     * For this function there will be no data part
    */
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_POSITION_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_ROBOT_POSITION;
    cmd[CMD_INSTANCE] = CMD_HEADER_READ_POSITION_INSTANCE_PULSE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_POSITION_ATTRIBUTE_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_POSITION_SERVICE_ALL;
    return cmd;
}

QByteArray motoman_command::readStatus(quint8 request_id_index){
    /*
     * For this function there will be no data part
    */
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_STATUS_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_STATUS;
    cmd[CMD_INSTANCE] = CMD_HEADER_READ_STATUS_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_STATUS_ATTRIBUTE_DATA_1;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_STATUS_SERVICE_ALL;
    return cmd;
}

QByteArray motoman_command::setRobotCartesianPosition(quint8 request_id_index, quint8 coordinate_type, quint8 move_type, quint8 speed_type, double speed, QVector<double> *position){
    QByteArray cmd = header_part + cartesian_move_data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_MOVE_CARTESIAN_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_MOVE_CARTESIAN;
    cmd[CMD_INSTANCE] = move_type;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_MOVE_CARTESIAN_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_MOVE_CARTESIAN_SERVICE;

    cmd[DATA_MOVE_CORDINATE_TYPE] = coordinate_type;
    cmd[DATA_MOVE_SPEED_TYPE] = speed_type;

    quint32 speed_in = (quint32)speed*10;

    quint32 x_in = (quint32)(position->at(0)*1000);
    quint32 y_in = (quint32)(position->at(1)*1000);
    quint32 z_in = (quint32)(position->at(2)*1000);
    quint32 roll_in = (quint32)(position->at(3)*10000);
    quint32 pitch_in = (quint32)(position->at(4)*10000);
    quint32 yaw_in = (quint32)(position->at(5)*10000);

    cmd[DATA_MOVE_SPEED + 3] = (quint8)(speed_in >> 24);
    cmd[DATA_MOVE_SPEED + 2] = (quint8)(speed_in >> 16);
    cmd[DATA_MOVE_SPEED + 1] = (quint8)(speed_in >> 8);
    cmd[DATA_MOVE_SPEED] = (quint8)speed_in;

    cmd[DATA_MOVE_X_AXIS + 3] = (quint8)(x_in >> 24);
    cmd[DATA_MOVE_X_AXIS + 2] = (quint8)(x_in >> 16);
    cmd[DATA_MOVE_X_AXIS + 1] = (quint8)(x_in >> 8);
    cmd[DATA_MOVE_X_AXIS] = (quint8)x_in;

    cmd[DATA_MOVE_Y_AXIS + 3] = (quint8)(y_in >> 24);
    cmd[DATA_MOVE_Y_AXIS + 2] = (quint8)(y_in >> 16);
    cmd[DATA_MOVE_Y_AXIS + 1] = (quint8)(y_in >> 8);
    cmd[DATA_MOVE_Y_AXIS] = (quint8)y_in;

    cmd[DATA_MOVE_Z_AXIS + 3] = (quint8)(z_in >> 24);
    cmd[DATA_MOVE_Z_AXIS + 2] = (quint8)(z_in >> 16);
    cmd[DATA_MOVE_Z_AXIS + 1] = (quint8)(z_in >> 8);
    cmd[DATA_MOVE_Z_AXIS] = (quint8)z_in;

    cmd[DATA_MOVE_RX + 3] = (quint8)(roll_in >> 24);
    cmd[DATA_MOVE_RX + 2] = (quint8)(roll_in >> 16);
    cmd[DATA_MOVE_RX + 1] = (quint8)(roll_in >> 8);
    cmd[DATA_MOVE_RX] = (quint8)roll_in;

    cmd[DATA_MOVE_RY + 3] = (quint8)(pitch_in >> 24);
    cmd[DATA_MOVE_RY + 2] = (quint8)(pitch_in >> 16);
    cmd[DATA_MOVE_RY + 1] = (quint8)(pitch_in >> 8);
    cmd[DATA_MOVE_RY] = (quint8)pitch_in;

    cmd[DATA_MOVE_RZ + 3] = (quint8)(yaw_in >> 24);
    cmd[DATA_MOVE_RZ + 2] = (quint8)(yaw_in >> 16);
    cmd[DATA_MOVE_RZ + 1] = (quint8)(yaw_in >> 8);
    cmd[DATA_MOVE_RZ] = (quint8)yaw_in;
    return cmd;
}

QByteArray motoman_command::selectJob(quint8 request_id_index, QString job_name, quint32 line){
    QByteArray cmd = header_part + job_data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_SELECT_JOB_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_SELECT_JOB;
    cmd[CMD_INSTANCE] = CMD_HEADER_SELECT_JOB_INSTANCE_SET_EXE_JOB;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_SELECT_JOB_ATTRIBUTE_JOB_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_SELECT_JOB_SERVICE_ALL;

    QByteArray name = job_name.toUtf8();
    for (int i = 0; i < job_name.size(); i++) {
        cmd[DATA_SELECT_JOB_NAME + i] = name[i];
    }

    cmd[DATA_SELECT_JOB_LINE_NUM] = (quint8)line;
    cmd[DATA_SELECT_JOB_LINE_NUM + 1] = (quint8)(line >> 8);
    cmd[DATA_SELECT_JOB_LINE_NUM + 2] = (quint8)(line >> 16);
    cmd[DATA_SELECT_JOB_LINE_NUM + 3] = (quint8)(line >> 24);

    return cmd;
}

QByteArray motoman_command::startJob(quint8 request_id_index){
    QByteArray cmd = header_part + data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_START_JOB_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_START_JOB;
    cmd[CMD_INSTANCE] = CMD_HEADER_START_JOB_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_START_JOB_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_START_JOB_SERVICE;
    cmd[DATA_BYTE0] = CMD_DATA_START_JOB;
    return cmd;
}

QByteArray motoman_command::readJob(quint8 request_id_index, quint8 job_index){
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_JOB_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_JOB_INFORMATION;
    cmd[CMD_INSTANCE] = job_index;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_JOB_ATTRIBUTE_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_JOB_SERVICE_ALL;
    return cmd;
}

QByteArray motoman_command::teachRobotPosition(quint8 request_id_index, quint8 coordinate_type, int index, QVector<double> *position){
    QByteArray cmd = header_part + teach_position_data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_TEACH_POSITION_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_TEACH_ROBOT_POSITION;
    cmd[CMD_INSTANCE] = index;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_TEACH_POSITION_ATTRIBUTE_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_TEACH_POSITION_SERVICE_WRITE;

    cmd[DATA_TEACH_POSITION_DATA_TYPE] = coordinate_type;
    cmd[DATA_TEACH_POSITION_COORDINATE_NUM] = '\x01';

    quint32 axis1 = (quint32)(position->at(0)*1000);
    quint32 axis2 = (quint32)(position->at(1)*1000);
    quint32 axis3 = (quint32)(position->at(2)*1000);
    quint32 axis4 = (quint32)(position->at(3)*10000);
    quint32 axis5 = (quint32)(position->at(4)*10000);
    quint32 axis6 = (quint32)(position->at(5)*10000);

    cmd[DATA_TEACH_POSITION_COORDINATE_1 + 3] = (quint8)(axis1 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_1 + 2] = (quint8)(axis1 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_1 + 1] = (quint8)(axis1 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_1] = (quint8)axis1;

    cmd[DATA_TEACH_POSITION_COORDINATE_2 + 3] = (quint8)(axis2 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_2 + 2] = (quint8)(axis2 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_2 + 1] = (quint8)(axis2 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_2] = (quint8)axis2;

    cmd[DATA_TEACH_POSITION_COORDINATE_3 + 3] = (quint8)(axis3 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_3 + 2] = (quint8)(axis3 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_3 + 1] = (quint8)(axis3 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_3] = (quint8)axis3;

    cmd[DATA_TEACH_POSITION_COORDINATE_4 + 3] = (quint8)(axis4 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_4 + 2] = (quint8)(axis4 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_4 + 1] = (quint8)(axis4 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_4] = (quint8)axis4;

    cmd[DATA_TEACH_POSITION_COORDINATE_5 + 3] = (quint8)(axis5 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_5 + 2] = (quint8)(axis5 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_5 + 1] = (quint8)(axis5 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_5] = (quint8)axis5;

    cmd[DATA_TEACH_POSITION_COORDINATE_6 + 3] = (quint8)(axis6 >> 24);
    cmd[DATA_TEACH_POSITION_COORDINATE_6 + 2] = (quint8)(axis6 >> 16);
    cmd[DATA_TEACH_POSITION_COORDINATE_6 + 1] = (quint8)(axis6 >> 8);
    cmd[DATA_TEACH_POSITION_COORDINATE_6] = (quint8)axis6;
    return cmd;
}

QByteArray motoman_command::writeByte(quint8 request_id_index, int index, quint8 data_byte){
    QByteArray cmd = header_part + wrtie_byte_data_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_WRITE_BYTE_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_WRITE_BYTE;
    cmd[CMD_INSTANCE] = index;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_WRITE_BYTE_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_WRITE_BYTE_SERVICE_SET_SINGLE;
    cmd[DATA_BYTE0] = data_byte;
    return cmd;
}

QByteArray motoman_command::teachPluralRobotPosition(quint8 request_id_index, quint8 coordinate_type, int numb, int index, QVector<double> *position){
    for(int i = 0; i < (numb*13 + 1)* 4; i++){
        teach_plural_position_data_part.append('\x00');
    }
    QByteArray cmd = header_part + teach_plural_position_data_part;

    quint16 data_size = (numb*13 + 1)* 4;
    quint8 data_size_1 = (quint8)data_size;
    quint8 data_size_2 = (quint8)(data_size >> 8);

    cmd[CMD_REQUEST_ID] = request_id_index;
//    cmd[CMD_DATA_SIZE] = CMD_HEADER_TEACH_PLURAL_POSITION_DATA_SIZE;
    cmd[CMD_DATA_SIZE] = data_size_1;
    cmd[CMD_DATA_SIZE + 1] = data_size_2;
    cmd[CMD_ID_ADDRESS] = CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_1;
    cmd[CMD_ID_ADDRESS + 1] = CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_2;
    cmd[CMD_INSTANCE] = index;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_TEACH_PLURAL_POSITION_ATTRIBUTE_ALL;
    cmd[CMD_SERVICE] = CMD_HEADER_TEACH_PLURAL_POSITION_SERVICE_WRITE;
//    cmd[DATA_BYTE0] = 0x02;
    cmd[DATA_BYTE0] = (quint8)numb;
    for(int i =0; i < numb; i++){
        cmd[4 + DATA_TEACH_POSITION_DATA_TYPE + i*13*4] = coordinate_type;
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_NUM + i*13*4] = '\x01';

        quint32 axis1 = (quint32)(position->at(0 + 6*i)*1000);
        quint32 axis2 = (quint32)(position->at(1 + 6*i)*1000);
        quint32 axis3 = (quint32)(position->at(2 + 6*i)*1000);
        quint32 axis4 = (quint32)(position->at(3 + 6*i)*10000);
        quint32 axis5 = (quint32)(position->at(4 + 6*i)*10000);
        quint32 axis6 = (quint32)(position->at(5 + 6*i)*10000);

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_1 + i*13*4 + 3] = (quint8)(axis1 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_1 + i*13*4 + 2] = (quint8)(axis1 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_1 + i*13*4 + 1] = (quint8)(axis1 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_1 + i*13*4] = (quint8)axis1;

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_2 + i*13*4 + 3] = (quint8)(axis2 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_2 + i*13*4 + 2] = (quint8)(axis2 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_2 + i*13*4 + 1] = (quint8)(axis2 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_2 + i*13*4] = (quint8)axis2;

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_3 + i*13*4 + 3] = (quint8)(axis3 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_3 + i*13*4 + 2] = (quint8)(axis3 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_3 + i*13*4 + 1] = (quint8)(axis3 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_3 + i*13*4] = (quint8)axis3;

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_4 + i*13*4 + 3] = (quint8)(axis4 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_4 + i*13*4 + 2] = (quint8)(axis4 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_4 + i*13*4 + 1] = (quint8)(axis4 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_4 + i*13*4] = (quint8)axis4;

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_5 + i*13*4 + 3] = (quint8)(axis5 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_5 + i*13*4 + 2] = (quint8)(axis5 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_5 + i*13*4 + 1] = (quint8)(axis5 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_5 + i*13*4] = (quint8)axis5;

        cmd[4 + DATA_TEACH_POSITION_COORDINATE_6 + i*13*4 + 3] = (quint8)(axis6 >> 24);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_6 + i*13*4 + 2] = (quint8)(axis6 >> 16);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_6 + i*13*4 + 1] = (quint8)(axis6 >> 8);
        cmd[4 + DATA_TEACH_POSITION_COORDINATE_6 + i*13*4] = (quint8)axis6;
    }
    return cmd;
}

QByteArray motoman_command::readIO(quint8 request_id_index){
    QByteArray cmd = header_part;
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_DATA_SIZE] = CMD_HEADER_READ_IO_DATA_SIZE;
    cmd[CMD_ID_ADDRESS] = CMD_ID_READ_IO;
    cmd[CMD_INSTANCE] = CMD_HEADER_READ_IO_INSTANCE_OUTPUT_BYTE_1;
    cmd[CMD_INSTANCE + 1] = CMD_HEADER_READ_IO_INSTANCE_OUTPUT_BYTE_2;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_READ_IO_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_READ_IO_SERVICE_READ;
    return cmd;
}

QByteArray motoman_command::saveFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_name){
    QByteArray cmd = header_part;

    QByteArray name = file_name.toUtf8();
    if(name.size() > 497){
        QByteArray file("Error");
        return file;
    } else if(name == "No data"){
        cmd[CMD_DATA_SIZE] = 0;
    } else{
        for(int i = 0; i < file_name.size(); i++){
            cmd.append(name[i]);
        }
        cmd[CMD_DATA_SIZE] = file_name.size();
        cmd[CMD_DATA_SIZE + 1] = (quint8)(file_name.size() >> 8);
    }

    cmd[CMD_PROCESS_DIV] = '\x02';
    cmd[CMD_ACK] = ack;
    cmd[CMD_BLOCK_NUM] = (quint8)block_num;
    cmd[CMD_BLOCK_NUM + 1] = (quint8)(block_num >> 8);
    cmd[CMD_BLOCK_NUM + 2] = (quint8)(block_num >> 16);
    cmd[CMD_BLOCK_NUM + 3] = (quint8)(block_num >> 24);
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_FILE;
    cmd[CMD_INSTANCE] = CMD_HEADER_FILE_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_FILE_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_FILE_SAVING_SERVICE;

    return cmd;
}

QByteArray motoman_command::loadFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_name){
    QByteArray cmd = header_part;

    QByteArray name = file_name.toUtf8();
    if(name.size() > 497){
        QByteArray file("Error");
        return file;
    } else{
        for(int i = 0; i < file_name.size(); i++){
            cmd.append(name[i]);
        }
        cmd[CMD_DATA_SIZE] = file_name.size();
        cmd[CMD_DATA_SIZE + 1] = (quint8)(file_name.size() >> 8);
    }

    cmd[CMD_PROCESS_DIV] = '\x02';
    cmd[CMD_ACK] = ack;
    cmd[CMD_BLOCK_NUM] = (quint8)block_num;
    cmd[CMD_BLOCK_NUM + 1] = (quint8)(block_num >> 8);
    cmd[CMD_BLOCK_NUM + 2] = (quint8)(block_num >> 16);
    cmd[CMD_BLOCK_NUM + 3] = (quint8)(block_num >> 24);
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_FILE;
    cmd[CMD_INSTANCE] = CMD_HEADER_FILE_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_FILE_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_FILE_LOADING_SERVICE;

    return cmd;
}

QByteArray motoman_command::listFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_extension){
    QByteArray cmd = header_part;

    QByteArray name = file_extension.toUtf8();
    if(name.size() > 497){
        QByteArray file("Error");
        return file;
    } else if(name == "No data"){
        cmd[CMD_DATA_SIZE] = 0;
    } else{
        for(int i = 0; i < file_extension.size(); i++){
            cmd.append(name[i]);
        }
        cmd[CMD_DATA_SIZE] = file_extension.size();
        cmd[CMD_DATA_SIZE + 1] = (quint8)(file_extension.size() >> 8);
    }

    cmd[CMD_PROCESS_DIV] = '\x02';
    cmd[CMD_ACK] = ack;
    cmd[CMD_BLOCK_NUM] = (quint8)block_num;
    cmd[CMD_BLOCK_NUM + 1] = (quint8)(block_num >> 8);
    cmd[CMD_BLOCK_NUM + 2] = (quint8)(block_num >> 16);
    cmd[CMD_BLOCK_NUM + 3] = (quint8)(block_num >> 24);
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_FILE;
    cmd[CMD_INSTANCE] = CMD_HEADER_FILE_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_FILE_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_FILE_LIST_SERVICE;

    return cmd;
}

QByteArray motoman_command::deleteFile(quint8 request_id_index, QString file_name){
    QByteArray cmd = header_part;

    QByteArray name = file_name.toUtf8();
    if(name.size() > 497){
        QByteArray file("Error");
        return file;
    } else if(name == "No data"){
        cmd[CMD_DATA_SIZE] = 0;
    } else{
        for(int i = 0; i < file_name.size(); i++){
            cmd.append(name[i]);
        }
        cmd[CMD_DATA_SIZE] = file_name.size();
        cmd[CMD_DATA_SIZE + 1] = (quint8)(file_name.size() >> 8);
    }

    cmd[CMD_PROCESS_DIV] = '\x02';
    cmd[CMD_REQUEST_ID] = request_id_index;
    cmd[CMD_ID_ADDRESS] = CMD_ID_FILE;
    cmd[CMD_INSTANCE] = CMD_HEADER_FILE_INSTANCE;
    cmd[CMD_ATTRIBUTE] = CMD_HEADER_FILE_ATTRIBUTE;
    cmd[CMD_SERVICE] = CMD_HEADER_FILE_DELETE_SERVICE;

    return cmd;
}
