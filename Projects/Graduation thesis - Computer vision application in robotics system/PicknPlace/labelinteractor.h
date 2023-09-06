#ifndef LABELINTERACTOR_H
#define LABELINTERACTOR_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QPointF>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class LabelInteractor : public QLabel
{
    Q_OBJECT
public:
    explicit LabelInteractor(QWidget *parent = nullptr);

    void getRealMousePosition(std::vector<cv::Point> &point_set);

    void getMousePosition(int &, int &);

    void mouseMoveEvent(QMouseEvent *mouse_event) override;

    void mouseReleaseEvent(QMouseEvent *mouse_event) override;

    void paintEvent(QPaintEvent *event) override;

    void clearLabel();

private:
    int u, v;
    QList<QPointF> points;
    int count;

    std::vector<cv::Point> real_points;

signals:
    void mouseMove(int, int);

    void mouseClicked();

};

#endif // LABELINTERACTOR_H
