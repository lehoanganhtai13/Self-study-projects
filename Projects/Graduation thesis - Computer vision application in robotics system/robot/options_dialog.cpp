#include "options_dialog.h"
#include "ui_options_dialog.h"

options_dialog::options_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::options_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Options");

    QFile file("C:\\Autoss\\Draft\\Settings\\save.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream str(&file);
        ui->path->setText(str.readAll());
        file.flush();
        file.close();
    }

    file.setFileName("C:\\Autoss\\Draft\\Settings\\rename.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream str(&file);
        if(str.readAll().contains("1")) ui->rename_check->setChecked(true);
        else ui->rename_check->setChecked(false);
        file.flush();
        file.close();
    }

    file.setFileName("C:\\Autoss\\Draft\\Settings\\convert.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream str(&file);
        if(str.readAll().contains("1")) ui->convert_check->setChecked(true);
        else ui->convert_check->setChecked(false);
        file.flush();
        file.close();
    }
}

options_dialog::~options_dialog()
{
    delete ui;
}

void options_dialog::showInFolder(QString dir){
    #ifdef _WIN32    //Code for Windows
        QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(dir)});
    #elif defined(__APPLE__)    //Code for Mac
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to reveal POSIX file \"" + dir + "\""});
        QProcess::execute("/usr/bin/osascript", {"-e", "tell application \"Finder\" to activate"});
    #endif
}

void options_dialog::on_open_clicked(){
//    qDebug() << ui->path->text();
    showInFolder(ui->path->text());
}


void options_dialog::on_select_clicked(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Path"));
    if(dir.size() == 0){
        return;
    }
    QFile file("C:\\Autoss\\Draft\\Settings\\save.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        str << dir;
        file.flush();
        file.close();

        ui->path->setText(dir);

        QDir remove_dir(QDir::homePath() + "/Program");
        if(remove_dir.exists()){
            remove_dir.removeRecursively();
        }
    }
}


void options_dialog::on_rename_check_stateChanged(int arg1){
    QFile file("C:\\Autoss\\Draft\\Settings\\rename.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        if(arg1 == 0){
            str << "0";
        } else if(arg1 > 0){
            str << "1";
        }
        file.flush();
        file.close();
    }
}


void options_dialog::on_convert_check_stateChanged(int arg1)
{
    QFile file("C:\\Autoss\\Draft\\Settings\\convert.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        if(arg1 == 0){
            str << "0";
        } else if(arg1 > 0){
            str << "1";
        }
        file.flush();
        file.close();
    }
}

