#ifndef MAIN_DIALOG_H
#define MAIN_DIALOG_H

#define _USE_MATH_DEFINES

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <visp3/core/vpConfig.h>
#include <visp3/vision/vpPose.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/core/vpXmlParserCamera.h>
#include <visp3/sensor/vpRealSense2.h>
#include <visp3/vision/vpHandEyeCalibration.h>

#include <vector>

#include <fileapi.h>
#include <windows.h>
#include <tuple>
#include <array>
#include <ctime>
#include <cstdlib>
#include <math.h>

#include <QDialog>
#include <QTimer>
#include <QDebug>
#include <delay_timer.h>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <yolo_seg.h>
#include <chrono>
#include <thread>
#include <QCameraInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QStandardPaths>
#include <QProgressDialog>
#include <QDateTime>
#include <QDate>
#include <QTime>

using namespace std;

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
using namespace rs2;

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/surface_matching.hpp>
#include <opencv2/surface_matching/ppf_helpers.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
//using namespace cv; fix error C2872: 'ACCESS_MASK': ambiguous symbol by comment this line

#include <opencv2/dnn/dnn.hpp>
using namespace cv::dnn;

#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafeatures2d.hpp>
using namespace cv::cuda;

#include <libusb.h>

#include <GLFW/glfw3.h>
#include <glut.h>

#include <motoman_communication.h>
#include <calibrate.h>
#include <poseestimation.h>
#include <camera.h>
#include <usystem.h>
#include <servo.h>
#include <evaluate_dialog.h>

//#include <Eigen/Core>
//#include <pcl/point_types.h>
//#include <pcl/point_cloud.h>
//#include <pcl/common/time.h>
//#include <pcl/console/print.h>
//#include <pcl/features/normal_3d_omp.h>
//#include <pcl/features/fpfh_omp.h>
//#include <pcl/filters/filter.h>
//#include <pcl/filters/voxel_grid.h>
//#include <pcl/io/pcd_io.h>
//#include <pcl/io/ply_io.h>
//#include <pcl/registration/icp.h>
//#include <pcl/registration/sample_consensus_prerejective.h>
//#include <pcl/segmentation/sac_segmentation.h>
//#include <pcl/visualization/pcl_visualizer.h>
//#include <vtkRenderWindow.h>
//#include <QVTKWidget.h>
//#include <vtkDebugLeaks.h>

namespace Ui {
class main_dialog;
}

//enum camera_module{
//    stereo_camera,
//    RGB_camera,
//    align_camera
//};

enum USB_type{
    none,
    usb_21,
    usb_31
};

enum mode{
    PickAndPlace,
    Calibrate
};

struct Point3D{
    double x, y, z;
};

class main_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit main_dialog(QWidget *parent = nullptr);
    ~main_dialog();

private slots:
    void on_camera_on_clicked();

    void turn_cam_on();

    void turn_cam_off();

    bool device_with_streams(vector<rs2_stream> stream_requests, string& out_serial);

    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    void on_module_currentIndexChanged(int index);

    void on_depth_unit_currentIndexChanged(int index);

    void on_preset_currentIndexChanged(int index);

    void on_start_clicked();

    void finished(int exitCode);

    void readyReadStandardOutput();

    void runSriptCallback();

    void positionCalculateCallback();

    void yolov5_segmentation();

    void on_select_detect_model_clicked();

    void on_select_system_folder_clicked();

    void on_select_calib_folder_clicked();

    void on_test_mode_currentIndexChanged(int index);

    void on_test_model_clicked();

    struct Point3D intrinsicsCalculation(rs2::depth_frame frame, int idx);

    void checkUSBSpeed(libusb_device **devs);

    void on_main_dialog_rejected();

    void on_calibrate_clicked();

    void updateReceiveUICallback(quint8 control_mode, bool general_response, QStringList job_list_received);

    DH_table readParameters(QJsonObject file, mode Mode);

    bool detectPoseMarker(cv::Mat &img, cv::Mat cameraMatrix, vector<double> distCoeffs, cv::Mat objPoints, cv::Vec3d &oRc, cv::Vec3d &oTc);

    void on_test_calib_clicked();

    void on_test_marker_clicked();

    void on_get_pose_clicked();

    void on_setup_clicked();

    void on_Evaluate_model_clicked();

public slots:
    void receiveCameraRGBFrame(double width, double height, double frame_rate);

    void receiveCamerDepthFrame(double width, double height, double frame_rate);

    void receiveCameraRGBDFrame(double width, double height, double frame_rate);

    void receiveTestingFrame(cv::Mat frame);

    void checkUSB();

    void distance(int event, int u, int v, int flags, void* param);

    void finishCalibCallback(bool finished, int ret);

    void cancelCalib();

    void mouseMove(int, int);

    void mouseClicked();

    void requestNewFrameCallback();

    void objectDetectedCallback();

    void systemFinishedCallback();

    void poseCalculatedCallback();

    void loadNetErrorCallback();

    void wrongCalculateCallback();

    void runRobotCallback();

    void pingFailCallback();

    void evaluateObject2CamCallback();

    void evaluateObject2BasePositionCallback();

    void evaluateObject2BaseRzCallback();

    void evaluateObject2BaseRyCallback();

    void evaluateObjectTimeCalculationCallback();

private:
    Ui::main_dialog *ui;

    Camera *cam;
    float depth_unit;
    config cfg;
    int preset;
    camera_module module;
    int frame_rate_color;
    int stream_width, stream_height;
    pipeline pipe;
    bool camera_run, sensor_run;

    colorizer color_map;
    rates_printer printer;
    delay_timer *delay, *delay2, *delay3;
    QPixmap img;

    QProcess *proc;
    bool system_capture, system_detecting, system_calculating, system_finish, request_frame, run_robot;
    float x, y, z;
    QString data_in;
    int x_min, x_max, y_min, y_max;
    rs2_intrinsics intr;
    bool detected;

    YoloSeg model;
    Net net;
    vector<cv::Scalar> color;
    vector<OutputSeg> result;

    int testing_mode;
    bool detecting;
    bool run_test;
    bool wait_frame;
    int show_graph_mode;

    QTimer *usb_timer;
    USB_type usb;

    rs2::pipeline_profile prof;

    double offset;

    cv::Mat main_frame;

    motoman_communication robot;
    bool read_pos, read_joint, read_status;
    QVector<double> robot_position, robot_joint;
    Calibration *calib;

    bool connect_status, finishCalib;
    QProgressDialog *calib_progress;
    int error;
    bool cancel_calib;

    bool test_marker;
    bool marker_ry, rotate_marker, translate_marker;

    const unsigned int N = 200;
    bool get_pose;

    QString system_dir;

    PoseEstimation pose;

    std::vector<cv::Point> working_area;
    cv::Mat system_img;

    uSystem *sys;
    bool showFrame, stop;

    QVector<double> approachingPoint, grippingPoint;
    bool status;

    DH_table system_robt_param;
    QVector<double> home;

    servo *stm32;

    Evaluate_dialog ev;
    bool evaluate;

    int evaluate_count;

signals:
    void runScript();
    void positionCalculate();
    void testingFrameSend(cv::Mat frame);
    void runRobot();
};

#endif // MAIN_DIALOG_H
