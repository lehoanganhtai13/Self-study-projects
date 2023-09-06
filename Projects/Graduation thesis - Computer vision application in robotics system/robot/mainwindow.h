#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QTimer>
#include <motoman_communication.h>
#include <QPalette>
#include <transfer_management.h>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRandomGenerator>
#include <license_server.h>
#include <delay_timer.h>
#include <QList>
#include <license_dialog.h>
#include <fileapi.h>
#include <windows.h>
#include <ping_dialog.h>
#include <ping.h>
#include <about_dialog.h>
#include <QMenuBar>
#include <QAction>
#include <license_generate_dialog.h>
#include <robodk_dialog.h>
#include <options_dialog.h>
#include <plugins_dialog.h>
#include <QCoreApplication>
#include <QPluginLoader>
#include <actionmanager.h>
#include <actioncontainer.h>
#include <QMetaType>
#include <PluginInterface.h>
#include <QPaintDevice>
#include <QDesktopWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

//    void ConvertCsvFileToList();
    bool GenerateJobList();
    int findMaxLine(QString dir);
    void ConnectionSetup(QString mode);
    void preCheckDateTime();



private:
    QString dir, program_dir, PC_program, main_program_dir, PC_main_program;
    QStringList list;
    QList<int> line_list, max_line_list;
    unsigned int list_size;
    bool main_program_flag;
    bool auto_get_pos;
    QTimer *timer, *timer2, *timer3, *timer4, *timer5, *timer6, *timer7;
    bool cartesian, pulse, joint;
    QPalette red_base = palette();
    QPalette green_base = palette();
    QPalette yellow_base = palette();
    bool ping_status, ping_checkable, cont_ping;
    QString cur_ip;
    bool connection_status, fail_to_connect_status, update_control_mode_status, update_postion_status, update_robot_status, servo_on_status, servo_off_status, update_delete_status, update_list_status, select_job_status, start_job_status, read_job_status;
    quint8 robot_status, control_mode;
    QString control_mode_str;
    bool servo_status;
    QString controller, brand;
    QStringList controller_list, brand_list;
    QString transfer_mode;
    QString ip_address;
    bool delete_busy_status, list_busy_status, program_running_status;
    QProgressDialog *upload_progress, *download_progress;
    QFile *temp;
    int total_bytes;
    QString transfer;
    bool system_del;
    QString sys_pro_del;
    bool sys_del_activate;
    int percentage;

    bool next_upload, end_process, start_pressed;
    QString cur_program, cur_tranferring;
    quint32 cur_line;

    QString running_mode;
    int line_index, program_index;
    QString previous_mode;

    bool license_activate, license_checkable, cheating_status, lock_status;
    int cheating_count;

    bool Autoss_key_managment;
    QAction *license_action;

    motoman_communication motoman_communication;
    transfer_management ftp;
    license_server license;
    delay_timer *delay, *delay2;
    license_dialog *dialog;
    ping_dialog *dialog1;
    ping m_ping;
    about_dialog *dialog2;
    license_generate_dialog *dialog3;
    robodk_dialog *dialog4;
    options_dialog *dialog5;

    plugins_dialog *dialog6;
    PluginInterface *pluginInterface, *pluginInterface2, *pluginInterface3, *pluginInterface4, *pluginInterface5, *pluginInterface6, *pluginInterface7;
    QAction *pluginAction, *pluginAction2, *pluginAction3, *pluginAction4, *pluginAction5, *pluginAction6, *pluginAction7;
    QObject *plugin, *plugin2, *plugin3, *plugin4, *plugin5, *plugin6, *plugin7;
    QPluginLoader plugin_loader, plugin_loader2, plugin_loader3, plugin_loader4, plugin_loader5, plugin_loader6, plugin_loader7;
    QString application_homepath;
    QStringList plugin_list;

    QString yaskawa_security_key = "9999999999999999";

private slots:
    void on_path_browse_clicked();

    void on_path_textChanged(const QString &arg1);

    void on_select_single_program_clicked();

    void on_program_selected_textChanged(const QString &arg1);

    void on_select_main_program_clicked();

    void on_main_program_existence_currentIndexChanged(int index);

    void on_Connect_clicked();

    void on_disconnect_clicked();

    void on_servo_on_clicked();

    void on_servo_off_clicked();

    void on_position_type_currentIndexChanged(int index);

    void on_Get_Position_clicked();

    void on_update_mode_currentIndexChanged(int index);

    void on_save_job_clicked();

    void on_brand_currentIndexChanged(int index);

    void on_controller_currentIndexChanged(int index);

    void on_transferring_mode_currentIndexChanged(int index);

    void on_transfer_program_clicked();

    void on_list_job_clicked();

    void on_delete_job_clicked();


    void on_start_job_clicked();

    void on_start_program_clicked();

    void on_program_transfering_textChanged(const QString &arg1);

    void on_restore_clicked();

//    void on_test_clicked();

    void on_running_mode_currentIndexChanged(int index);

    void on_resume_running_clicked();

    void on_program_backup_currentIndexChanged(int index);

    void on_license_triggered();

    void on_exit_triggered();

    void on_uninstall_triggered();

    void on_about_triggered();

    void on_RoboDK_clicked();

    void on_Options_triggered();

    void on_Plugins_triggered();

public slots:
    void updateReceiveUICallback(quint8, bool, QStringList);
    void timerCallback();
    void timer2Callback();
    void timer3Callback();
    void timer4Callback();
    void timer5Callback();
    void timer6Callback();
    void timer7Callback();
    void transferCallback(bool);
    void bytesCallback(int);
    void startJobCallback();
    void daysleftCallback(int,QString,bool);
    void pingCallback(bool);
    void autossKeyManagementCallback(bool);
    void licenseActionTriggered(bool);
    void loadPluginCallback(bool);
    void pluginActionTriggered(bool);
    void pluginActionTriggered2(bool);
    void pluginActionTriggered3(bool);
    void pluginActionTriggered4(bool);
    void pluginActionTriggered5(bool);
    void pluginActionTriggered6(bool);
    void pluginActionTriggered7(bool);

signals:
    void closeWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
