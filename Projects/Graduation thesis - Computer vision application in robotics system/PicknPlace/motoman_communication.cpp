#include "motoman_communication.h"

motoman_communication::motoman_communication(QObject *parent) : QObject(parent)
{
    request_id_index = 0;
    response_id_index = 0;
    running_status = 0;

    robot_position.append(0);
    robot_position.append(0);
    robot_position.append(0);
    robot_position.append(0);
    robot_position.append(0);
    robot_position.append(0);

    robot_pulse.append(0);
    robot_pulse.append(0);
    robot_pulse.append(0);
    robot_pulse.append(0);
    robot_pulse.append(0);
    robot_pulse.append(0);

    robot_degree.append(0);
    robot_degree.append(0);
    robot_degree.append(0);
    robot_degree.append(0);
    robot_degree.append(0);
    robot_degree.append(0);

    job_select_status_error = true;
    job_start_status_error = false;
    control_mode = 0;

    connect(&server, SIGNAL(dataRecieveSignal()), this, SLOT(motomanDataCallback()));
    connect(&file_control, SIGNAL(dataRecieveSignal()), this, SLOT(motomanFileCallback()));

}

motoman_communication::~motoman_communication(){
    server.udpDisConnect();
    file_control.udpDisConnect();
}

void motoman_communication::motomanDataCallback(){
//    qDebug() << "call";
    QByteArray data = server.getUdpData();
    response_id_index = data[CMD_REQUEST_ID];
    char response_id = request_command[response_id_index];
    if(response_id == CMD_ID_READ_ROBOT_POSITION){
        if(read_robot_position_type == READ_ROBOT_POSITION_CARTESIAN){
            motomanReadPositionResponse(data);
        }
        else if(read_robot_position_type == READ_ROBOT_POSITION_PULSE){
            motomanReadPulseResponse(data);
        }
    }
    else if(response_id == CMD_ID_READ_STATUS){
        if(read_stauts_type == READ_ROBOT_STATUS_MODE){
            motomanReadStatusResponse(data);
        } else if(read_stauts_type == READ_CONTROL_MODE){
            control_mode = motomanReadControlModeResponse(data);
        }
    }
    else if(response_id == CMD_ID_SELECT_JOB){
        response_signal = motomanSelectJobResponse(data);
    }
    else if(response_id == CMD_ID_SERVO_ON){
        if(turn_servo_mode == TURN_SERVO_ON){
            response_signal = motomanOnServoResponse(data);
            qDebug() << "servo turned on signal:" << response_signal;
        } else if(turn_servo_mode == TURN_SERVO_OFF){
            response_signal = motomanOffServoResponse(data);
        }
    }
    else if(response_id == CMD_ID_START_JOB){
        response_signal = motomanStartJobResponse(data);
    }
    else if(response_id == CMD_ID_READ_JOB_INFORMATION){
        motomanReadJobResponse(data);
    }
    else if(response_id == CMD_ID_TEACH_ROBOT_POSITION){
        response_signal = motomanTeachPositionResponse(data);
    }
    else if(response_id == CMD_ID_WRITE_BYTE){
        response_signal = motomanWriteByteResponse(data);
    }
    else if(response_id == CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_1){
        response_signal = motomanTeachPluralPositionResponse(data);
    }
    else if(response_id == CMD_ID_READ_IO){
        response_signal = motomanReadIOResponse(data);
    }
    else if(response_id == CMD_ID_HEADER_FILE_SAVING/* && motoman_controller == "DX"*/){
        motomanSaveFileResponse(data);
    }
    else if(response_id == CMD_ID_HEADER_FILE_LOADING/* && motoman_controller == "DX"*/){
        load_file_block_num++;
        motomanLoadFileResponse(data, load_file_block_num);
//        qDebug() << "received";
    }
    emit dataRecieveUISignal(control_mode, response_signal, list);
}

void motoman_communication::motomanFileCallback(){
    QByteArray file_data = file_control.getUdpData();
    file_response_id_index = file_data[CMD_REQUEST_ID];
    char file_response_id = request_command[file_response_id_index];

    if(file_response_id == CMD_ID_HEADER_FILE_SAVING/* && motoman_controller == "YRC"*/){
        motomanSaveFileResponse(file_data);
//        qDebug() << "received";
    } else if(file_response_id == CMD_ID_HEADER_FILE_LOADING/* && motoman_controller == "YRC"*/){
        load_file_block_num++;
        motomanLoadFileResponse(file_data, load_file_block_num);
//        qDebug() << "received";
    } else if(file_response_id == CMD_ID_HEADER_FILE_LIST){
//        qDebug() << 34;
        motomanListFileResponse(file_data);
        emit dataRecieveUISignal(control_mode, response_signal, list);
//        qDebug() << list;
//        qDebug() << "emited";
    } else if(file_response_id == CMD_ID_HEADER_FILE_DELETE){
        response_signal = motomanDeleteFileResponse(file_data);
        emit dataRecieveUISignal(control_mode, response_signal, list);
    }
}

