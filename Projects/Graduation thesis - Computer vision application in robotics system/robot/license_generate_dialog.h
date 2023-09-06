#ifndef LICENSE_GENERATE_DIALOG_H
#define LICENSE_GENERATE_DIALOG_H

#include <QDialog>
#include <QDate>
#include <QRandomGenerator>
#include <QDebug>

namespace Ui {
class license_generate_dialog;
}

class license_generate_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit license_generate_dialog(QWidget *parent = nullptr);
    ~license_generate_dialog();

private:
    QString key_type;
    int license_duration, unlock_key_num;

private slots:
    void on_key_type_currentIndexChanged(int index);

    void on_license_duration_currentIndexChanged(int index);

    void on_unlock_key_currentIndexChanged(int index);

    void on_generate_key_clicked();

private:
    Ui::license_generate_dialog *ui;
};

#endif // LICENSE_GENERATE_DIALOG_H
