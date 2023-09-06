#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QDebug>
#include <QThread>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
using namespace rs2;

enum camera_module{
    stereo_camera,
    RGB_camera,
    align_camera
};

class Camera : public QThread
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = nullptr);

    void registerCamera(camera_module module, pipeline &p, colorizer color, rates_printer pr);

    void getFrame(cv::Mat &frame);

    void getDepthFrame(depth_frame &dep_frame);

    virtual void run() override;

    void stop();

    bool wait_frame;

private:
    camera_module cam_module;
    pipeline pipe;
    colorizer color_map;
    rates_printer printer;

    cv::Mat outputFrame;

    bool camera_run;


signals:
    void sendCameraRGBFrame(double width, double height, double frame_rate);

    void sendCamerDepthFrame(double width, double height, double frame_rate);

    void sendCameraRGBDFrame(double width, double height, double frame_rate);

};

#endif // CAMERA_H
