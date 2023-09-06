#ifndef LICENSE_DIALOG_H
#define LICENSE_DIALOG_H

#include <QDialog>
#include <QDebug>
#include <license_server.h>
#include <QFile>
#include <QDir>
#include <QMessageBox>

namespace Ui {
class license_dialog;
}

class license_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit license_dialog(QWidget *parent = nullptr);
    ~license_dialog();

    void changeCount(int count);
    void lockCount();
    void unlockCount();

    bool license_checked;
    bool count_lock;

    license_server license;

private slots:
    void on_paste_clicked();

    void on_load_clicked();

    void on_license_type_clicked();

    void on_free_type_clicked();

signals:
    void updateDaysLeft(int, QString, bool);

private:
    Ui::license_dialog *ui;
};

#endif // LICENSE_DIALOG_H
