#ifndef EVALUATE_DIALOG_H
#define EVALUATE_DIALOG_H

#include <QDialog>
#include <delay_timer.h>

namespace Ui {
class Evaluate_dialog;
}

class Evaluate_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Evaluate_dialog(QWidget *parent = nullptr);
    ~Evaluate_dialog();

    void clearScreen();
    void getPredictedResult(double x, double y, double z, double rx, double ry, double rz);
    void getReference(double x, double y, double z, double rx, double ry, double rz);
    void getTimeCalculation(double t);
    void stopWait();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_clicked();

signals:
    void evaluateObject2Cam();
    void evaluateObject2BasePosition();
    void evaluateObject2BaseRz();
    void evaluateObject2BaseRy();
    void evaluateObjectTimeCalculation();

private:
    Ui::Evaluate_dialog *ui;
    bool wait;
    delay_timer delayer;

    double x_p, y_p, z_p, rx_p, ry_p, rz_p;
    double x_r, y_r, z_r, rx_r, ry_r, rz_r;
    double time;

public:
    int mode;
};

#endif // EVALUATE_DIALOG_H
