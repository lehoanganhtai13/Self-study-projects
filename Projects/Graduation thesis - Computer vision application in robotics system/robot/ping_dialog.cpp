#include "ping_dialog.h"
#include "ui_ping_dialog.h"

ping_dialog::ping_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ping_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Command Prompt");

//    ui->watcher->setStyleSheet("QLabel { background-color : red; color : blue; }");
    this->setWindowIcon(QIcon(":/image/command prompt.jpg"));
    ui->watcher->clear();

    status = false;

    ip = "";
    data = "";
}

ping_dialog::~ping_dialog(){
    delete ui;
}

void ping_dialog::setIpAdress(QString ip){
    this->ip = ip;
}

void ping_dialog::run(){
    m_ping = new ping(this);
    m_ping->setIpAddress(ip);
    m_ping->start();
    connect(m_ping, SIGNAL(output(QString)), this, SLOT(outputCallback(QString)));
}

void ping_dialog::stop(){
    m_ping->stop();
    ui->watcher->clear();
    delay.delay(100);
    delete m_ping;
}

void ping_dialog::outputCallback(QString data){
//    qDebug() << "dataaaaaaaaa:" << this->data;
    if(data.contains("Finished")){
        delay.delay(1000);
        if(!this->data.contains("(0% loss)")){
//            qDebug() << "fail dataaaaaaaaa:" << this->data;
            status = false;
        } else status = true;
        this->data.clear();
        delay.delay(1000);
        m_ping->stop();
        delete m_ping;
//        qDebug() << "ping statusssss:" << status;
        emit pingStatus(status);
        ui->watcher->clear();
//        qDebug() << "clear watcherrrrrrr";
        return;
    }

    ui->watcher->appendPlainText(data);
    this->data.append(data);
    return;
}