void motoman_communication::motomanSetConnection(QHostAddress address, quint16 udp_port, quint16 file_port){
    udp_address = address;
    server_port = udp_port;
    file_control_port = file_port;
}

bool motoman_communication::motomanConnect(){
    bool status = server.udpConnect(udp_address, server_port);
    return status;
}

void motoman_communication::motomanDisconnect(){
    server.udpDisConnect();
}

bool motoman_communication::motomanFIleCOntrolConnect(){
    bool status = file_control.udpConnect(udp_address, file_control_port);
    return status;
}

void motoman_communication::motomanFileControlDisconnect(){
    file_control.udpDisConnect();
}

void motoman_communication::motomanOnServo(){
    QByteArray data = motoman_command.setServoOn(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_SERVO_ON;
    turn_servo_mode = TURN_SERVO_ON;
    request_id_index++;
}

bool motoman_communication::motomanOnServoResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00') {
        return false;
    } else return true;
}

void motoman_communication::motomanOffServo(){
    QByteArray data = motoman_command.setServoOff(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_SERVO_ON;
    turn_servo_mode = TURN_SERVO_OFF;
    request_id_index++;
}

bool motoman_communication::motomanOffServoResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00') {
        return false;
    } else return true;
}

void motoman_communication::motomanReadControlMode(){
    QByteArray data = motoman_command.readStatus(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_STATUS;
    read_stauts_type = READ_CONTROL_MODE;
    request_id_index++;
}

quint8 motoman_communication::motomanReadControlModeResponse(QByteArray data){
    quint8 control_mode = 0;
//    qDebug() << data[DATA_BYTE0];
    control_mode = control_mode | (quint8)data[DATA_BYTE0];
//    qDebug() << control_mode;

    /*
     * In data 1, we take bit5, bit6, and bit7 to know "control mode" of the controller
     */
    control_mode &= RESPONSE_VALUE_READ_CONTROL_MODE_MASK;
//    qDebug() << control_mode;
    return control_mode;
}

void motoman_communication::motomanReadPosition(){
    QByteArray data = motoman_command.readRobotPosition(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_ROBOT_POSITION;
    read_robot_position_type = READ_ROBOT_POSITION_CARTESIAN;
    request_id_index++;
}

void motoman_communication::motomanReadPulse(){
    QByteArray data = motoman_command.readRobotPulse(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_ROBOT_POSITION;
    read_robot_position_type = READ_ROBOT_POSITION_PULSE;
    request_id_index++;
}

void motoman_communication::motomanReadStatus(){
    QByteArray data = motoman_command.readStatus(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_STATUS;
    read_stauts_type = READ_ROBOT_STATUS_MODE;
    request_id_index++;
}

void motoman_communication::motomanReadPositionResponse(QByteArray data){
    /*
     * Since the coordinate of robot (x,y,z,Rx,Ry,Rz) could be both negative and positive so variables should be qint32
     */
    qint32 x_axis = 0;
    qint32 y_axis = 0;
    qint32 z_axis = 0;
    qint32 roll_angle = 0;
    qint32 pitch_angle = 0;
    qint32 yaw_angle = 0;

    /*
     * Each axis contains data in 4 bytes so we have to combine them into 1 byte (1 place to show)
     */
    x_axis = ( (((quint32)(quint8)data[DATA_X + 3]) << 24) | (((quint32)(quint8)data[DATA_X + 2]) << 16) | (((quint32)(quint8)data[DATA_X + 1]) << 8) | (quint32)(quint8)data[DATA_X] );
    y_axis = ( (((quint32)(quint8)data[DATA_Y + 3]) << 24) | (((quint32)(quint8)data[DATA_Y + 2]) << 16) | (((quint32)(quint8)data[DATA_Y + 1]) << 8) | (quint32)(quint8)data[DATA_Y] );
    z_axis = ( (((quint32)(quint8)data[DATA_Z + 3]) << 24) | (((quint32)(quint8)data[DATA_Z + 2]) << 16) | (((quint32)(quint8)data[DATA_Z + 1]) << 8) | (quint32)(quint8)data[DATA_Z] );
    roll_angle = ( (((quint32)(quint8)data[DATA_ROLL + 3]) << 24) | (((quint32)(quint8)data[DATA_ROLL + 2]) << 16) | (((quint32)(quint8)data[DATA_ROLL + 1]) << 8) | (quint32)(quint8)data[DATA_ROLL] );
    pitch_angle = ( (((quint32)(quint8)data[DATA_PITCH + 3]) << 24) | (((quint32)(quint8)data[DATA_PITCH + 2]) << 16) | (((quint32)(quint8)data[DATA_PITCH + 1]) << 8) | (quint32)(quint8)data[DATA_PITCH] );
    yaw_angle = ( (((quint32)(quint8)data[DATA_YAW + 3]) << 24) | (((quint32)(quint8)data[DATA_YAW + 2]) << 16) | (((quint32)(quint8)data[DATA_YAW + 1]) << 8) | (quint32)(quint8)data[DATA_YAW] );

    /*
     * We have to divide these factors to get the final values x,y,z(mm) and Rx,Ry,Rz(degree)
     */
    robot_position[0] = (x_axis/1000.0);
    robot_position[1] = (y_axis/1000.0);
    robot_position[2] = (z_axis/1000.0);
    robot_position[3] = (roll_angle/10000.0);
    robot_position[4] = (pitch_angle/10000.0);
    robot_position[5] = (yaw_angle/10000.0);
}

void motoman_communication::motomanReadPulseResponse(QByteArray data){
    /*
     * Since the pulse of each joint in the robot could be both negative and positive so variables should be qint32
     */
    qint32 j1 = 0;
    qint32 j2 = 0;
    qint32 j3 = 0;
    qint32 j4 = 0;
    qint32 j5 = 0;
    qint32 j6 = 0;

    /*
     * Each axis contains data in 4 bytes so we have to combine them into 1 byte (1 place to show)
     */
    j1 = ( (((quint32)(quint8)data[DATA_X + 3]) << 24) | (((quint32)(quint8)data[DATA_X + 2]) << 16) | (((quint32)(quint8)data[DATA_X + 1]) << 8) | (quint32)(quint8)data[DATA_X] );
    j2 = ( (((quint32)(quint8)data[DATA_Y + 3]) << 24) | (((quint32)(quint8)data[DATA_Y + 2]) << 16) | (((quint32)(quint8)data[DATA_Y + 1]) << 8) | (quint32)(quint8)data[DATA_Y] );
    j3 = ( (((quint32)(quint8)data[DATA_Z + 3]) << 24) | (((quint32)(quint8)data[DATA_Z + 2]) << 16) | (((quint32)(quint8)data[DATA_Z + 1]) << 8) | (quint32)(quint8)data[DATA_Z] );
    j4 = ( (((quint32)(quint8)data[DATA_ROLL + 3]) << 24) | (((quint32)(quint8)data[DATA_ROLL + 2]) << 16) | (((quint32)(quint8)data[DATA_ROLL + 1]) << 8) | (quint32)(quint8)data[DATA_ROLL] );
    j5 = ( (((quint32)(quint8)data[DATA_PITCH + 3]) << 24) | (((quint32)(quint8)data[DATA_PITCH + 2]) << 16) | (((quint32)(quint8)data[DATA_PITCH + 1]) << 8) | (quint32)(quint8)data[DATA_PITCH] );
    j6 = ( (((quint32)(quint8)data[DATA_YAW + 3]) << 24) | (((quint32)(quint8)data[DATA_YAW + 2]) << 16) | (((quint32)(quint8)data[DATA_YAW + 1]) << 8) | (quint32)(quint8)data[DATA_YAW] );


    robot_pulse[0] = j1;
    robot_pulse[1] = j2;
    robot_pulse[2] = j3;
    robot_pulse[3] = j4;
    robot_pulse[4] = j5;
    robot_pulse[5] = j6;
}

void motoman_communication::motomanReadStatusResponse(QByteArray data){
    running_status = 0;

//    running_status = running_status | ( ((quint16)(quint8)data[DATA_BYTE0]) << 8 );
//    running_status = running_status | (quint16)(quint8)data[DATA_BYTE0];
    running_status = running_status | (quint8)data[DATA_BYTE0];

    /*
     * In data 1, we only want take bit3 to know "Running status" of the robot
     */
    running_status &= RESPONSE_VALUE_READ_ROBOT_STATUS_MASK;
}

QVector<double> motoman_communication::motomanUpdatePosition(){
    return robot_position;
//    qDebug() << robot_position;
}

QVector<double> motoman_communication::motomanUpdatePulse(){
    return robot_pulse;
}

QVector<double> motoman_communication::motomanUpdateDegree(){
    robot_degree[0] = robot_pulse[0]*30.0/34816.0;
    robot_degree[1] = robot_pulse[1]*90.0/102400.0;
    robot_degree[2] = robot_pulse[2]*90.0/51200.0;
    robot_degree[3] = robot_pulse[3]*30.0/10204.0;
    robot_degree[4] = robot_pulse[4]*30.0/10204.0;
    robot_degree[5] = robot_pulse[5]*30.0/10204.0;
    return robot_degree;
}

quint8 motoman_communication::motomanUpdateStatus(){
    return running_status;
}

void motoman_communication::motomanMoveCartesian(quint8 coordinate_type, quint8 move_type, quint8 speed_type, double speed, QVector<double> *position){
    QByteArray data = motoman_command.setRobotCartesianPosition(request_id_index, coordinate_type, move_type, speed_type, speed, position);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_MOVE_CARTESIAN;
    request_id_index++;
}

void motoman_communication::motomanSelectJob(QString job_name, quint32 line){
    QByteArray data = motoman_command.selectJob(request_id_index, job_name, line);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_SELECT_JOB;
    request_id_index++;
    job = job_name;
}

bool motoman_communication::motomanSelectJobResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to select job";
        job_select_status_error = true;
        return false;
    } else {
        job_select_status_error = false;
        emit startJob();
//        qDebug() << "Job's selected";
        return true;
//        emit selectJobSignal(job);
    }
}

void motoman_communication::motomanStartJob(){
    if(!job_select_status_error) {
        QByteArray data = motoman_command.startJob(request_id_index);
        server.sendData(udp_address, server_port, data);
        request_command[request_id_index] = CMD_ID_START_JOB;
        request_id_index++;
    } else {
//        qDebug() << "Failed to start job";
        job_start_status_error = true;
        emit dataRecieveUISignal(control_mode, false, list);
        return;
    }

}

bool motoman_communication::motomanStartJobResponse(QByteArray data){
//    qDebug() << data;
    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to start job";
        job_start_status_error = true;
        return false;
    } else {
        qDebug() << "Job's running";
        return true;
    }
}

void motoman_communication::motomanReadJob(quint8 job_index, QString information){
    job_information = information;
    QByteArray data = motoman_command.readJob(request_id_index, job_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_JOB_INFORMATION;
    request_id_index++;
}

void motoman_communication::motomanReadJobResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00'){
        qDebug() << "Failed to read job";
        return;
    }

    if(job_information == "name"){
        QByteArray name_byte;
        for(int i = DATA_BYTE0; i < DATA_BYTE0 + 32; i++){
            name_byte.append(data[i]);
        }
//        qDebug() << name_byte;
        name_info = QString(name_byte);
        qDebug() << name_info;
    } else if(job_information == "line"){
        quint32 line = 0;
        line = line | (quint32)(quint8)data[DATA_BYTE0 + 32];
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 1]) << 8;
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 2]) << 16;
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 3]) << 24;
        info = line;
        qDebug() << info;
    } else if(job_information == "step"){
        quint32 step = 0;
        step = step | (quint32)(quint8)data[DATA_BYTE0 + 32 + 4];
        step = step | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 1]) << 8;
        step = step | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 2]) << 16;
        step = step | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 3]) << 24;
        info = step;
        qDebug() << info;
    } else if(job_information == "speed"){
        quint32 speed = 0;
        speed = speed | (quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 4];
        speed = speed | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 4 + 1]) << 8;
        speed = speed | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 4 + 2]) << 16;
        speed = speed | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 4 + 4 + 3]) << 24;
        info = speed;
        qDebug() << info;
    } else if(job_information == "name, line"){
        QByteArray name_byte;
        for(int i = DATA_BYTE0; i < DATA_BYTE0 + 32; i++){
            name_byte.append(data[i]);
        }
//        qDebug() << name_byte;
        name_info = QString(name_byte);
        qDebug() << name_info;

        quint32 line = 0;
        line = line | (quint32)(quint8)data[DATA_BYTE0 + 32];
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 1]) << 8;
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 2]) << 16;
        line = line | ((quint32)(quint8)data[DATA_BYTE0 + 32 + 3]) << 24;
        info = line;
        qDebug() << info;
    }
}

