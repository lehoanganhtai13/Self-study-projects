#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include <QDialog>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QString>

namespace Ui {
class options_dialog;
}

class options_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit options_dialog(QWidget *parent = nullptr);
    ~options_dialog();

private slots:
    void on_open_clicked();

    void on_select_clicked();

    void on_rename_check_stateChanged(int arg1);

    void on_convert_check_stateChanged(int arg1);

private:
    void showInFolder(QString dir);

private:
    Ui::options_dialog *ui;
};

#endif // OPTIONS_DIALOG_H
