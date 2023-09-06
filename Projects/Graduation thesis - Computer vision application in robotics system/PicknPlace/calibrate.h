#ifndef CALIBRATE_H
#define CALIBRATE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QString>

#include <cstdio>
#include <cstdint>

#include <visp3/core/vpConfig.h>
#include <visp3/vision/vpPose.h>
#include <visp3/detection/vpDetectorAprilTag.h>
#include <visp3/gui/vpDisplayOpenCV.h>
#include <visp3/core/vpXmlParserCamera.h>
#include <visp3/sensor/vpRealSense2.h>
#include <visp3/vision/vpHandEyeCalibration.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/surface_matching.hpp>
#include <opencv2/surface_matching/ppf_helpers.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>

class Calibration : public QThread
{
    Q_OBJECT
public:
    explicit Calibration(QObject *parent = nullptr);

    void registerInput(std::vector<vpHomogeneousMatrix> cMoi, std::vector<vpHomogeneousMatrix> wMei, int iteration, QString base2cam_log);

    vpHomogeneousMatrix HandEyeTransformation();

    void calibrateLeastSquares();

    void calibrateHandEye();

    void virtual run() override;

private:
    std::vector<vpHomogeneousMatrix> cMo, wMe;
    vpHomogeneousMatrix bMc;
    int N;
    QFile log_file;

signals:
    void finishCalculate(bool finished, int ret);

};

#endif // CALIBRATE_H