void motoman_communication::motomanTeachPosition(quint8 coordinate_type, int index, QVector<double> *position){
    QByteArray data = motoman_command.teachRobotPosition(request_id_index, coordinate_type, index, position);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_TEACH_ROBOT_POSITION;
    request_id_index++;
}

bool motoman_communication::motomanTeachPositionResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00'){
        qDebug() << "Failed to teach position";
        return false;
    } else /*qDebug() << "Position's teached";*/
        return true;
}

void motoman_communication::motomanWriteByte(int index, quint8 data_byte){
    QByteArray data = motoman_command.writeByte(request_id_index, index, data_byte);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_WRITE_BYTE;
    request_id_index++;
    byte = index;
}

bool motoman_communication::motomanWriteByteResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to write byte";
        return false;
    } else /*qDebug() << "Byte's written";*/
        return true;
    emit writeByteSignal(byte);
}

void motoman_communication::motomanTeachPluralPosition(quint8 coordinate_type,int numb, int index, QVector<double> *position){
    QByteArray data = motoman_command.teachPluralRobotPosition(request_id_index, coordinate_type, numb,index, position);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_1;
    request_id_index++;
}

bool motoman_communication::motomanTeachPluralPositionResponse(QByteArray data){
    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to teach positions";
        return false;
    } else /*qDebug() << "Positions are teached";*/
        return true;
}

