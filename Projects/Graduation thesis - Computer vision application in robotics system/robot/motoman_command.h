#ifndef MOTOMAN_COMMAND_H
#define MOTOMAN_COMMAND_H

#include <QObject>
#include <udp.h>

#define HEADER_SIZE     32

#define CMD_DATA_SIZE   6   // Data part size
#define CMD_PROCESS_DIV 9   // Processing division
#define CMD_ACK         10  // Acknowledge
#define CMD_REQUEST_ID  11  // Request ID
#define CMD_BLOCK_NUM   12  // Block No.
#define CMD_ID_ADDRESS  24  // Command No.
#define CMD_STATUS      25
#define CMD_INSTANCE    26  // Instance
#define CMD_ATTRIBUTE   28  // Attribute
#define CMD_SERVICE     29  // Service

#define DATA_BYTE0      32
#define DATA_BYTE1      33
#define DATA_BYTE2      34
#define DATA_BYTE3      35
#define DATA_BYTE4      36
#define DATA_BYTE5      37
#define DATA_BYTE6      38
#define DATA_BYTE7      39

#define DATA_X          52  // In table of data part, before "first axis data" there are also 5 other data fields (Data type->Extended type) + header size
#define DATA_Y          56  // data for x axis start from (32 + 4*5) = 52 and move forward 4 bytes for every other axes and angles's data
#define DATA_Z          60
#define DATA_ROLL       64
#define DATA_PITCH      68
#define DATA_YAW        72

#define DATA_MOVE_ROBOT_NUM          32
#define DATA_MOVE_STATION_NUM        36
#define DATA_MOVE_SPEED_TYPE         40
#define DATA_MOVE_SPEED              44
#define DATA_MOVE_CORDINATE_TYPE     48
#define DATA_MOVE_X_AXIS             52
#define DATA_MOVE_Y_AXIS             56
#define DATA_MOVE_Z_AXIS             60
#define DATA_MOVE_RX                 64
#define DATA_MOVE_RY                 68
#define DATA_MOVE_RZ                 72
#define DATA_MOVE_COORDINATE_NUM     96

#define DATA_SELECT_JOB_NAME         32
#define DATA_SELECT_JOB_LINE_NUM     64

#define DATA_TEACH_POSITION_DATA_TYPE                   32
#define DATA_TEACH_POSITION_COORDINATE_NUM              44
#define DATA_TEACH_POSITION_COORDINATE_1                52
#define DATA_TEACH_POSITION_COORDINATE_2                56
#define DATA_TEACH_POSITION_COORDINATE_3                60
#define DATA_TEACH_POSITION_COORDINATE_4                64
#define DATA_TEACH_POSITION_COORDINATE_5                68
#define DATA_TEACH_POSITION_COORDINATE_6                72

#define CMD_ID_SERVO_ON                                 '\x83'
#define CMD_ID_READ_ROBOT_POSITION                      '\x75'
#define CMD_ID_READ_STATUS                              '\x72'
#define CMD_ID_MOVE_CARTESIAN                           '\x8A'
#define CMD_ID_SELECT_JOB                               '\x87'
#define CMD_ID_START_JOB                                '\x86'
#define CMD_ID_READ_JOB_INFORMATION                     '\x73'
#define CMD_ID_TEACH_ROBOT_POSITION                     '\x7F'
#define CMD_ID_WRITE_BYTE                               '\x7A'
#define CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_1       '\x07'
#define CMD_ID_TEACH_PLURAL_ROBOT_POSITION_BYTE_2       '\x03'
#define CMD_ID_READ_IO                                  '\x78'
#define CMD_ID_FILE                                     '\x00'
#define CMD_ID_HEADER_FILE_SAVING                       '\x16'
#define CMD_ID_HEADER_FILE_LOADING                      '\x15'
#define CMD_ID_HEADER_FILE_LIST                         '\x32'
#define CMD_ID_HEADER_FILE_DELETE                       '\x09'

#define CMD_HEADER_SERVO_INSTANCE_HOLD                  '\x01'
#define CMD_HEADER_SERVO_INSTANCE_ON                    '\x02'
#define CMD_HEADER_SERVO_INSTANCE_HLOCK                 '\x03'
#define CMD_HEADER_SERVO_ATTRIBUTE                      '\x01'
#define CMD_HEADER_SERVO_SERVICE                        '\x10'

#define CMD_DATA_SERVO_ON                               '\x01'
#define CMD_DATA_SERVO_OFF                              '\x02'

#define CMD_HEADER_READ_POSITION_DATA_SIZE              '\x00'
#define CMD_HEADER_READ_POSITION_INSTANCE_CARTESIAN     '\x65'      // In Ethernet document : 101 - decimal
#define CMD_HEADER_READ_POSITION_INSTANCE_PULSE         '\x01'      // In Ethernet document : 1 - decimal
#define CMD_HEADER_READ_POSITION_ATTRIBUTE_ALL          '\x00'      // In Ethernet document : 0 - decimal for nonexistent axis
#define CMD_HEADER_READ_POSITION_SERVICE_ALL            '\x01'

