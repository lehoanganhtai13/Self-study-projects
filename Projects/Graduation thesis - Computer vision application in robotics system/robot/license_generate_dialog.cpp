#include "license_generate_dialog.h"
#include "ui_license_generate_dialog.h"

license_generate_dialog::license_generate_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::license_generate_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("License generator");
    key_type = "license key";
    unlock_key_num = 1;
    license_duration = 1;
}

license_generate_dialog::~license_generate_dialog()
{
    delete ui;
}

void license_generate_dialog::on_key_type_currentIndexChanged(int index){
    if(index == 0){
        key_type = "license key";
    } else if(index == 1){
        key_type = "unlock key";
    }
}


void license_generate_dialog::on_license_duration_currentIndexChanged(int index){
    if(index == 0){
        license_duration = 1;
    } else if(index == 1){
        license_duration = 3;
    } else if(index == 2){
        license_duration = 6;
    } else if(index == 3){
        license_duration = 12;
    }
}


void license_generate_dialog::on_unlock_key_currentIndexChanged(int index){
    if(index == 0){
        unlock_key_num = 1;
    } else if(index == 1){
        unlock_key_num = 2;
    }
}


void license_generate_dialog::on_generate_key_clicked(){
    QDate expired_date = QDate::currentDate().addMonths(license_duration);
    int day = expired_date.day();
    int month = expired_date.month();
    int year = expired_date.year();
    int d1 = day / 10;
    int d2 = day - d1*10;
    int m1 = month / 10;
    int m2 = month - m1*10;
    int y1 = year / 1000;
    int y2 = (year - y1*1000) / 100;
    int y3 = (year - y1*1000 - y2*100) / 10;
    int y4 = year - y1*1000 - y2*100 - y3*10;

    if(key_type == "unlock key"){
        QString x1, x2, x3, x4, a, b, c, d, s1, s2, s3, s4, x5, x6, x7,x8;
        x1 = QString::number(QRandomGenerator::global()->bounded(1,3));
        x2 = QString::number(QRandomGenerator::global()->bounded(2,5));
        x3 = QString::number(QRandomGenerator::global()->bounded(3,7));
        x4 = QString::number(QRandomGenerator::global()->bounded(4,9));

        a = "A";
        b = "T";
        c = "U";
        d = "K";

        x8 = QString::number(QRandomGenerator::global()->bounded(1,3));
        x7 = QString::number(QRandomGenerator::global()->bounded(2,5));
        x6 = QString::number(QRandomGenerator::global()->bounded(3,7));
        x5 = QString::number(QRandomGenerator::global()->bounded(4,9));

        if(unlock_key_num == 1){
            s1 =  QString::number(1);
            s2 =  QString::number(3);
            s3 =  QString::number(1);
            s4 =  QString::number(0);
        } else if(unlock_key_num == 2){
            s1 =  QString::number(2);
            s2 =  QString::number(1);
            s3 =  QString::number(0);
            s4 =  QString::number(7);
        }

        QString license = x1 + x2 + x3 + x4 + "-" + a + b + c + d + "-" + s1 + s2 + s3 + s4 + "-" + x5 + x6 + x7 + x8;
        ui->key->setText(license);
        return;
    } else if(key_type == "license key"){
        int tmp = 0;
            int tmp1, tmp2, tmp3, tmp4, ts1, ts2, ts3, ts4, tmp_result;
            while(1){
                tmp = QRandomGenerator::global()->bounded(65,69);
                if(tmp % 2 != 0) break;
            }
            QString x1, s1;
            if(tmp - 60 >= d1){
                x1 = QString::number(tmp - 60 - d1);
                tmp1 = tmp - 60 - d1;
                s1 = "1";
                ts1 = 1;
            }else{
                x1 = QString::number(d1 - (tmp - 60));
                tmp1 = d1 - (tmp - 60);
                s1 = "0";
                ts1 = 0;
            }

            QString a;
            switch (tmp) {
            case 65:
                a = "A";
                break;
            case 67:
                a = "C";
                break;
            case 69:
                a = "E";
                break;
            default:
                break;
            }

            tmp = QRandomGenerator::global()->bounded(70,79);
            qDebug() << tmp;
            QString x4, s2;
            if(tmp - 70 >= d2){
                x4 = QString::number(tmp - 70 - d2);
                tmp4 = tmp - 70 - d2;
                s2 = "9";
                ts2 = 9;
            }else{
                x4 = QString::number(d2 - (tmp - 70));
                tmp4 = d2 - (tmp - 70);
                s2 = "8";
                ts2 = 8;
            }
            QString b;
            switch (tmp) {
            case 70:
                b = "F";
                break;
            case 71:
                b = "G";
                break;
            case 72:
                b = "H";
                break;
            case 73:
                b = "I";
                break;
            case 74:
                b = "J";
                break;
            case 75:
                b = "K";
                break;
            case 76:
                b = "L";
                break;
            case 77:
                b = "M";
                break;
            case 78:
                b = "N";
                break;
            case 79:
                b = "O";
                break;
            default:
                break;
            }

            while(1){
                tmp = QRandomGenerator::global()->bounded(80,88);
                if(tmp % 2 == 0) break;
            }
            QString x2, s3;
            if(tmp - 80 >= m1){
                x2 = QString::number(tmp - 80 - m1);
                tmp2 = tmp - 80 - m1;
                s3 = "3";
                ts3 = 3;
            }else{
                x2 = QString::number(m1 - (tmp - 80));
                tmp2 = m1 - (tmp - 80);
                s3 = "2";
                ts3 = 2;
            }
            QString c;
            switch (tmp) {
            case 80:
                c = "P";
                break;
            case 82:
                c = "R";
                break;
            case 84:
                c = "T";
                break;
            case 86:
                c = "V";
                break;
            case 88:
                c = "X";
                break;
            default:
                break;
            }

            while(1){
                tmp = QRandomGenerator::global()->bounded(81,89);
                if(tmp % 2 != 0) break;
            }
            QString x3, s4;
            if(tmp - 80 >= m2){
                x3 = QString::number(tmp - 80 - m2);
                tmp3 = tmp - 80 - m2;
                s4 = "7";
                ts4 = 7;
            }else{
                x3 = QString::number(m2 - (tmp - 80));
                tmp3 = m2 - (tmp - 80);
                s4 = "6";
                ts4 = 6;
            }
            QString d;
            switch (tmp) {
            case 81:
                d = "Q";
                break;
            case 83:
                d = "S";
                break;
            case 85:
                d = "U";
                break;
            case 87:
                d = "W";
                break;
            case 89:
                d = "Y";
                break;
            default:
                break;
            }

            QString x5, x6;
            int sum1 = tmp1 + tmp2 + tmp3 + tmp4;
            int sum2 = ts1 + ts2 + ts3 + ts4;
            if(sum1 >= sum2){
                x5 = QString::number(sum1 + sum2).split("").at(1);
                x6 = QString::number(sum1 + sum2).split("").at(2);
            } else{
                x5 = QString::number(sum2 + sum1).split("").at(2);
                x6 = QString::number(sum2 + sum1).split("").at(1);
            }

            if(tmp1 >= tmp3){
        //        x5 = QString::number(y3);
                tmp_result = tmp1 - tmp3 + y4;
            }
            else{
        //        x5 = QString::number(y4);
                tmp_result = tmp3 - tmp1 + y4;
            }
            QString x7 = QString::number(tmp_result);

            if(tmp2 >= tmp4){
        //        x6 = QString::number(y4);
                tmp_result = tmp2 - tmp4 + y3;
            }
            else{
        //        x6 = QString::number(y3);
                tmp_result = tmp4 - tmp2 + y3;
            }
            QString x8 = QString::number(tmp_result);
            qDebug() << x5;
            qDebug() << x6;
            qDebug() << x7;
            qDebug() << x8;

            qDebug() << "x1: " << x1;
            qDebug() << "x2: " << x2;
            qDebug() << "x3: " << x3;
            qDebug() << "x4: " << x4;
            qDebug() << "a: " << a;
            qDebug() << "b: " << b;
            qDebug() << "c: " << c;
            qDebug() << "d: " << d;
            qDebug() << "x5: " << x5;
            qDebug() << "x6: " << x6;
            qDebug() << "x7: " << x7;
            qDebug() << "x8: " << x8;
            qDebug() << "\n\n";
            QString license = x1 + x2 + x3 + x4 + "-" + a + b + c + d + "-" + s1 + s2 + s3 + s4 + "-" + x5 + x6 + x7 + x8;
            ui->key->setText(license);
            return;
    }
}

