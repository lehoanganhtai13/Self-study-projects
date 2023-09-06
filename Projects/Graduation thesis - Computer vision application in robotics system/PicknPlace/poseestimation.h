#ifndef POSEESTIMATION_H
#define POSEESTIMATION_H

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <QObject>
#include <QString>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QDir>

#include <delay_timer.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <sstream>

class PoseEstimation : public QObject
{
    Q_OBJECT
public:
    explicit PoseEstimation(QObject *parent = nullptr);

    void getLine(cv::Point p1, cv::Point p2, double &a, double &b);
    void calculateDistance(cv::Mat, int &);
    bool isPointValid(cv::Mat, cv::Point, int, bool);
    void createBackground(cv::Mat&, double, double, double, double, std::string);
    void drawAxis(cv::Mat&, cv::Point, cv::Point, cv::Scalar, const float);
    bool findPointHead(cv::Mat, cv::Point, cv::Point &, bool, double, double);
    double getOrientation(const std::vector<cv::Point> &, cv::Mat&, std::vector<cv::Point> &, bool &);
    double run(cv::Mat&, QString, std::vector<cv::Point> &, int, bool &);

private:
    delay_timer delay;

signals:

};

#endif // POSEESTIMATION_H