void motoman_communication::motomanReadIO(){
    QByteArray data = motoman_command.readIO(request_id_index);
    server.sendData(udp_address, server_port, data);
    request_command[request_id_index] = CMD_ID_READ_IO;
    request_id_index++;
}

bool motoman_communication::motomanReadIOResponse(QByteArray data){
    io_signal = 0;

    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to read IO signal";
        return false;
    } else {
        io_signal = io_signal | (quint8)data[DATA_BYTE0];
        return true;
    }
}

quint8 motoman_communication::motomanUpdateIO(){
    return io_signal;
}

void motoman_communication::motomanSaveFile(QString job_name){
    qDebug() << 2;
    name = job_name;
    QByteArray data = motoman_command.saveFile(request_id_index, CMD_ACK_REQUEST, 0, job_name);
//    qDebug() << data;

//    if(motoman_controller == "YRC"){
//        file_control.sendData(udp_address, file_control_port, data);
//        qDebug() << 11;
//    } else if(motoman_controller == "DX"){
//        server.sendData(udp_address, server_port, data);
//        qDebug() << 12;
//    }
    file_control.sendData(udp_address, file_control_port, data);
    request_command[request_id_index] = CMD_ID_HEADER_FILE_SAVING;
    request_id_index++;
//    qDebug() << motoman_controller;
}

