#include "labelinteractor.h"

LabelInteractor::LabelInteractor(QWidget *parent)
    : QLabel{parent}
{
    setMouseTracking(true);
    count = 0;
}

void LabelInteractor::getRealMousePosition(std::vector<cv::Point> &point_set){
    point_set = real_points;
}

void LabelInteractor::getMousePosition(int &x, int &y){
    x = u;
    y = v;
}

void LabelInteractor::mouseMoveEvent(QMouseEvent *mouse_event){
    u = mouse_event->x();
    v = mouse_event->y();

//    qDebug() << "u:" << u;
//    qDebug() << "v:" << v;

    emit mouseMove(u, v);
}

void LabelInteractor::mouseReleaseEvent(QMouseEvent *mouse_event){
    if(mouse_event->button() == Qt::LeftButton){
        qDebug() << "Mouse's clicked";

        if(count == 4){
            count = 1;
            points.clear();
            real_points.clear();
        } else count = count + 1;

        real_points.push_back(cv::Point(u,v));
        points.append(mouse_event->pos());
        update();

        emit mouseClicked();
    }
}

void LabelInteractor::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    QBrush brush;
    QColor red = Qt::red;
    red.setAlphaF(0.5);
    brush.setColor(red);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    QPointF polyPoints[4];

    for(int i = 0; i < points.size(); i++){
        painter.setPen(QPen(Qt::red, 10));
        painter.drawPoint(points.at(i));
        polyPoints[i] = points.at(i);
        if(i > 0){
            painter.setPen(QPen(Qt::red, 3));
            painter.drawLine(points.at(i - 1), points.at(i));
        }
        if(i == 3){
            painter.setPen(QPen(Qt::red, 3));
            painter.drawLine(points.at(3), points.at(0));
            painter.drawPolygon(polyPoints, 4);
        }
    }
    painter.end();
}

void LabelInteractor::clearLabel(){
    points.clear();
    QPainter painter(this);

    for(int i = 0; i < points.size(); i++){
        painter.setPen(QPen(Qt::red, 10));
        painter.drawPoint(points.at(i));
        if(i > 0){
            painter.setPen(QPen(Qt::red, 3));
            painter.drawLine(points.at(i - 1), points.at(i));
        }
    }
    painter.end();
}
