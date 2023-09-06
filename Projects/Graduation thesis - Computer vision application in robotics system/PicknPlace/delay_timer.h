#ifndef DELAY_TIMER_H
#define DELAY_TIMER_H

#include <QObject>
#include <QTime>
#include <QCoreApplication>

class delay_timer : public QObject
{
    Q_OBJECT
public:
    explicit delay_timer(QObject *parent = nullptr);

    void delay(int millisecondsToWait);

signals:

};

#endif // DELAY_TIMER_H