void motoman_communication::motomanSaveFileResponse(QByteArray data){
    qDebug() << 3;
//    qDebug() << data[CMD_STATUS];
    if(data[CMD_STATUS] != '\x00'){
        qDebug() << "Failed to save file";
        return;
    }
//    name.append(".JBI");
    QByteArray data_receive;
    for(int i = DATA_BYTE0; i < data.size(); i++){
        data_receive.append(data[i]);
    }
//    qDebug() << data_receive;
    QFile file(name);
//    file.setFileName("/program/" + name);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)){
        QTextStream out(&file);
        out << data_receive << '\n';
        file.flush();
        file.close();
        qDebug() << "Saved to file " << name;
    }

    // Check if it is that last data, since last block will be 0x80000000 + N, where N is the number of the previous block
    if(data[15] == '\x80'){
        return;
    }

    quint32 block_num = 0;
    block_num = block_num | (quint32)(quint8)data[CMD_BLOCK_NUM];
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 1]) << 8;
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 2]) << 16;
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 3]) << 24;
    QByteArray data_reply = motoman_command.saveFile(request_id_index, CMD_ACK_REPLY, block_num, "No data");
//    qDebug() << block_num;
//    qDebug() << data_reply;

//    delay(80);
//    if(motoman_controller == "YRC"){
//        file_control.sendData(udp_address, file_control_port, data_reply);
//        qDebug() << 11;
//    } else if(motoman_controller == "DX"){
//        server.sendData(udp_address, server_port, data_reply);
//    }
    file_control.sendData(udp_address, file_control_port, data_reply);
}

