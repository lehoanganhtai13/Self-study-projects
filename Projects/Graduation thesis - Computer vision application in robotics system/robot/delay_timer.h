#ifndef DELAY_TIMER_H
#define DELAY_TIMER_H

#include <QObject>
#include <QThread>
#include <QTime>
#include <QCoreApplication>

class delay_timer : public QThread
{
    Q_OBJECT
public:
    explicit delay_timer(QObject *parent = nullptr);

    void delay(int millisecondsToWait);

protected:
    void run();

signals:

};

#endif // DELAY_TIMER_H