#define READ_ROBOT_POSITION_CARTESIAN   0
#define READ_ROBOT_POSITION_PULSE       1

#define READ_ROBOT_STATUS_MODE          0
#define READ_CONTROL_MODE               1

#define TURN_SERVO_OFF                  0
#define TURN_SERVO_ON                   1

#define CMD_HEADER_READ_STATUS_DATA_SIZE                '\x00'
#define CMD_HEADER_READ_STATUS_INSTANCE                 '\x01'
#define CMD_HEADER_READ_STATUS_ATTRIBUTE_DATA_1         '\x01'
#define CMD_HEADER_READ_STATUS_ATTRIBUTE_DATA_2         '\x02'
#define CMD_HEADER_READ_STATUS_ATTRIBUTE_DATA_ALL       '\x00'
#define CMD_HEADER_READ_STATUS_SERVICE_ALL              '\x01'
#define CMD_HEADER_READ_STATUS_SERVICE_SINGLE           '\x0E'

#define RESPONSE_VALUE_READ_ROBOT_STATUS_MASK           0x08  // Running status on the bit3
#define RESPONSE_VALUE_READ_CONTROL_MODE_MASK           0xE0  // Teach mode on bit5, Play mode on bit6, Remote mode on bit7
#define RESPONSE_VALUE_READ_TOOL_IO_MASK                0x01  // OUT #01 on the bit0 2^0 = 1 - decimal
#define RESPONSE_VALUE_READ_CONVEYOR_IO_MASK            0x10  // OUT #05 on the bit4 2^4 = 16 - decimal

#define RATIO_PULSE_OVER_DEGREE_JOINT_S     34816.0/30.0
#define RATIO_PULSE_OVER_DEGREE_JOINT_L     102400.0/90.0
#define RATIO_PULSE_OVER_DEGREE_JOINT_U     51200.0/90.0
#define RATIO_PULSE_OVER_DEGREE_JOINT_R     10204.0/30.0    // The same for joint B and joint T

#define CMD_HEADER_MOVE_CARTESIAN_DATA_SIZE                 '\x68'  // Data size = 26*4 = 104 bytes - decimal
#define CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS         '\x01'
#define CMD_HEADER_MOVE_CARTESIAN_INSTANCE_STRAIGHT_ABS     '\x02'
#define CMD_HEADER_MOVE_CARTESIAN_INSTANCE_STRAIGHT_INC     '\x03'
#define CMD_HEADER_MOVE_CARTESIAN_ATTRIBUTE                 '\x01'  // Fixed to 1
#define CMD_HEADER_MOVE_CARTESIAN_SERVICE                   '\x02'  // Fixed to 2 to : write data to the specified coordinate

#define CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS                   '\x00'  // Joint speed
#define CMD_DATA_MOVE_SPEED_TYPE_V_SPEED                    '\x01'  // Linear speed cartesian
#define CMD_DATA_MOVE_SPEED_TYPE_VR_SPEED                   '\x02'  // Joint speed cartesian
#define CMD_DATA_MOVE_COORDINATE_TYPE_BASE                  '\x10'
#define CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT                 '\x11'
#define CMD_DATA_MOVE_COORDINATE_TYPE_USER                  '\x12'
#define CMD_DATA_MOVE_COORDINATE_TYPE_TOOL                  '\x13'

#define CMD_HEADER_SELECT_JOB_DATA_SIZE                     '\x24'
#define CMD_HEADER_SELECT_JOB_INSTANCE_SET_EXE_JOB          '\x01'
#define CMD_HEADER_SELECT_JOB_ATTRIBUTE_JOB_NAME            '\x01'
#define CMD_HEADER_SELECT_JOB_ATTRIBUTE_JOB_ALL             '\x00'
#define CMD_HEADER_SELECT_JOB_SERVICE_ALL                   '\x02'

#define CMD_HEADER_START_JOB_DATA_SIZE                      '\x04'
#define CMD_HEADER_START_JOB_INSTANCE                       '\x01'  // Fixed to 1
#define CMD_HEADER_START_JOB_ATTRIBUTE                      '\x01'  // Fixed to 1
#define CMD_HEADER_START_JOB_SERVICE                        '\x10'  // Fixed to 0x10

#define CMD_DATA_START_JOB                                  '\x01'  // Fixed to 1

#define CMD_HEADER_READ_JOB_DATA_SIZE                       '\x00'
#define CMD_HEADER_READ_JOB_ATTRIBUTE_ALL                   '\x00'
#define CMD_HEADER_READ_JOB_ATTRIBUTE_NAME                  '\x01'
#define CMD_HEADER_READ_JOB_ATTRIBUTE_LINE                  '\x02'
#define CMD_HEADER_READ_JOB_ATTRIBUTE_STEP                  '\x03'
#define CMD_HEADER_READ_JOB_ATTRIBUTE_SPEED                 '\x04'
#define CMD_HEADER_READ_JOB_SERVICE_SINGLE                  '\x0E'
#define CMD_HEADER_READ_JOB_SERVICE_ALL                     '\x01'