void motoman_communication::motomanLoadFile(QString data_load){
    load_file_block_num = 0;
    QByteArray data = motoman_command.loadFile(request_id_index, CMD_ACK_REQUEST, load_file_block_num, data_load);

//    if(motoman_controller == "YRC"){
//        file_control.sendData(udp_address, file_control_port, data);
//        qDebug() << 11;
//    } else if(motoman_controller == "DX"){
//        server.sendData(udp_address, server_port, data);
//        qDebug() << 12;
//    }
    file_control.sendData(udp_address, file_control_port, data);
    request_command[request_id_index] = CMD_ID_HEADER_FILE_LOADING;
    request_id_index++;

    QFile file(data_load);
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QTextStream out(&file);
        str_file = out.readAll();
        file.flush();
        file.close();
        load_file_status = true;
    }
}

void motoman_communication::motomanLoadFileResponse(QByteArray data, quint32 block_num){
    if(data[CMD_STATUS] != '\x00'){
        qDebug() << "Failed to load file";
        return;
    }

//    QString str_data = QString(data);
    if(load_file_status){
        if(str_file.size() > 400){
            QString temp = str_file.left(400);
            str_file = str_file.right(str_file.size() - 400);
            QByteArray data_reply = motoman_command.loadFile(request_id_index, CMD_ACK_REPLY, load_file_block_num, temp);

//            if(motoman_controller == "YRC"){
//                file_control.sendData(udp_address, file_control_port, data_reply);
//                qDebug() << 11;
//            } else if(motoman_controller == "DX"){
//                server.sendData(udp_address, server_port, data_reply);
//                qDebug() << 12;
//            }
            file_control.sendData(udp_address, file_control_port, data_reply);
        } else {
            // 0x80000000 = 2147483648 (decimal)
            QByteArray data_reply = motoman_command.loadFile(request_id_index, CMD_ACK_REPLY, load_file_block_num + 2147483648, str_file);

//            if(motoman_controller == "YRC"){
//                file_control.sendData(udp_address, file_control_port, data_reply);
//                qDebug() << 11;
//            } else if(motoman_controller == "DX"){
//                server.sendData(udp_address, server_port, data_reply);
//                qDebug() << 12;
//            }
            file_control.sendData(udp_address, file_control_port, data_reply);

            load_file_status = false;
        }
    }
}

void motoman_communication::motomanListFile(QString file_extension){
//    qDebug() << 33;
    qDebug() << list;

    QByteArray data = motoman_command.listFile(request_id_index, CMD_ACK_REQUEST, 0, file_extension);
    file_control.sendData(udp_address, file_control_port, data);
    request_command[request_id_index] = CMD_ID_HEADER_FILE_LIST;
    request_id_index++;
}

QStringList motoman_communication::motomanListFileResponse(QByteArray data){
    qDebug() << "list jobbbbb:" << data;
//    qDebug() << 35;
    if(data[CMD_STATUS] != '\x00'){
        qDebug() << "Failed to save file";
        return list;
    }

    QByteArray data_receive;
    for(int i = DATA_BYTE0; i < data.size(); i++){
        data_receive.append(data[i]);
    }
//    qDebug() << data_receive;
//    qDebug() << data_receive.size();
    QDir dir("C:\\Autoss\\Draft\\Job_info");
    if(!dir.exists()){
        dir.mkpath("C:\\Autoss\\Draft\\Job_info");
        SetFileAttributesW(L"C:\\Autoss", FILE_ATTRIBUTE_HIDDEN);
        SetFileAttributesW(L"C:\\Autoss\\Draft\\Job_info", FILE_ATTRIBUTE_HIDDEN);
    }
    QFile f("C:\\Autoss\\Draft\\Job_info\\list.txt");
    QFile file("C:\\Autoss\\Draft\\Job_info\\list.txt");
//    file.setFileName("/program/" + name);

    // Overwrite the file
    if(file.open(QIODevice::WriteOnly /*| QIODevice::Append | QIODevice::Text*/)){
        QTextStream out(&file);
        out << data_receive << '\n';
        file.flush();
        file.close();
        qDebug() << "Saved to file";
    }

    // Check if it is that last data, since last block will be 0x80000000 + N, where N is the number of the previous block
    if(data[15] == '\x80'){
        QFile file("C:\\Autoss\\Draft\\Job_info\\list.txt");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream str(&file);
            while(!str.atEnd()){
                list.append(str.readLine().split("\n").at(0));
            }
            list.removeLast();
//            qDebug() << list;
            list.sort();
        }
        file.flush();
        file.close();

        qDebug() << "Finished";
        return list;
    }

    quint32 block_num = 0;
    block_num = block_num | (quint32)(quint8)data[CMD_BLOCK_NUM];
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 1]) << 8;
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 2]) << 16;
    block_num = block_num | ((quint32)(quint8)data[CMD_BLOCK_NUM + 3]) << 24;
    QByteArray data_reply = motoman_command.saveFile(request_id_index, CMD_ACK_REPLY, block_num, "No data");
    qDebug() << data_reply;
    qDebug() << file_control.socket->hasPendingDatagrams();
