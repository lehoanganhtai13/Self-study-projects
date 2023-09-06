#ifndef USYSTEM_H
#define USYSTEM_H

#include <QObject>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QVector>

#include <yolo_seg.h>
#include <poseestimation.h>
#include <delay_timer.h>
#include <motoman_command.h>
#include <motoman_communication.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/dnn/dnn.hpp>
using namespace cv::dnn;

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
using namespace rs2;

#include <visp3/core/vpConfig.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/core/vpRzyxVector.h>

enum system_mode{
    detection,
    pose_estimation
};

struct CalibrationMatrix{
    double x,y,z,rx,ry,rz;
};

class uSystem : public QThread
{
    Q_OBJECT
public:
    explicit uSystem(QObject *parent = nullptr);

    void systemDetectRegister(cv::Mat img, rs2::depth_frame frame, std::string model_path, rs2_intrinsics intrinsics_param, std::vector<cv::Point> points, QString system_dir);

    void systemCalibrationRegister(CalibrationMatrix calibration_param);

    void systemRobotRegister(DH_table table);

    void getDetectResult(OutputSeg &output);

    void getPoseEstimateResult(double &x, double &y, double &z, double &rx, double &ry, double &rz);

    QVector<double> getApproachingPoint();

    void getImageDetected(cv::Mat &outputImg);

    void errorEvaluate(double x, double y, double z, double rx, double ry, double rz, double &er_x, double &er_y, double &er_z, double &er_ry, int level);

    void stop();

    virtual void run() override;

private:
    system_mode mode;
    bool system_running;

    int detect_count;

    cv::Mat imgInput;
    Net net;
    cv::Scalar color;
    std::vector<OutputSeg> result;
    std::string path;

    rs2_intrinsics intr;

    YoloSeg detect_model;
    PoseEstimation pose_model;

    CalibrationMatrix calib;

    cv::Mat mask;

    std::vector<cv::Point> limit_points;

    QString system_path;

    double sys_Rx, sys_Ry, sys_Rz, sys_x, sys_y, sys_z;
    QVector<double> approaching_point;

    QString datetime_str;
    int obj_count;

    delay_timer delayer;

    bool finished;
    bool head_up;

    motoman_communication robot;
    DH_table DH;

    int idx;

signals:
    void systemFinished();

    void requestNewFrame();

    void wrongCalculate();

    void objectDetected();

    void poseCalculated();

    void loadNetError();

};

#endif // USYSTEM_H
