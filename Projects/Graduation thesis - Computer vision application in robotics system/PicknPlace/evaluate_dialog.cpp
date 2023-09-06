#include "evaluate_dialog.h"
#include "ui_evaluate_dialog.h"

Evaluate_dialog::Evaluate_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Evaluate_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Model Evaluation");
    mode = 0;
}

Evaluate_dialog::~Evaluate_dialog()
{
    delete ui;
}

void Evaluate_dialog::clearScreen(){
    ui->x_predicted->clear();
    ui->y_predicted->clear();
    ui->z_predicted->clear();
    ui->rx_predicted->clear();
    ui->ry_predicted->clear();
    ui->rz_predicted->clear();
    ui->x_reference->clear();
    ui->y_reference->clear();
    ui->z_reference->clear();
    ui->rx_reference->clear();
    ui->ry_reference->clear();
    ui->rz_reference->clear();
}

void Evaluate_dialog::getPredictedResult(double x, double y, double z, double rx, double ry, double rz){
    x_p = x;
    y_p = y;
    z_p = z;
    rx_p = rx;
    ry_p = ry;
    rz_p = rz;
}

void Evaluate_dialog::getReference(double x, double y, double z, double rx, double ry, double rz){
    x_r = x;
    y_r = y;
    z_r = z;
    rx_r = rx;
    ry_r = ry;
    rz_r = rz;
}

void Evaluate_dialog::getTimeCalculation(double t){
    time = t;
}

void Evaluate_dialog::on_comboBox_currentIndexChanged(int index){
    mode = index;
}


void Evaluate_dialog::on_pushButton_clicked(){
    if(mode == 0){
        wait = true;
        clearScreen();
        emit evaluateObject2Cam();
        while(wait){
            delayer.delay(20);
        }
        ui->x_predicted->setText(QString::number(x_p));
        ui->y_predicted->setText(QString::number(y_p));
        ui->z_predicted->setText(QString::number(z_p));
        ui->ry_predicted->setText(QString::number(ry_p));
        ui->rz_predicted->setText(QString::number(rz_p));
        ui->x_reference->setText(QString::number(x_r));
        ui->y_reference->setText(QString::number(y_r));
        ui->z_reference->setText(QString::number(z_r));
        ui->rx_reference->setText(QString::number(rx_r));
        ui->ry_reference->setText(QString::number(ry_r));
        ui->rz_reference->setText(QString::number(rz_r));
    } else if(mode == 1){
        wait = true;
        clearScreen();
        emit evaluateObject2BasePosition();
        while(wait){
            delayer.delay(20);
        }
        ui->x_predicted->setText(QString::number(x_p));
        ui->y_predicted->setText(QString::number(y_p));
        ui->z_predicted->setText(QString::number(z_p));
        ui->x_reference->setText(QString::number(x_r));
        ui->y_reference->setText(QString::number(y_r));
        ui->z_reference->setText(QString::number(z_r));
    } else if(mode == 2){
        wait = true;
        clearScreen();
        emit evaluateObject2BaseRy();
        while(wait){
            delayer.delay(20);
        }
        ui->ry_predicted->setText(QString::number(ry_p));
        ui->ry_reference->setText(QString::number(ry_r));
    } else if(mode == 3){
        wait = true;
        clearScreen();
        emit evaluateObject2BaseRz();
        while(wait){
            delayer.delay(20);
        }
        ui->rz_predicted->setText(QString::number(rz_p));
        ui->rz_reference->setText(QString::number(rz_r));
    } else if(mode == 4){
        wait = true;
        clearScreen();
        emit evaluateObjectTimeCalculation();
        while(wait){
            delayer.delay(20);
        }
        ui->time->setText(QString::number(time));
    }
}

void Evaluate_dialog::stopWait(){
    wait = false;
}