//    while(file_control.socket->hasPendingDatagrams()){};
    file_control.sendData(udp_address, file_control_port, data_reply);
    qDebug() << "again";
    return list;
}

void motoman_communication::motomanDeleteFile(QString filename){
//    qDebug() << filename;
    QByteArray data = motoman_command.deleteFile(request_id_index, filename);
    file_control.sendData(udp_address, file_control_port, data);
    request_command[request_id_index] = CMD_ID_HEADER_FILE_DELETE;
    request_id_index++;
}

bool motoman_communication::motomanDeleteFileResponse(QByteArray data){
//    qDebug() << data;
    if(data[CMD_STATUS] != '\x00'){
//        qDebug() << "Failed to read IO signal";
        return false;
    } else {
        return true;
    }
}

QMatrix4x4 motoman_communication::motomanTransformationMatrix(double x, double y, double z, double Rx1, double Ry1, double Rz1){
    double Rx, Ry, Rz;
    Rx = Rx1*M_PI/180.0;
    Ry = Ry1*M_PI/180.0;
    Rz = Rz1*M_PI/180.0;
    QMatrix4x4 T = QMatrix4x4(
                                cos(Rz)*cos(Ry),    cos(Rz)*sin(Ry)*sin(Rx)-sin(Rz)*cos(Rx),    cos(Rz)*sin(Ry)*cos(Rx)+sin(Rz)*sin(Rx),    x,
                                sin(Rz)*cos(Ry),    sin(Rz)*sin(Ry)*sin(Rx)+cos(Rz)*cos(Rx),    sin(Rz)*sin(Ry)*cos(Rx)-cos(Rz)*sin(Rx),    y,
                                -sin(Ry),           cos(Ry)*sin(Rx),                            cos(Ry)*cos(Rx),                            z,
                                0,                  0,                                          0,                                          1);
    return T;
}

Cartesian motoman_communication::motomanForwardKinematics(Joint input, DH_table param){
    QVector<double> a(6, 0);
    QVector<double> d(6, 0);
    QVector<double> alpha(6, 0);
    QVector<double> theta(6, 0);
    Cartesian output;

    a[0] = param.a1;
    a[1] = param.a2;
    a[2] = param.a3;
    d[3] = param.d4;
    d[5] = param.d6 - param.tool_length;

    for(int i = 0; i < 6; i++){
        alpha[i] = param.alpha[i]*M_PI/180.0;
        theta[i] = (input.theta[i] + param.theta[i])*M_PI/180.0;
    }

    QVector<QMatrix4x4> A(6);
    for(int i = 0; i < 6; i++){
        A[i] = QMatrix4x4(cos(theta[i]), -sin(theta[i])*cos(alpha[i]), sin(theta[i])*sin(alpha[i]) , a[i]*cos(theta[i]),
                          sin(theta[i]), cos(theta[i])*cos(alpha[i]) , -cos(theta[i])*sin(alpha[i]), a[i]*sin(theta[i]),
                          0            , sin(alpha[i])               , cos(alpha[i])               , d[i]              ,
                          0            , 0                           , 0                           , 1);
    }

    QMatrix4x4 T = A[0]*A[1]*A[2]*A[3]*A[4]*A[5];
    output.x = T(0, 3);
    output.y = T(1, 3);
    output.z = T(2, 3);

    double temp = pow(T(2,1), 2.0) + pow(T(2,2), 2.0);
    double Rx, Ry, Rz;
    if(temp != 0){
        Rx = atan2(T(2,1),T(2,2))*180.0/3.14;
        Ry = atan2(-T(2,0),sqrt(temp))*180.0/3.14;
        Rz = atan2(T(1,0),T(0,0))*180.0/3.14;
    } else if(temp == 0){
        if(T(2,0) == (double)-1){
            Rx = atan2(T(0,1),T(1,1))*180.0/M_PI;
            Ry = 90.0;
            Rz = 0;
        } else if(T(2,0) == (double)1){
            Rx = atan2(T(0,1),T(1,1))*180.0/M_PI;
            Ry = -90.0;
            Rz = 0;
        }
    }

    output.rx = Rx;
    output.ry = Ry;
    output.rz = Rz;
    return output;
}

