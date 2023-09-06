#include "license_dialog.h"
#include "ui_license_dialog.h"

license_dialog::license_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::license_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("License Options");
    count_lock = false;
    QDir dir;
    dir.setPath("C:\\Autoss\\Draft\\License");
    if(dir.exists()){
        QFile file("C:\\Autoss\\Draft\\License\\Key.txt");
        if(file.open(QIODevice::ReadOnly)){
            QString temp = file.readLine();
            file.flush();
            file.close();

            if(temp.size() < 19){
                if( ui->count->text().contains("0")){}
                else  ui->count->setText("0");
                if(ui->free_type->isChecked()){}
                else{
                    ui->license_type->setChecked(false);
                    ui->free_type->setChecked(true);
                }
                return;
            }

            int days_left = license.licenseProcess(temp);
            if(days_left + 1 > 0){
                ui->key->setText(temp);
                ui->free_type->setChecked(false);
                ui->license_type->setChecked(true);
                ui->count->setText("1");
            } else{
                if( ui->count->text().contains("0")){}
                else  ui->count->setText("0");
                if(ui->free_type->isChecked()){}
                else{
                    ui->license_type->setChecked(false);
                    ui->free_type->setChecked(true);
                }
            }

            emit updateDaysLeft(days_left, temp, true);
        }
    } else{
        if( ui->count->text().contains("0")){}
        else  ui->count->setText("0");
        if(ui->free_type->isChecked()){}
        else{
            ui->license_type->setChecked(false);
            ui->free_type->setChecked(true);
        }
        emit updateDaysLeft(-1, "", false);
    }
//    qDebug() << "initialize ";

}

license_dialog::~license_dialog()
{
    delete ui;
}

void license_dialog::changeCount(int count){
    if(count == 0){
        if(!(ui->count->text().contains("0"))) ui->count->setText(QString::number(count));
//        if(ui->license_type->isChecked()){
//            ui->license_type->setChecked(false);
//            ui->free_type->setChecked(true);
//        }
    }

    if(count == 1){
        if(!(ui->count->text().contains("1"))) ui->count->setText(QString::number(count));
    }
}

void license_dialog::lockCount(){
    count_lock = true;
}

void license_dialog::unlockCount(){
    count_lock = false;
}

void license_dialog::on_paste_clicked(){
    ui->key->paste();
}


void license_dialog::on_load_clicked(){
    if(!ui->license_type->isChecked()) return;

    // If get the unlock key, check to unlock the software
    license.unlockConsider();
    int days_left = license.licenseProcess(ui->key->text());

    QFile file("C:\\Autoss\\Draft\\License\\Lock.txt");
    if(file.open(QIODevice::ReadOnly)){
        if(file.readLine().contains("lock")){
            if(!ui->count->text().contains("0")) ui->count->setText("0");
            QMessageBox::critical(this, "Activate license error", "Your software has been locked. Please contact us for unlock key!");
            file.flush();
            file.close();
            return;
        } else{
            QMessageBox::information(this, tr("Activate license instruction"), "Enter the license and press refresh to activate");
            file.flush();
            file.close();
            file.remove();
            count_lock = false;
            return;
        }
    } else{
//        qDebug() << "count lock: " << count_lock;
        if(count_lock){
            if(!ui->count->text().contains("0")) ui->count->setText("0");
            QMessageBox::critical(this, "Activate license error", "Restore system time to activate the license!");
            return;
        }
    }

    if(days_left > 0){
        emit updateDaysLeft(days_left, ui->key->text(), true);
        ui->count->setText("1");
    } else{
        if(!ui->count->text().contains("0")) ui->count->setText("0");
        emit updateDaysLeft(0, ui->key->text(), true);
    }    
}

void license_dialog::on_license_type_clicked(){
    ui->free_type->setChecked(false);
}


void license_dialog::on_free_type_clicked(){
    ui->license_type->setChecked(false);
    ui->count->setText("0");
    emit updateDaysLeft(-1, "", false);
}

