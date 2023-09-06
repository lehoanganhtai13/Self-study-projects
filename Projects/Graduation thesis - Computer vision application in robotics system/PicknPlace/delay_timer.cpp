#include "delay_timer.h"

delay_timer::delay_timer(QObject *parent)
    : QObject{parent}
{

}

void delay_timer::delay(int millisecondsToWait){
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
        //        qDebug() << "done time";
}