Joint motoman_communication::motomanInverseKinematics(Cartesian input, DH_table param, Joint prev_joint){
    Joint output;

    // Transformation matrix
    QMatrix4x4 T06 = motomanTransformationMatrix(input.x, input.y, input.z, input.rx, input.ry, input.rz);

    QVector<double> a(6, 0);
    QVector<double> d(6, 0);
    QVector<double> alpha(6, 0);
    QVector<double> theta(6, 0);

    a[0] = param.a1;
    a[1] = param.a2;
    a[2] = param.a3;
    d[3] = param.d4;
    d[5] = param.d6 - param.tool_length;

    double PWx = input.x - abs(d[5])*T06(0,2);
    double PWy = input.y - abs(d[5])*T06(1,2);
    double PWz = input.z - abs(d[5])*T06(2,2);

    double a34 = sqrt(pow(param.a3,2) + pow(param.d4,2));
    double r = sqrt(pow(PWx,2) + pow(PWy,2));
    double AB = r - param.a1;
    double len = sqrt(pow(PWz,2) + pow(AB,2));

    // Theta1
    output.theta[0] = atan2(PWy, PWx);

    // Theta2
    double beta = 0.0;
    if(PWz >= 0)
        beta = acos(AB/len);
    else if(PWz < 0)
        beta = -acos(AB/len);

    double alpha_ = acos((pow(param.a2,2) + pow(len,2) - pow(a34,2)) / (2*param.a2*len));
    output.theta[1] = M_PI_2 - alpha_ - beta;

    // Theta3
    double gamma = acos((pow(param.a2,2) - pow(len,2) + pow(a34,2)) / (2*param.a2*a34));
    output.theta[2] = gamma - M_PI_2;

    for(int i = 0; i < 3; i++){
        alpha[i] = param.alpha[i]*M_PI/180.0;
        theta[i] = output.theta[i] + param.theta[i]*M_PI/180.0;
    }

    QVector<QMatrix4x4> A(3);
    for(int i = 0; i < 3; i++){
        A[i] = QMatrix4x4(cos(theta[i]), -sin(theta[i])*cos(alpha[i]), sin(theta[i])*sin(alpha[i]) , a[i]*cos(theta[i]),
                          sin(theta[i]), cos(theta[i])*cos(alpha[i]) , -cos(theta[i])*sin(alpha[i]), a[i]*sin(theta[i]),
                          0            , sin(alpha[i])               , cos(alpha[i])               , d[i]              ,
                          0            , 0                           , 0                           , 1);
    }
    QMatrix4x4 T23 = A[2];
    QMatrix4x4 T12 = A[1];
    QMatrix4x4 T01 = A[0];
    QMatrix4x4 T36 = T23.inverted()*T12.inverted()*T01.inverted()*T06;

    // Theta5
    output.theta[4] = asin(-T36(2,2));
    if(abs(prev_joint.theta[4]*M_PI/180.0 - output.theta[4]) > M_PI_2)
        output.theta[4] = M_PI - output.theta[4];

    // Theta4
    if(cos(output.theta[4]) > 0)
        output.theta[3] = atan2(-T36(1,2),-T36(0,2));
    else if(cos(output.theta[4]) < 0)
        output.theta[3] = atan2(T36(1,2),T36(0,2));

    // Theta6
    if(cos(output.theta[4]) > 0)
        output.theta[5] = atan2(-T36(2,1),-T36(2,0)) + param.rz*M_PI/180.0;
    else if(cos(output.theta[4]) < 0)
        output.theta[5] = atan2(T36(2,1),T36(2,0)) + param.rz*M_PI/180.0;

    if(abs(prev_joint.theta[5]*M_PI/180.0 - output.theta[5]) > (M_PI_2 + M_PI_4))
       output.theta[5] = output.theta[5] + M_PI*2.0;

    for(int i = 0; i < 6; i++)
        output.theta[i] = output.theta[i]*180.0/M_PI;


    return output;
}

bool motoman_communication::checkPose(Joint pose){
    if(pose.theta[0] > -169.0 && pose.theta[0] < 169.0)
        if(pose.theta[1] > -84.0 && pose.theta[1] < 89.0)
            if(pose.theta[2] > -49.0 && pose.theta[2] < 89.0)
                if(pose.theta[3] > -139.0 && pose.theta[3] < 139.0)
                    if(pose.theta[4] > -29.0 && pose.theta[4] < 210.0)
                        if(pose.theta[5] > -359.0 && pose.theta[5] < 359.0)
                            return true;
    return false;
}
