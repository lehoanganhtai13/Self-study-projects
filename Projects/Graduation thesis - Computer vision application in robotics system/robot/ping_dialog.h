#ifndef PING_DIALOG_H
#define PING_DIALOG_H

#include <QDialog>
#include <ping.h>
#include <QIcon>
#include <delay_timer.h>

namespace Ui {
class ping_dialog;
}

class ping_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit ping_dialog(QWidget *parent = nullptr);
    ~ping_dialog();

    void setIpAdress(QString ip);
    void run();
    void stop();

public slots:
    void outputCallback(QString data);

signals:
    void pingStatus(bool);

private:
    Ui::ping_dialog *ui;

    ping *m_ping;
    QString ip;

    delay_timer delay;
    bool status;
    QString data;
};

#endif // PING_DIALOG_H