#define CMD_HEADER_TEACH_POSITION_DATA_SIZE                 '\x34'  // 13*4 = 52 - decimal
#define CMD_HEADER_TEACH_POSITION_ATTRIBUTE_ALL             '\x00'
#define CMD_HEADER_TEACH_POSITION_SERVICE_WRITE             '\x02'
#define CMD_HEADER_TEACH_POSITION_SERVICE_READ              '\x01'

#define CMD_DATA_TEACH_POSITION_COORDINATE_TYPE_PULSE       '\x00'
#define CMD_DATA_TEACH_POSITION_COORDINATE_TYPE_BASE        '\x10'
#define CMD_DATA_TEACH_POSITION_COORDINATE_TYPE_ROBOT       '\x11'
#define CMD_DATA_TEACH_POSITION_COORDINATE_TYPE_USER        '\x12'
#define CMD_DATA_TEACH_POSITION_COORDINATE_TYPE_TOOL        '\x13'

#define CMD_HEADER_WRITE_BYTE_DATA_SIZE                     '\x01'
#define CMD_HEADER_WRITE_BYTE_ATTRIBUTE                     '\x01'
#define CMD_HEADER_WRITE_BYTE_SERVICE_SET_ALL               '\x02'
#define CMD_HEADER_WRITE_BYTE_SERVICE_SET_SINGLE            '\x10'

#define CMD_HEADER_TEACH_PLURAL_POSITION_DATA_SIZE          '\x6C'  // In Ethernet document : 27*4 = 108 - decimal (2 positions)
#define CMD_HEADER_TEACH_PLURAL_POSITION_ATTRIBUTE_ALL      '\x00'  // Fixed to 0
#define CMD_HEADER_TEACH_PLURAL_POSITION_SERVICE_WRITE      '\x34'

#define CMD_HEADER_READ_IO_DATA_SIZE                        '\x00'  // Data is only valid when writing
#define CMD_HEADER_READ_IO_INSTANCE_OUTPUT_BYTE_1           '\xE9'
#define CMD_HEADER_READ_IO_INSTANCE_OUTPUT_BYTE_2           '\x03'
#define CMD_HEADER_READ_IO_ATTRIBUTE                        '\x01'  // Fixed to 1
#define CMD_HEADER_READ_IO_SERVICE_READ                     '\x0E'

#define CMD_HEADER_FILE_INSTANCE                            '\x00'
#define CMD_HEADER_FILE_ATTRIBUTE                           '\x00'
#define CMD_HEADER_FILE_SAVING_SERVICE                      '\x16'
#define CMD_HEADER_FILE_LOADING_SERVICE                     '\x15'
#define CMD_HEADER_FILE_LIST_SERVICE                        '\x32'
#define CMD_HEADER_FILE_DELETE_SERVICE                      '\x09'


#define CMD_ACK_REQUEST                                     '\x00'
#define CMD_ACK_REPLY                                       '\x01'

class motoman_command : public QObject
{
    Q_OBJECT
public:
    explicit motoman_command(QObject *parent = nullptr);

    QByteArray setServoOn(quint8 request_id_index);
    QByteArray setServoOff(quint8 request_id_index);
    QByteArray readControlMode(quint8 request_id_index);
    QByteArray readRobotPosition(quint8 request_id_index);
    QByteArray readRobotPulse(quint8 request_id_index);
    QByteArray readStatus(quint8 request_id_index);
    QByteArray setRobotCartesianPosition(quint8 request_id_index, quint8 coordinate_type,
                                         quint8 move_type, quint8 speed_type, double speed, QVector<double> *position);
    QByteArray selectJob(quint8 request_id_index, QString job_name, quint32 line);
    QByteArray startJob(quint8 request_id_index);
    QByteArray readJob(quint8 request_id_index, quint8 job_index);
    QByteArray teachRobotPosition(quint8 request_id_index, quint8 coordinate_type, int index, QVector<double> *position);
    QByteArray writeByte(quint8 request_id_index, int index, quint8 data_byte);
    QByteArray teachPluralRobotPosition(quint8 request_id_index, quint8 coordinate_type,int numb, int index, QVector<double> *position);
    QByteArray readIO(quint8 request_id_index);
    QByteArray saveFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_name);
    QByteArray loadFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_name);
    QByteArray listFile(quint8 request_id_index, quint8 ack, quint32 block_num, QString file_extension);
    QByteArray deleteFile(quint8 request_id_index, QString file_name);

signals:

private:
    QByteArray header_part;
    QByteArray data_part;
    QByteArray cartesian_move_data_part;
    QByteArray job_data_part;
    QByteArray teach_position_data_part;
    QByteArray teach_plural_position_data_part;
    QByteArray wrtie_byte_data_part;
};

#endif // MOTOMAN_COMMAND_H
