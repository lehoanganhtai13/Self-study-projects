#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /*
     * In order to generate executable file:
     * + Change debug mode into release mode
     * + Open command prompt of the compiler used (ex: mingw 64-bit), cd "directory_of_the_exe_file_generated", and type windeployqt "project_name.exe"
     */

    ui->setupUi(this);
    this->setWindowTitle("BKT - Robot Control Panel");

    int scale = (int)((float)QPaintDevice::logicalDpiX() / 96.0 * 100.0);
    QDesktopWidget app;
    const QRect resolution = app.availableGeometry(this);
    int height = resolution.height();
    int width = resolution.width();
    qDebug() << height << width;
    if(!(height >= 1000 && width >= 1920)){
        QMessageBox::StandardButton confirm = QMessageBox::question(this, "Resolution Incompatibility", "Since user resolution is lower 1920 x 1080, it may cause some display errors.\nStill want to open the application?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(confirm == QMessageBox::No){
            throw;
        }
    }
    if(!(scale == 100)){
        QString msg = "Since user scale is " + QString::number(scale) + "%, it may cause some display errors.\nStill want to open the application?";
        QMessageBox::StandardButton confirm = QMessageBox::question(this, "Scale Incompatibility", msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(confirm == QMessageBox::No){
            throw;
        }
    }

    // Setup license dialog
    dialog = new license_dialog(this);
    connect(dialog, SIGNAL(updateDaysLeft(int,QString,bool)), this, SLOT(daysleftCallback(int,QString,bool)));

    // Setup ping dialog
    dialog1 = new ping_dialog(this);
    connect(dialog1, SIGNAL(pingStatus(bool)), this, SLOT(pingCallback(bool)));
    ping_status = false;
    ping_checkable = false;

    // Setup about dialog
    dialog2 = new about_dialog(this);

    ui->total_progess_bar->setValue(0);
    percentage = 0;


    red_base.setColor(QPalette::Base, QColor(Qt::red));
    green_base.setColor(QPalette::Base, QColor(Qt::green));
    yellow_base.setColor(QPalette::Base, QColor(Qt::yellow));

    main_program_flag = 0;

    connection_status = false;
    fail_to_connect_status = false;
    update_control_mode_status = false;
    control_mode_str = "none";
    robot_status = 0;
    update_postion_status = false;
    update_robot_status = false;
    servo_on_status = false;
    servo_off_status = false;
    update_delete_status = false;
    update_list_status = false;
    delete_busy_status = false;
    list_busy_status = false;
    select_job_status = false;
    start_job_status = false;
    program_running_status = false;
    system_del = false;
    sys_del_activate = false;
    cont_ping = false;
    cur_ip = "";
    connect(&motoman_communication, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));

    // Update position
    auto_get_pos = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerCallback()));

    // Update control mode
    timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(timer2Callback()));

    // Update robot status
    timer3 = new QTimer(this);
    connect(timer3, SIGNAL(timeout()), this, SLOT(timer3Callback()));

    // delete command busy status
    timer4 = new QTimer(this);
    connect(timer4, SIGNAL(timeout()), this, SLOT(timer4Callback()));

    // list command busy status
    timer5 = new QTimer(this);
    connect(timer5, SIGNAL(timeout()), this, SLOT(timer5Callback()));

    // Update current running program information
    timer6 = new QTimer(this);
    connect(timer6, SIGNAL(timeout()), this, SLOT(timer6Callback()));

    // timer for delay
    delay = new delay_timer(this);
    delay2 = new delay_timer(this);

    // Update license status
    timer7 = new QTimer(this);
    connect(timer7, SIGNAL(timeout()), this, SLOT(timer7Callback()));

    // Initiallize controller for each brand chosen
    controller_list << "DX100" << "DX200" << "YRC1000micro" << "R-J3ib" << "R-30iA" << "R-30iB" << "KRC2" << "KRC3" << "KRC4";
    brand_list << "Motoman" << "Fanuc" << "Kuka";
    brand = brand_list[0];
    controller = controller_list[0];

    connect(&ftp, SIGNAL(finishTransfer(bool)), this, SLOT(transferCallback(bool)));
    connect(&ftp, SIGNAL(bytesUploaded(int)), this, SLOT(bytesCallback(int)));

    connect(&motoman_communication, SIGNAL(startJob()), this, SLOT(startJobCallback()));
    running_mode = "pointer in controller";

    lock_status = false;
    cheating_status = false;

    QDir dir("C:\\Autoss\\Draft\\License");
    if(dir.exists()){
//        qDebug() << "exist";
        QFile license_file("C:\\Autoss\\Draft\\License\\Key.txt");
        if(license_file.open(QIODevice::ReadOnly)){
//            qDebug() << "in file";
            QString temp = license_file.readLine();

            if(temp.size() < 19 || lock_status){
                license_checkable = false;
            }
            license_checkable = true;
        }
    }

    // Setup checking Autoss management key
    Autoss_key_managment = false;
    connect(&license, SIGNAL(confirmAutossManagement(bool)), this, SLOT(autossKeyManagementCallback(bool)));
    license_action = new QAction(QIcon(":/image/logo.ico"), "License generate", this);
    dialog3 = new license_generate_dialog(this);
    connect(license_action, SIGNAL(triggered(bool)),this, SLOT(licenseActionTriggered(bool)));

    preCheckDateTime();
    timer7->start(500);

    // Setup RoboDK dialog
    dialog4 = new robodk_dialog(this);

    // Creating settings folder
    QDir setting_dir("C:\\Autoss\\Draft\\Settings");
    if(!setting_dir.exists()){
        if(setting_dir.mkdir("C:\\Autoss\\Draft\\Settings")){
            SetFileAttributesW(L"C:\\Autoss\\Draft\\Settings", FILE_ATTRIBUTE_HIDDEN);
            QFile file("C:\\Autoss\\Draft\\Settings\\save.txt");
            if(file.open(QIODevice::WriteOnly)){
                QTextStream str(&file);
                str << QDir::homePath() + "/Program";
                file.flush();
                file.close();

                file.setFileName("C:\\Autoss\\Draft\\Settings\\rename.txt");
                if(file.open(QIODevice::WriteOnly)){
                    str.setDevice(&file);
                    str << "1";
                    file.flush();
                    file.close();

                    file.setFileName("C:\\Autoss\\Draft\\Settings\\convert.txt");
                    if(file.open(QIODevice::WriteOnly)){
                        str.setDevice(&file);
                        str << "1";
                        file.flush();
                        file.close();

                        QDir save_dir(QDir::homePath() + "/Program");
                        if(!save_dir.exists()) save_dir.mkdir(QDir::homePath() + "/Program");
                    }
                }
            }
        }
    }

    // Setup options dialog
    dialog5 = new options_dialog(this);

    // Setup plugins folder
    application_homepath = QCoreApplication::applicationDirPath();
    QDir plugins_dir("C:\\Autoss\\Draft\\Plugins");
    if(!plugins_dir.exists()){
        plugins_dir.mkdir("C:\\Autoss\\Draft\\Plugins");
        QString plugins_folder = "C:\\Autoss\\Draft\\Plugins";
        LPCWSTR new_dir = (const wchar_t*) plugins_folder.utf16();
        SetFileAttributesW(new_dir, FILE_ATTRIBUTE_HIDDEN);
        QFile plugin_list("C:\\Autoss\\Draft\\Plugins\\plugin_list.txt");
        if(plugin_list.open(QIODevice::WriteOnly)){
            plugin_list.flush();
            plugin_list.close();
            QFile load_list("C:\\Autoss\\Draft\\Plugins\\load_list.txt");
            if(load_list.open(QIODevice::WriteOnly)){
                QTextStream str(&load_list);
                str << "0,0,0,0,0,0,0";
                plugin_list.flush();
                plugin_list.close();
            }
        }
    }

    QDir robot_info_dir("C:\\Autoss\\Draft\\Robot_info");
    if(!robot_info_dir.exists()){
        qDebug() << "in";
        robot_info_dir.mkdir(("C:\\Autoss\\Draft\\Robot_info"));
        QString info_folder = "C:\\Autoss\\Draft\\Robot_info";
        LPCWSTR new_dir = (const wchar_t*) info_folder.utf16();
        SetFileAttributesW(new_dir, FILE_ATTRIBUTE_HIDDEN);
        QFile info_file("C:\\Autoss\\Draft\\Robot_info\\mode.txt");
        if(info_file.open(QIODevice::WriteOnly)){
            info_file.flush();
            info_file.close();
            info_file.setFileName("C:\\Autoss\\Draft\\Robot_info\\status.txt");
            if(info_file.open(QIODevice::WriteOnly)){
                info_file.flush();
                info_file.close();
                info_file.setFileName("C:\\Autoss\\Draft\\Robot_info\\servo.txt");
                if(info_file.open(QIODevice::WriteOnly)){
                    info_file.flush();
                    info_file.close();
                    info_file.setFileName("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
                    if(info_file.open(QIODevice::WriteOnly)){
                        info_file.flush();
                        info_file.close();
                    }
                }
            }
        }
    }

    // Setup plugins dialog
    dialog6 = new plugins_dialog(this);
    connect(dialog6, SIGNAL(loadPlugin(bool)),this, SLOT(loadPluginCallback(bool)));
    plugin_list.clear();
    dialog6->updatePlugin();
}

MainWindow::~MainWindow()
{
    if(timer->isActive()){
        timer->stop();
        delay->delay(200);
    }
    if(timer2->isActive()){
        timer2->stop();
        delay->delay(200);
    }
    if(timer3->isActive()){
        timer3->stop();
        delay->delay(200);
    }
    if(timer4->isActive()){
        timer4->stop();
        delay->delay(200);
    }
    if(timer5->isActive()){
        timer5->stop();
        delay->delay(200);
    }
    if(timer6->isActive()){
        timer6->stop();
        delay->delay(200);
    }
    if(timer7->isActive()){
        timer7->stop();
        delay->delay(200);
    }

    delete timer;
    delete timer2;
    delete timer3;
    delete timer4;
    delete timer5;
    delete timer6;
    delete timer7;
    delete temp;
    delete dialog;
    delete dialog1;
    delete dialog2;
    delete dialog3;
    delete dialog4;
    delete dialog5;
    delete dialog6;
    delete delay;
    delete upload_progress;
    delete download_progress;
    delete ui;
}

void MainWindow::updateReceiveUICallback(quint8 control_mode, bool general_response, QStringList job_list_received){
//    qDebug() << "call back";
//    qDebug() << select_job_status;
    if(select_job_status){
        if(general_response){
            ui->evet_log->appendPlainText("Program's selected");
            ui->evet_log->appendPlainText("");
        } else{
            ui->evet_log->appendPlainText("Failed to select program");
            ui->evet_log->appendPlainText("");
        }
        select_job_status = false;
    }

//    qDebug() << start_job_status;
    if(start_job_status){
        if(general_response){
            ui->evet_log->appendPlainText("Program's running");
            ui->evet_log->appendPlainText("");
        } else{
            ui->evet_log->appendPlainText("Failed to start program");
            ui->evet_log->appendPlainText("");
        }
        start_job_status = false;
    }

//    qDebug() << update_delete_status;
    if(update_delete_status){
        if(general_response){
            if(system_del){
                ui->evet_log->appendPlainText(sys_pro_del + "'s deleted");
                ui->evet_log->appendPlainText("");
                system_del = false;
            } else{
                ui->evet_log->appendPlainText(ui->job_list->currentText() + "'s deleted");
                ui->evet_log->appendPlainText("");
            }
        } else{
            ui->evet_log->appendPlainText("Failed to delete program");
            ui->evet_log->appendPlainText("");
        }
        update_delete_status = false;
        motoman_communication.motomanFileControlDisconnect();
    }

    if(read_job_status && (transfer_mode == "automatically transfer" || transfer_mode == "automatically transfer and run" || transfer_mode == "transfer and resume")){
        cur_program = motoman_communication.name_info + ".JBI";
        cur_line = motoman_communication.info;

        QDir dir("C:\\Autoss\\Draft\\Job_info");
        if(!dir.exists()){
            dir.mkpath("C:\\Autoss\\Draft\\Job_info");
            SetFileAttributesW(L"C:\\Autoss", FILE_ATTRIBUTE_HIDDEN);
            SetFileAttributesW(L"C:\\Autoss\\Draft\\Job_info", FILE_ATTRIBUTE_HIDDEN);
        }
        QFile f("C:\\Autoss\\Draft\\Job_info\\info.txt");
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream str(&f);
        str << cur_program << " " << cur_line;
        f.flush();
        f.close();

        read_job_status = false;
    }

    if(update_control_mode_status){
//        qDebug() << "mode in";
//        qDebug() << control_mode_str;
//        qDebug() << control_mode;
        QFile info_file("C:\\Autoss\\Draft\\Robot_info\\mode.txt");
        if(!info_file.open(QIODevice::WriteOnly)){
            QMessageBox::warning(this, "Write robot status errors", "Failed to open log text.");
        }

        QTextStream str_info(&info_file);
        if(control_mode_str == "teach"){
            if(control_mode == 64){
                control_mode_str = "play";
                ui->evet_log->appendPlainText("Play mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "play";
            } else if(control_mode == 192){
                control_mode_str = "remote";
                ui->evet_log->appendPlainText("Remote mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "remote";
            } else str_info << "teach";
        } else if(control_mode_str == "play"){
            if(control_mode == 32){
                control_mode_str = "teach";
                ui->evet_log->appendPlainText("Teach mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "teach";
            } else if(control_mode == 192){
                control_mode_str = "remote";
                ui->evet_log->appendPlainText("Remote mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "remote";
            } else str_info << "play";
        } else if(control_mode_str == "remote"){
            if(control_mode == 32){
                control_mode_str = "teach";
                ui->evet_log->appendPlainText("Teach mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "teach";
            } else if(control_mode == 64){
                control_mode_str = "play";
                ui->evet_log->appendPlainText("Play mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "play";
            } else str_info << "remote";
        } else if(control_mode_str == "none"){
            if(control_mode == 32){
                control_mode_str = "teach";
                ui->evet_log->appendPlainText("Teach mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "teach";
            } else if(control_mode == 64){
                control_mode_str = "play";
                ui->evet_log->appendPlainText("Play mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "play";
            } else if(control_mode == 192){
                control_mode_str = "remote";
                ui->evet_log->appendPlainText("Remote mode is on");
                ui->evet_log->appendPlainText("");
                str_info << "remote";
            }
            info_file.flush();
            info_file.close();
        }

        update_control_mode_status = false;
    }

    if(update_postion_status){
        if(cartesian){
            QVector<double> robot_position = motoman_communication.motomanUpdatePosition();
            ui->j1_v->setText(QString::number(robot_position[0]));
            ui->j2_v->setText(QString::number(robot_position[1]));
            ui->j3_v->setText(QString::number(robot_position[2]));
            ui->j4_v->setText(QString::number(robot_position[3]));
            ui->j5_v->setText(QString::number(robot_position[4]));
            ui->j6_v->setText(QString::number(robot_position[5]));
        }
        else if(pulse){
            QVector<double> robot_pulse = motoman_communication.motomanUpdatePulse();
            ui->j1_v->setText(QString::number(robot_pulse[0]));
            ui->j2_v->setText(QString::number(robot_pulse[1]));
            ui->j3_v->setText(QString::number(robot_pulse[2]));
            ui->j4_v->setText(QString::number(robot_pulse[3]));
            ui->j5_v->setText(QString::number(robot_pulse[4]));
            ui->j6_v->setText(QString::number(robot_pulse[5]));
        }
        else if(joint){
            QVector<double> robot_joint = motoman_communication.motomanUpdateDegree();
            ui->j1_v->setText(QString::number(robot_joint[0]));
            ui->j2_v->setText(QString::number(robot_joint[1]));
            ui->j3_v->setText(QString::number(robot_joint[2]));
            ui->j4_v->setText(QString::number(robot_joint[3]));
            ui->j5_v->setText(QString::number(robot_joint[4]));
            ui->j6_v->setText(QString::number(robot_joint[5]));
        }
        update_postion_status = false;
    }

    if(update_robot_status){
        quint8 previous_status = robot_status;
        robot_status = motoman_communication.motomanUpdateStatus();
//        qDebug() << robot_status;

        QFile status_file("C:\\Autoss\\Draft\\Robot_info\\status.txt");
        if(!status_file.open(QIODevice::WriteOnly)){
            QMessageBox::warning(this, "Write robot status errors", "Failed to open log text.");
        }
        QTextStream str(&status_file);
        if(robot_status){
            ui->status->setText("Running");
            ui->status->setPalette(yellow_base);
            str << "running";
        } else{
            ui->status->setText("Ready");
            ui->status->setPalette(green_base);
            str << "ready";
            if(previous_status != 0){
                program_running_status = false;
                list_busy_status = true;
                if(timer5->isActive()){
                    timer5->stop();
                    timer5->start(20000);
                } else timer5->start(20000);

                if(timer6->isActive()){
//                    qDebug() << "timmer66666666666666666666666";
                    timer6->stop();
                }
            }
        }
        status_file.flush();
        status_file.close();
        update_robot_status = false;
    }

    if(servo_on_status){
//        qDebug() << servo_status;
//        if(servo_status == 1){
//            if(servo_signal == 1) /*return*/;
//            else{
//                servo_status = 0;
//                ui->evet_log->appendPlainText("Servo's off");
//                ui->evet_log->appendPlainText("");
//            }
//        } else {
//            if(servo_signal == 0) /*return*/;
//            else{
//                qDebug() << servo_status;
//                qDebug() << servo_signal;
//                servo_status = 1;
//                ui->evet_log->appendPlainText("Servo's on");
//                ui->evet_log->appendPlainText("");
//            }
//        }
        QFile servo_file("C:\\Autoss\\Draft\\Robot_info\\servo.txt");
        if(!servo_file.open(QIODevice::WriteOnly)){
            QMessageBox::warning(this, "Write robot status errors", "Failed to open log text.");
        }
        QTextStream str(&servo_file);
        if(general_response){
            servo_status = true;
            ui->evet_log->appendPlainText("Servo's turned on");
            ui->evet_log->appendPlainText("");
            str << "on";
        } else{
            ui->evet_log->appendPlainText("Failed to turn on servo");
            ui->evet_log->appendPlainText("");
            str << "off";
        }
        servo_file.flush();
        servo_file.close();
        servo_on_status = false;
    }

    if(servo_off_status){
        QFile servo_file("C:\\Autoss\\Draft\\Robot_info\\servo.txt");
        if(!servo_file.open(QIODevice::WriteOnly)){
            QMessageBox::warning(this, "Write robot status errors", "Failed to open log text.");
        }
        QTextStream str(&servo_file);
        if(general_response){
            ui->evet_log->appendPlainText("Servo's turned off");
            ui->evet_log->appendPlainText("");
            servo_status = false;
            str << "off";
        } else{
            ui->evet_log->appendPlainText("Failed to turn off servo");
            ui->evet_log->appendPlainText("");
            str << "on";
        }
        servo_file.flush();
        servo_file.close();
        servo_off_status = false;
    }

//    qDebug() << "list status" << update_list_status;
    if(update_list_status){
        if(job_list_received.size() > 0){
//            qDebug() << "list calleddddddddddd";
            ui->job_list->clear();
//            qDebug() << job_list_received.size();
            for(int i = 0; i < job_list_received.size(); i++){
                ui->job_list->addItem(job_list_received.at(i));
            }
            update_list_status = false;
            motoman_communication.motomanFileControlDisconnect();
//            qDebug() << "next";
            if(timer4->isActive()){
                timer4->stop();
                timer4->start(20000);
            } else timer4->start(20000);
//            qDebug() << timer4->isActive();
            delete_busy_status = true;
//            qDebug() << delete_busy_status;
        }
    }
}

void MainWindow::ConnectionSetup(QString mode){
    if(mode == "on"){
        ui->evet_log->appendPlainText("Cartesian position type mode is on");
        ui->evet_log->appendPlainText("");

        ui->evet_log->appendPlainText("Manually get position mode is on");
        ui->evet_log->appendPlainText("");

        ui->evet_log->appendPlainText("Single program mode is on");
        ui->evet_log->appendPlainText("");

        ui->evet_log->appendPlainText("Follow pointer in controller mode is on");
        ui->evet_log->appendPlainText("");

        ui->status->setText("Ready");
        ui->status->setPalette(green_base);

        cartesian = true;
        pulse = false;
        joint = false;

        controller = ui->controller->currentText();
        brand = ui->brand->currentText();
        ui->evet_log->appendPlainText("Robot brand: " + brand);
        ui->evet_log->appendPlainText("");
        ui->evet_log->appendPlainText("Controller: " + controller);
        ui->evet_log->appendPlainText("");

        transfer_mode = "manually";

        running_mode = "pointer in controller";

        previous_mode = "none";
    } else if(mode == "off"){
//        qDebug() << servo_status;
        if(servo_status){
            servo_off_status = true;
            motoman_communication.motomanOffServo();
            ui->evet_log->appendPlainText("Servo's turned off");
            ui->evet_log->appendPlainText("");
        }

        ui->status->clear();
        ui->status->setPalette(ui->status->style()->standardPalette());

        ui->j1_v->clear();
        ui->j2_v->clear();
        ui->j3_v->clear();
        ui->j4_v->clear();
        ui->j5_v->clear();
        ui->j6_v->clear();

        ui->j1->setText("X");
        ui->j2->setText("Y");
        ui->j3->setText("Z");
        ui->j4->setText("Rx");
        ui->j5->setText("Ry");
        ui->j6->setText("Rz");

        ui->update_mode->setCurrentIndex(0);
        ui->position_type->setCurrentIndex(0);

        connection_status = false;
        fail_to_connect_status = false;
        update_control_mode_status = false;
        robot_status = 0;
        control_mode_str = "none";
        servo_status = false;
        update_postion_status = false;
        update_robot_status = false;
        servo_on_status = false;
        servo_off_status = false;
        update_delete_status = false;
        update_list_status = false;
        delete_busy_status = false;
        delete_busy_status = false;
        select_job_status = false;
        start_job_status = false;
        program_running_status = false;
        system_del = false;
        sys_del_activate = false;
        cont_ping = false;
        cur_ip = "";

        ui->brand->setCurrentIndex(0);
        ui->controller->setCurrentIndex(0);

        ui->job_list->clear();

        ui->program_backup->clear();

        ui->transferring_mode->setCurrentIndex(0);
        ui->running_mode->setCurrentIndex(0);

        ping_status = false;
        ping_checkable = false;

    }
}

void MainWindow::pingCallback(bool ready_2_connect){
//    qDebug() << "received ping statusssssssssss";
//    qDebug() << "ping checkableeeeeeeee:" << ping_checkable;
//    qDebug() << "ping statussss:" << ping_status;
    if(!ping_checkable) {
        ping_status = ready_2_connect;
        ping_checkable = true;
        return;
    } else{
        while(1){
            if(ready_2_connect){
//                qDebug() << "ping status continue to be trueeeee";
                delay->delay(2000);
                dialog1->setIpAdress(cur_ip);
                dialog1->run();
                break;
            } else{
                ping_status = false;
                ping_checkable = false;
//                qDebug() << "ping status is changeddddddddddd";

                ConnectionSetup("on");
                timer->stop();
                timer2->stop();
                timer3->stop();
                timer4->stop();
                timer5->stop();
                timer6->stop();

                motoman_communication.motomanDisconnect();

                connection_status = false;
                fail_to_connect_status = true;
                ui->status->setText("Disconnected");
                ui->status->setPalette(red_base);

                ui->evet_log->appendPlainText("Lost connection to robot IP " + ui->robot_ip->text());
                ui->evet_log->appendPlainText("");
                QMessageBox::warning(this, "Connection error", "Please check your Ethernet cable, IP domain or network transmission");

                return;
            }
            delay->delay(250);
        }
    }
}

void MainWindow::on_Connect_clicked()
{
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to perform connection to robot");
        return;
    }

    // In case, there's already a connection performance, confirm the user about that
    if(connection_status){
        ui->evet_log->appendPlainText("Already connected to robot IP " + cur_ip);
        ui->evet_log->appendPlainText("");
        return;
    }

    dialog1->setIpAdress(ui->robot_ip->text());
    dialog1->show();
    delay->delay(200);
    dialog1->run();

    while(1){
        if(ping_checkable){
            if(ping_status){
                dialog1->close();
                cont_ping = true;
                break;
            } else{
                dialog1->close();
                connection_status = false;
                fail_to_connect_status = true;
                ui->evet_log->appendPlainText("Failed to connect to robot IP " + ui->robot_ip->text());
                ui->evet_log->appendPlainText("");
                ui->status->setText("Disconnected");
                ui->status->setPalette(red_base);
                ping_checkable = false;
                return;
            }
        }
        delay->delay(250);
    }

//    qDebug() << "Get to connecttttttttt";
    QHostAddress udp_address;
    quint16 udp_port = ui->robot_port->text().toUShort();
    quint16 file_port = ui->fie_port->text().toUShort();
    udp_address.setAddress(QHostAddress(ui->robot_ip->text()).toIPv4Address());
    motoman_communication.motomanSetConnection(udp_address, udp_port, file_port);
    connection_status = motoman_communication.motomanConnect();
//    connection_status = connection_status & motoman_communication.motomanFIleCOntrolConnect();
//    qDebug() << "step 1111";
    if(!connection_status){
        connection_status = false;
        fail_to_connect_status = true;
        ui->evet_log->appendPlainText("Failed to connect to robot IP " + ui->robot_ip->text());
        ui->evet_log->appendPlainText("");
        motoman_communication.motomanDisconnect();
        ui->status->setText("Disconnected");
        ui->status->setPalette(red_base);
        return;
    }
    cur_ip = ui->robot_ip->text();
//    qDebug() << "step 2222";

    ui->evet_log->appendPlainText("Successfully connect to robot IP " + ui->robot_ip->text());
    ui->evet_log->appendPlainText("");
    ConnectionSetup("on");
    timer2->start(200);
    timer3->start(100);

    ip_address = udp_address.toString();    

    QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
    if(!ip_file.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this, "Write robot ip errors", "Failed to open log text.");
    }
    QTextStream str(&ip_file);
    str << ip_address;
    ip_file.flush();
    ip_file.close();

    delay->delay(1000);
    dialog1->setIpAdress(cur_ip);
    dialog1->run();
}

void MainWindow::on_disconnect_clicked()
{
    // Turn off the status "Disconnected"
    if(fail_to_connect_status){
        fail_to_connect_status = false;
        ui->status->clear();
        ui->status->setPalette(ui->status->style()->standardPalette());
        return;
    }

    // If there wasn't any connection performance, do nothing
    if(!connection_status){
        return;
    }

    dialog1->stop();
    delay->delay(100);

    ConnectionSetup("off");
    if(timer->isActive()) timer->stop();
    if(timer2->isActive()) timer2->stop();
    if(timer3->isActive()) timer3->stop();
    if(timer4->isActive()) timer4->stop();
    if(timer5->isActive()) timer5->stop();
    if(timer6->isActive()) timer6->stop();

    ping_checkable = false;

    delay->delay(200);
    motoman_communication.motomanDisconnect();
//    motoman_communication.motomanFileControlDisconnect();

    connection_status = false;
    ui->evet_log->appendPlainText("Disconnected");
    ui->evet_log->appendPlainText("");
    ui->evet_log->appendPlainText("");
    ui->evet_log->appendPlainText("");
    ui->evet_log->appendPlainText("");
}

void MainWindow::on_brand_currentIndexChanged(int index)
{
    if(index == 0){
        ui->controller->setItemText(0, "DX100");
        ui->controller->setItemText(1, "DX200");
        ui->controller->setItemText(2, "YRC1000micro");
        controller = controller_list[0];
        brand = brand_list[0];
        ui->Connect->setDisabled(false);
        ui->disconnect->setDisabled(false);
    } else if(index == 1){
        ui->controller->setItemText(0, "R-J3ib");
        ui->controller->setItemText(1, "R-30iA");
        ui->controller->setItemText(2, "R-30iB");
        controller = controller_list[3];
        brand = brand_list[1];
        ui->Connect->setDisabled(true);
        ui->disconnect->setDisabled(true);
    } else if(index == 2){
        ui->controller->setItemText(0, "KRC2");
        ui->controller->setItemText(1, "KRC3");
        ui->controller->setItemText(2, "KRC4");
        controller = controller_list[6];
        brand = brand_list[2];
        ui->Connect->setDisabled(true);
        ui->disconnect->setDisabled(true);
    }
    ui->evet_log->appendPlainText("Robot brand: " + brand);
    ui->evet_log->appendPlainText("");
    ui->controller->setCurrentIndex(0);
}

void MainWindow::on_controller_currentIndexChanged(int index)
{
    if(index == 0){
        if(brand == "Motoman") controller = controller_list[0];
        if(brand == "Fanuc") controller = controller_list[3];
        if(brand == "Kuka") controller = controller_list[6];
    } else if(index == 1){
        if(brand == "Motoman") controller = controller_list[1];
        if(brand == "Fanuc") controller = controller_list[4];
        if(brand == "Kuka") controller = controller_list[7];
    } else if(index == 2){
        if(brand == "Motoman") controller = controller_list[2];
        if(brand == "Fanuc") controller = controller_list[5];
        if(brand == "Kuka") controller = controller_list[8];
    }
    ui->evet_log->appendPlainText("Controller: " + controller);
    ui->evet_log->appendPlainText("");
}

void MainWindow::on_servo_on_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "Servo command", "Please make connection to robot to turn on servo");
        return;
    }
    if(control_mode_str == "teach"){
        QMessageBox::warning(this, "Servo command error", "Teach mode is on");
    }
    servo_on_status = true;
    motoman_communication.motomanOnServo();
    return;
}


void MainWindow::on_servo_off_clicked()
{
    if(!connection_status) return;
    servo_off_status = true;
    motoman_communication.motomanOffServo();
//    timer2->stop();
//    qDebug() << timer2->isActive();
//    while(timer2->isActive()){};
//    qDebug() << "timer off";
    return;
}

void MainWindow::timer2Callback(){
//    qDebug() << "call";
    update_control_mode_status = true;
    motoman_communication.motomanReadControlMode();
}

void MainWindow::on_position_type_currentIndexChanged(int index)
{
    if(index == 0){
        cartesian = true;
        pulse = false;
        joint = false;
        ui->j1->setText("X");
        ui->j2->setText("Y");
        ui->j3->setText("Z");
        ui->j4->setText("Rx");
        ui->j5->setText("Ry");
        ui->j6->setText("Rz");
        ui->evet_log->appendPlainText("Cartesian position type mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 1){
        pulse = true;
        cartesian = false;
        joint = false;
        ui->j1->setText("S");
        ui->j2->setText("L");
        ui->j3->setText("U");
        ui->j4->setText("R");
        ui->j5->setText("B");
        ui->j6->setText("T");
        ui->evet_log->appendPlainText("Pulse position type mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 2){
        joint = true;
        cartesian = false;
        pulse = false;
        ui->j1->setText("S");
        ui->j2->setText("L");
        ui->j3->setText("U");
        ui->j4->setText("R");
        ui->j5->setText("B");
        ui->j6->setText("T");
        ui->evet_log->appendPlainText("Joint position type mode is on");
        ui->evet_log->appendPlainText("");
    }
    return;
}

void MainWindow::on_Get_Position_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "Get position command", "Please make connection to robot to get robot's position");
        return;
    }
//    qDebug() << auto_get_pos;
//    qDebug() << cartesian;
    update_postion_status = true;
    if(auto_get_pos){
        timer->start(190);
    } else {
        if(cartesian){
            motoman_communication.motomanReadPosition();
        } else motoman_communication.motomanReadPulse();
    }
}

void MainWindow::timerCallback(){
    update_postion_status = true;
    if(cartesian) motoman_communication.motomanReadPosition();
    else motoman_communication.motomanReadPulse();
}

void MainWindow::on_update_mode_currentIndexChanged(int index)
{
    if(index == 0){
        auto_get_pos = false;
        if(timer->isActive()){
            timer->stop();
        }
        ui->evet_log->appendPlainText("Automatically update position mode is off");
        ui->evet_log->appendPlainText("");
    } else if(index == 1){
        auto_get_pos = true;
//        qDebug() << auto_get_pos;
        ui->evet_log->appendPlainText("Automatically update position mode is on");
        ui->evet_log->appendPlainText("");
    }
    return;
}

void MainWindow::timer3Callback(){
    update_robot_status = true;
    motoman_communication.motomanReadStatus();
//    qDebug() << "timer3";
}


void MainWindow::on_save_job_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "Save job command", "Please make connection to robot to save job");
        return;
    }
    transfer = "download";
    download_progress = new QProgressDialog("Downloading program", "Close", 0, 100);
    download_progress->setValue(QRandomGenerator::global()->bounded(10,20));
    download_progress->setWindowTitle("BKT");
    download_progress->setMaximumSize(250,92);
    download_progress->setMinimumSize(250,92);
    download_progress->setVisible(true);
    ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, ui->job_list->currentText(), "download");
//    qDebug() << 1;
//    motoman_communication.motoman_controller = controller.split("1").at(0);
//    motoman_communication.motomanSaveFile(ui->job_name->text());
//    ftp.ftpserverConnect(ui->robot_ip->text(), "rcmaster", "99999999", 21);
//    ftp.ftpserverSave(dir, ui->job_name->text());
//    ftp.start(QThread::HighestPriority);
}

void MainWindow::on_list_job_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "List job command", "Please make connection to robot to list job");
        return;
    }
    if(program_running_status){
        QMessageBox::warning(this, "System busy", "Can't use list commnad while program's running");
        return;
    }
    if(list_busy_status){
        QMessageBox::warning(this, "System busy", QString::number(timer5->remainingTime()/1000) + " more secs to use list command");
        return;
    }
    motoman_communication.motomanFIleCOntrolConnect();
    motoman_communication.list.clear();
    update_list_status = true;
    motoman_communication.motomanListFile("*.JBI");
}

void MainWindow::on_delete_job_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "Delete job command", "Please make connection to robot to delete job");
        return;
    }
    if(delete_busy_status){
        QMessageBox::warning(this, "System busy", QString::number(timer4->remainingTime()/1000) + " more secs to use delete command");
        return;
    }
    motoman_communication.motomanFIleCOntrolConnect();
    update_delete_status = true;
    motoman_communication.motomanDeleteFile(ui->job_list->currentText());
}

void MainWindow::timer4Callback(){
    delete_busy_status = false;
}

void MainWindow::transferCallback(bool finish_status){
    if(transfer == "download"){
        download_progress->setValue(100);
    }

    if(finish_status){
        ui->evet_log->appendPlainText("Transfered successfully");
        ui->evet_log->appendPlainText("");
    } else{
        ui->evet_log->appendPlainText("Failed to transfer");
        ui->evet_log->appendPlainText("");
    }

    if(!end_process && (transfer_mode == "automatically transfer" || transfer_mode == "automatically transfer and run" || transfer_mode == "transfer and resume")){
        next_upload = true;
    }
}

void MainWindow::bytesCallback(int bytes_sent){
    upload_progress->setValue(bytes_sent*100/total_bytes);
}

void MainWindow::on_start_job_clicked()
{
 //   qDebug() << servo_status;
    if(!servo_status){
        QMessageBox::information(this, "Start job command", "Please turn servo on to start job");
        ui->evet_log->appendPlainText("Servo's not turned on yet");
        ui->evet_log->appendPlainText("");
        return;
    }
    select_job_status = true;
    QString name = ui->job_list->currentText();
    motoman_communication.motomanSelectJob(name.split(".JBI").at(0), 1);
}

void MainWindow::startJobCallback(){
//    qDebug() << "called";
    start_job_status = true;
    program_running_status = true;
    motoman_communication.motomanStartJob();
}

void MainWindow::timer5Callback(){
    list_busy_status = false;
}

void MainWindow::on_path_browse_clicked()
{
    dir = QFileDialog::getExistingDirectory(this, tr("Path"));
    if(dir.size() == 0){
        return;
    }
    ui->path->setText(dir);
    ui->evet_log->appendPlainText("Path: " + dir);
    ui->evet_log->appendPlainText("");
    return;
}

void MainWindow::on_path_textChanged(const QString &arg1)
{
    ui->path->setToolTip(ui->path->text());
    return;
}


void MainWindow::on_select_single_program_clicked()
{
    if(main_program_flag == 1){
        ui->evet_log->appendPlainText("Main-sub program mode is on");
        ui->evet_log->appendPlainText("");
        return;
    }
    program_dir = QFileDialog::getOpenFileName(this, tr("Job list"), dir, "JBI files (*.JBI);; All files (*.*)");
    if(program_dir.size() == 0){
        return;
    }
    QStringList temp = program_dir.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    PC_program = temp.last();
    ui->program_selected->setText(PC_program);
    ui->no_program->setText("1");
    ui->evet_log->appendPlainText("Program: " + PC_program);
    ui->evet_log->appendPlainText("");
    return;
}


void MainWindow::on_program_selected_textChanged(const QString &arg1)
{
    ui->program_selected->setToolTip(ui->program_selected->text());
    return;
}

//void MainWindow::ConvertCsvFileToList(){
//    QFile jbi_file(program_dir);

//    // Check csv file whether it exists or not
//    if(!jbi_file.exists()){
//        ui->evet_log->appendPlainText("Failed to find CSV file");
//        jbi_file.close();
//        return;
//    }

//    jbi_file.open(QIODevice::ReadOnly | QIODevice::Text);
//    QTextStream fin(&jbi_file);
//    QString line = fin.readLine();
//    qDebug() << line;
//    jbi_file.close();
//    return;
//}

bool MainWindow::GenerateJobList(){
//    QDir source_dir(dir);
//    if(!source_dir.exists()){
//        ui->evet_log->appendPlainText("Error while detecting file directory");
//        ui->evet_log->appendPlainText("");
//        return;
//    }
//    QStringList const files = source_dir.entryList(QStringList() << "*.JBI", QDir::Files);
//    qDebug() << files;

    QFile jbi_file(main_program_dir);

    if(!jbi_file.exists()){
        ui->evet_log->appendPlainText("Failed to find JBI file");
        ui->evet_log->appendPlainText("");
        jbi_file.close();
        return false;
    }

    jbi_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream fin(&jbi_file);
    list.clear();
    list.append(PC_main_program);

    line_list.clear();

    max_line_list.clear();
    max_line_list.append(0);

    int line_count;
    bool NOP_check = false;
    bool CALL_check = false;
    while(!fin.atEnd()){
//        if(fin.readLine() == "NOP"){
//            line_count = 0;
//            QString temp = fin.readLine();
//            if(!temp.contains("CALL")){
//                ui->evet_log->appendPlainText("This program does not contain any sub-program");
//                ui->evet_log->appendPlainText("");
//                list.clear();
//                return false;
//            }
//            line_count = line_count + 1;
//            line_list.append(line_count);

//            list.append(temp.split(QLatin1Char(':')).at(1) + ".JBI");
//            break;
//        }
        if(!NOP_check){
            if(fin.readLine() == "NOP"){
                NOP_check = true;
                line_count = 0;
            }
        }

        if(NOP_check){
            QString temp = fin.readLine();
            line_count = line_count + 1;
            if(temp.contains("CALL")){
                CALL_check = true;
                line_list.append(line_count);
                list.append(temp.split(QLatin1Char(':')).at(1) + ".JBI");
                break;
            }
        }
    }
    if(NOP_check && !CALL_check){
        ui->evet_log->appendPlainText("This program does not contain any sub-program");
        ui->evet_log->appendPlainText("");
        list.clear();
        line_list.clear();
        max_line_list.clear();
        return false;
    }
    for(int i = 0; fin.atEnd() != true; i++){
        QString temp = fin.readLine();
        if(temp == "END") break;

        line_count = line_count + 1;
        if(temp.contains("CALL")){
            line_list.append(line_count);

            list.append(temp.split(QLatin1Char(':')).at(1) + ".JBI");
        }
    }
    list_size = list.size();

//    for(int i = 0; i < list.size() - 1; i++){
//        qDebug() << line_list.at(i);
//    }

    jbi_file.close();

    // Find maximum line of each subprogram
    QString folder_dir = main_program_dir.split(PC_main_program).at(0);
    for(int i = 1; i < list.size(); i++){
        max_line_list.append(findMaxLine(folder_dir + list.at(i)));
    }
    qDebug() << "maximum line for each program:" << max_line_list;

    return true;
}

int MainWindow::findMaxLine(QString dir){
    QFile sub(dir);
    if(sub.open(QIODevice::ReadOnly)){
        while(!sub.atEnd()){
            QString temp = sub.readLine();
            if(temp.contains("NPOS")){
                QString line_part = temp.split(" ").at(1);
                QString no_robot_pos = line_part.split(",").at(0);
                sub.flush();
                sub.close();
                return no_robot_pos.toInt();
            }
            delay->delay(5);
        }
    }
}

void MainWindow::on_select_main_program_clicked()
{
    if(main_program_flag == 0){
        ui->evet_log->appendPlainText("Single program mode is on");
        ui->evet_log->appendPlainText("");
        return;
    }
    main_program_dir = QFileDialog::getOpenFileName(this, tr("Job list"), dir, "JBI files (*.JBI);; All files (*.*)");
    if(main_program_dir.size() == 0){
        return;
    }
    QStringList temp = main_program_dir.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    PC_main_program = temp.last();
    ui->program_selected->setText(PC_main_program);
    if(GenerateJobList()){
        ui->evet_log->appendPlainText("Main program: " + PC_main_program);
        ui->evet_log->appendPlainText("");
        ui->evet_log->appendPlainText("Program list:");
        for(unsigned int i = 0; i < list_size; i++){
            ui->evet_log->appendPlainText(list.at(i));
        }
        ui->evet_log->appendPlainText("");
        ui->no_program->setText(QString::number(list_size));
    }

    cur_program = PC_main_program;
    cur_tranferring = PC_main_program;
    ui->program_transfering->setText(cur_tranferring);

    ui->total_progess_bar->setValue(0);
    return;
}

void MainWindow::on_main_program_existence_currentIndexChanged(int index)
{
    if(index == 1){
        main_program_flag = 1;
        ui->program_selected->clear();
        ui->no_program->clear();
        ui->evet_log->appendPlainText("Main program's included");
        ui->evet_log->appendPlainText("");
        return;
    }

    if(index == 0){
        main_program_flag = 0;
        ui->program_selected->clear();
        ui->no_program->clear();
        ui->evet_log->appendPlainText("Main program's removed");
        ui->evet_log->appendPlainText("");
        return;
    }
}

//void MainWindow::on_test_clicked()
//{
//    motoman_communication.motomanReadJob(1, "name, line");
//}

void MainWindow::on_transferring_mode_currentIndexChanged(int index)
{
    if(index == 0){
        transfer_mode = "manually";
        ui->evet_log->appendPlainText("Manually transfer mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 1){
        transfer_mode = "automatically transfer";
        ui->evet_log->appendPlainText("Automatically transfer mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 2){
        transfer_mode = "automatically transfer and run";
        ui->evet_log->appendPlainText("Automatically transfer and run program mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 3){
        transfer_mode = "transfer and resume";
        ui->evet_log->appendPlainText("Transfer and resume program from pointer mode is on");
        ui->evet_log->appendPlainText("");
    }
}

void MainWindow::on_transfer_program_clicked()
{
    if(!connection_status){
        QMessageBox::information(this, "Transfer program command", "Please make connection to robot to transfer program");
        return;
    }
    if(transfer_mode == "manually"){
//        motoman_communication.motomanLoadFile(ui->program_selected->text());
//        ftp.ftpLoad(QDir::toNativeSeparators(program_dir), PC_program);
//        qDebug() << QDir::toNativeSeparators(program_dir);
//        ftp.ftpConnect("192.168.255.1", "rcmaster", "99999999", 21);
//        ftp.ftpLoad(program_dir, PC_program);
        transfer = "upload";
        upload_progress = new QProgressDialog(PC_program, "Close", 0, 100);
        upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
        upload_progress->setWindowTitle("Uploading program");
        upload_progress->setVisible(true);
        upload_progress->setMaximumSize(250,92);
        upload_progress->setMinimumSize(250,92);
        temp = new QFile(this);
        temp->setFileName(program_dir);
        total_bytes = temp->size();
        ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, program_dir, PC_program, "upload");
//        ftp.start(/*QThread::HighestPriority*/);
        qDebug() << "manually transfered";
    } else if(transfer_mode == "automatically transfer" || transfer_mode == "automatically transfer and run" || transfer_mode == "transfer and resume"){
        percentage = 0;

        if(transfer_mode == "automatically transfer and run"){
            if(!servo_status){
                ui->evet_log->appendPlainText("Servo's not turned on yet");
                ui->evet_log->appendPlainText("");
                return;
            }
        }

        // Initiallizing progress bar of whole process
        ui->total_progess_bar->setMaximum(list.size());
        ui->total_progess_bar->setMinimum(0);

        start_pressed = false;

        int numb_sub = list.size() - 1;
        if(numb_sub <= 4){
            ui->evet_log->appendPlainText("Number of subprograms is below 5");
            ui->evet_log->appendPlainText("");

            end_process = false;
            for(int i = 0; i < list.size(); i++){
                next_upload = false;

                cur_tranferring = list.at(i);
                ui->program_transfering->setText(cur_tranferring);
                QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;

                transfer = "upload";
                upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
                upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
                upload_progress->setWindowTitle("Uploading program");
                upload_progress->setVisible(true);
                upload_progress->setMaximumSize(250,92);
                upload_progress->setMinimumSize(250,92);
                temp = new QFile(this);
                temp->setFileName(dir);
                total_bytes = temp->size();

                if(i == list.size() - 1){
                    end_process = true;
                }

                ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");

                // Duration for each transfering
                while(!next_upload && !end_process){
                    delay->delay(100);
//                    qDebug() << "wait";
                }
                delay->delay(500);
            }

            if(transfer_mode == "automatically transfer and run"){
                select_job_status = true;
                motoman_communication.motomanSelectJob(PC_main_program.split(".JBI").at(0), 1);
                cur_program = list.at(0);

                // Call timer to backup last program running
                timer6->start(40);
            }
        } else{
            int count = list.size() / 4;
            int left = 0;
            if(list.size() % 4 != 0){
                left = list.size() - count*4;
                count += 1;
            }
//            qDebug() << count << left;

            for(int i = 0; i < count; i++){
                if(i == count - 1 && left > 0){
//                    qDebug() << "left";
                    end_process = false;
                    for(int i = 0; i < left; i++){
                        next_upload = false;

                        cur_tranferring = list.at(i + 4*(count - 1));
                        ui->program_transfering->setText(cur_tranferring);
                        QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;

                        transfer = "upload";
                        upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
                        upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
                        upload_progress->setWindowTitle("Uploading program");
                        upload_progress->setVisible(true);
                        upload_progress->setMaximumSize(250,92);
                        upload_progress->setMinimumSize(250,92);
                        temp = new QFile(this);
                        temp->setFileName(dir);
                        total_bytes = temp->size();

                        if(i == left - 1){
                            end_process = true;
                        }

                        ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");

                        // Duration for each transfering
                        while(!next_upload && !end_process){
                            delay->delay(100);
//                            qDebug() << "wait";
                        }


                        // Update progress bar of whole process
                        percentage = percentage + 1;
//                        qDebug() << "percemmmmmm: " << percentage;
                        ui->total_progess_bar->setValue(percentage);
                        delay->delay(500);
                    }

                    sys_del_activate = true;
                    while(1){
//                        qDebug() << "innnnnnnnnnn";
                        if(cur_program == list.at(4*(count - 1)) && sys_del_activate){
                            for(int j = 4*(count - 2); j < 4*(count - 1); j++){
//                                qDebug() << "innnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
                                motoman_communication.motomanFIleCOntrolConnect();
                                update_delete_status = true;
                                system_del = true;
                                sys_pro_del = list.at(j);

                                motoman_communication.motomanDeleteFile(sys_pro_del);
                                sys_del_activate = false;
                                delay->delay(1000);
                            }

                            break;
                        }
                        delay->delay(500);
                    }

                    continue;
                }

                end_process = false;
                for(int j = 0; j < 4; j++){
                    next_upload = false;

                    cur_tranferring = list.at(j + 4*i);
                    ui->program_transfering->setText(cur_tranferring);
//                    qDebug() << cur_tranferring;
                    QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;
//                    qDebug() << dir;

                    transfer = "upload";
                    upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
                    upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
                    upload_progress->setWindowTitle("Uploading program");
                    upload_progress->setVisible(true);
                    upload_progress->setMaximumSize(250,92);
                    upload_progress->setMinimumSize(250,92);
                    temp = new QFile(this);
                    temp->setFileName(dir);
                    total_bytes = temp->size();

                    if(j == 3){
                        end_process = true;
                    }

                    ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");
//                    qDebug() << "next step";
//                    qDebug() << next_upload << end_process;

                    // Duration for each transfering
                    while(!next_upload && !end_process){
                        delay->delay(100);
//                        qDebug() << "wait";
                    }

                    // Update progress bar of whole process
                    percentage = percentage + 1;
//                    qDebug() << "percemmmmmm: " << percentage;
                    ui->total_progess_bar->setValue(percentage);
                    delay->delay(500);
                }
//                qDebug() << "period";
                if(transfer_mode == "transfer and resume") return;

                // If "automatically run", we have to press "start program" to run
                if(i == 0){
                    if(transfer_mode == "automatically transfer and run"){
                        select_job_status = true;
                        motoman_communication.motomanSelectJob(PC_main_program.split(".JBI").at(0), 1);
                        cur_program = list.at(0);
                        start_pressed = true;
                        timer6->start(40);
                    }
                }

//                qDebug() << "while";
                sys_del_activate = true;
                while(1){
                    if(i > 0 && cur_program == list.at(i*4) && sys_del_activate){
                        qDebug() << "delete";
                        for(int j = 0; j < 4; j++){
                            motoman_communication.motomanFIleCOntrolConnect();
                            update_delete_status = true;
                            system_del = true;
                            sys_pro_del = list.at((i - 1)*4 + j);

                            // Skip deleting main program
                            if(sys_pro_del == PC_main_program) continue;

                            motoman_communication.motomanDeleteFile(sys_pro_del);
                            sys_del_activate = false;
                            delay->delay(1000);
                        }
                    }

                    // Wait until 2 programs finished running to move to next transferring
//                    qDebug() << list.at(1 + i*4);
//                    qDebug() << cur_program;
//                    qDebug() << start_pressed;
                    if(start_pressed && (cur_program == list.at(1 + i*4))  && (cur_line > (max_line_list.at(1 + i*4) - 10))){
                        break;
                    }
                    delay->delay(100);
//                    qDebug() << "wait";
                }
            }
        }
    }
}

void MainWindow::on_start_program_clicked(){
    if(!servo_status){
        QMessageBox::information(this, "Start program command", "Please turn servo on to start program");
        ui->evet_log->appendPlainText("Servo's not turned on yet");
        ui->evet_log->appendPlainText("");
        return;
    }
    select_job_status = true;
    motoman_communication.motomanSelectJob(PC_main_program.split(".JBI").at(0), 1);
    cur_program = list.at(0);
    start_pressed = true;
    timer6->start(50);
}

void MainWindow::timer6Callback(){
    motoman_communication.motomanReadJob(1, "name, line");
    read_job_status = true;
}

void MainWindow::on_program_transfering_textChanged(const QString &arg1)
{
    ui->program_transfering->setToolTip(cur_tranferring);

}


void MainWindow::on_restore_clicked(){
    if(!connection_status){
        QMessageBox::information(this, "Show previous program command", "Please make connection to robot to show log");
        return;
    }
    QFile f("C://Autoss//Draft//Job_info//info.txt");
    if(!f.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "Restore error", "Fail to get the backup data");
        return;
    }
    QString temp, pro, line;
    temp = f.readLine();
    pro = temp.split(" ").at(0);
    line = temp.split(" ").at(1);
//    qDebug() << "line " + line;
    ui->program_backup->clear();
    ui->program_backup->addItem(pro);
    ui->program_backup->addItem(pro + ", line " + line);
}



void MainWindow::on_running_mode_currentIndexChanged(int index){
    if(index == 0){
        running_mode = "pointer in controller";
        ui->evet_log->appendPlainText("Follow pointer in controller mode is on");
        ui->evet_log->appendPlainText("");
    } else if(index == 1){
        running_mode = "pointer in system";
        ui->evet_log->appendPlainText("Follow pointer in previous program mode is on");
        ui->evet_log->appendPlainText("");
    }
}



void MainWindow::on_resume_running_clicked(){
    if(!connection_status){
        QMessageBox::information(this, "Resume process command", "Please make connection to robot to resume the process");
        return;
    }
    if(transfer_mode == "manually" || transfer_mode == "automatically transfer" || transfer_mode == "automatically transfer and run"){
        QMessageBox::warning(this, "Resume error", "Not approriate transfer mode");
        return;
    }

    if(running_mode == "pointer in controller"){
        motoman_communication.motomanReadJob(1, "name, line");
        read_job_status = true;
        while(read_job_status){
            delay->delay(500);
        }
        qDebug() << "takeout : " << cur_program << cur_line;
    } else if(running_mode == "pointer in system"){
        QFile f("C://Autoss//Draft//Job_info//info.txt");
        f.open(QIODevice::ReadOnly);
        QString temp, pro, line;
        temp = f.readLine();
        pro = temp.split(" ").at(0);
        line = temp.split(" ").at(1);
        cur_program = pro;
        if(previous_mode == "without line"){
            cur_line = 1;
        } else if(previous_mode == "with line"){
            cur_line = line.toInt();
        } else if(previous_mode == "none"){
            QMessageBox::warning(this, "Resume error", "Cannot find the starting point");
        }
    }

    // Find the position of the selected subprogram and the next one to call in the list
    for(int i = 0; i < list.size(); i++){
//        qDebug() << list.at(i);
        if(list.at(i) == cur_program){
            line_index = i - 1;
            program_index = i;
            goto next;
        }
        if(i == list.size() - 1){
            QMessageBox::warning(this, "Resume error", "The subprogram is not included in the main program");
            return;
        }
    }

    next:
    qDebug() << "takein : " << line_index << program_index;
//    qDebug() << line_list;

    // line in the main program to call the next subprogram after the current one
    int line_call = line_list.at(line_index + 1);
    qDebug() << "line call: " << line_call;

    percentage = 0;
    ui->total_progess_bar->setValue(0);

    if(transfer_mode == "automatically transfer and run"){
        if(!servo_status){
            QMessageBox::warning(this, "Resume error", "Servo's not turned on yet");
            return;
        }
    }

    int size = list.size() - program_index;
//    qDebug() << "size: " << size;
    // Initiallizing progress bar of whole process
    ui->total_progess_bar->setMaximum(size);
    ui->total_progess_bar->setMinimum(0);

    start_pressed = false;

    // Get job list for  job scanning
    motoman_communication.motomanFIleCOntrolConnect();
    motoman_communication.list.clear();
    update_list_status = true;
    motoman_communication.motomanListFile("*.JBI");
    while(update_list_status){
        delay->delay(20);
    }

    int numb_sub = size - 1;
    if(numb_sub <= 4){
        ui->evet_log->appendPlainText("Number of subprograms left is below 5");
        ui->evet_log->appendPlainText("");

        end_process = false;
        for(int i = program_index; i < list.size(); i++){
            next_upload = false;

            cur_tranferring = list.at(i);
            ui->program_transfering->setText(cur_tranferring);
            QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;

            transfer = "upload";
            upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
            upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
            upload_progress->setWindowTitle("Uploading program");
            upload_progress->setVisible(true);
            upload_progress->setMaximumSize(250,92);
            upload_progress->setMinimumSize(250,92);
            temp = new QFile(this);
            temp->setFileName(dir);
            total_bytes = temp->size();

            if(i == list.size() - 1){
                end_process = true;
            }

            // Skip transfering the existed subprogram
            if(ui->job_list->findText(cur_tranferring) != -1){
                upload_progress->setValue(100);
                delay->delay(500);
                // Update progress bar of whole process
                percentage = percentage + 1;
//                    qDebug() << "percemmmmmm: " << percentage;
                ui->total_progess_bar->setValue(percentage);
                continue;
            }

            ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");

            // Duration for each transfering
            while(!next_upload && !end_process){
                delay->delay(100);
//                qDebug() << "wait";
            }
            delay->delay(500);
        }

        select_job_status = true;
        motoman_communication.motomanSelectJob(cur_program.split(".JBI").at(0), cur_line);

        // Call timer to backup last program running
        timer6->start(50);

        // Wait to swap from subprogram to next one called in the main one
        while(1){
            if(cur_program == list.at(program_index) && cur_line > (max_line_list.at(program_index) - 1)){
                motoman_communication.motomanSelectJob(PC_main_program.split(".JBI").at(0), /*line_list.at(line_call)*/ line_call);
                timer6->start(50);
                break;
            }
        }
    } else{
        int count = size / 4;
        int left = 0;
        if(size % 4 != 0){
            left = size - count*4;
            count += 1;
        }
//        qDebug() << count << left;

        for(int i = 0; i < count; i++){
            if(i == count - 1 && left > 0){
//                qDebug() << "left";
                end_process = false;
                for(int i = 0; i < left; i++){
                    next_upload = false;

                    // Start from the chosen subprogram
                    cur_tranferring = list.at(i + 4*(count - 1) + program_index);
                    ui->program_transfering->setText(cur_tranferring);
                    QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;

                    transfer = "upload";
                    upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
                    upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
                    upload_progress->setWindowTitle("Uploading program");
                    upload_progress->setVisible(true);
                    upload_progress->setMaximumSize(250,92);
                    upload_progress->setMinimumSize(250,92);
                    temp = new QFile(this);
                    temp->setFileName(dir);
                    total_bytes = temp->size();

                    if(i == left - 1){
                        end_process = true;
                    }

                    ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");

                    // Duration for each transfering
                    while(!next_upload && !end_process){
                        delay->delay(100);
//                        qDebug() << "wait";
                    }


                    // Update progress bar of whole process
                    percentage = percentage + 1;
//                    qDebug() << "percemmmmmm: " << percentage;
                    ui->total_progess_bar->setValue(percentage);
                    delay->delay(500);
                }

                sys_del_activate = true;
                while(1){
//                        qDebug() << "innnnnnnnnnn";
                    if(cur_program == list.at(4*(count - 1) + program_index) && sys_del_activate){
                        for(int j = 4*(count - 2) + program_index; j < 4*(count - 1) + program_index; j++){
//                                qDebug() << "innnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
                            motoman_communication.motomanFIleCOntrolConnect();
                            update_delete_status = true;
                            system_del = true;
                            sys_pro_del = list.at(j);

                            motoman_communication.motomanDeleteFile(sys_pro_del);
                            sys_del_activate = false;
                            delay->delay(1000);
                        }
                        break;
                    }
                    delay->delay(500);
                }
                continue;
            }

            end_process = false;
            for(int j = 0; j < 4; j++){
                next_upload = false;

                cur_tranferring = list.at(j + 4*i + program_index);

                ui->program_transfering->setText(cur_tranferring);
//                qDebug() << cur_tranferring;
                QString dir = main_program_dir.split(PC_main_program).at(0) + cur_tranferring;
//                qDebug() << dir;

                transfer = "upload";
                upload_progress = new QProgressDialog(cur_tranferring, "Close", 0, 100);
                upload_progress->setValue(QRandomGenerator::global()->bounded(2,7));
                upload_progress->setWindowTitle("Uploading program");
                upload_progress->setVisible(true);
                upload_progress->setMaximumSize(250,92);
                upload_progress->setMinimumSize(250,92);
                temp = new QFile(this);
                temp->setFileName(dir);
                total_bytes = temp->size();

                if(j == 3){
                    end_process = true;
                }

                // Skip transfering the selected subprogram
                if(cur_tranferring == list.at(program_index)){
                    upload_progress->setValue(100);
                    delay->delay(500);
                    // Update progress bar of whole process
                    percentage = percentage + 1;
    //                    qDebug() << "percemmmmmm: " << percentage;
                    ui->total_progess_bar->setValue(percentage);
                    continue;
                }

                // Skip transfering the existed subprogram
                if(ui->job_list->findText(cur_tranferring) != -1){
                    upload_progress->setValue(100);
                    delay->delay(500);
                    // Update progress bar of whole process
                    percentage = percentage + 1;
    //                    qDebug() << "percemmmmmm: " << percentage;
                    ui->total_progess_bar->setValue(percentage);
                    continue;
                }

                ftp.transfer(ui->robot_ip->text(), "rcmaster", yaskawa_security_key, 21, dir, cur_tranferring, "upload");
//                qDebug() << "next step";
//                qDebug() << next_upload << end_process;

                // Duration for each transfering
                while(!next_upload && !end_process){
                    delay->delay(100);
                    qDebug() << "wait";
                }

                // Update progress bar of whole process
                percentage = percentage + 1;
//                    qDebug() << "percemmmmmm: " << percentage;
                ui->total_progess_bar->setValue(percentage);
                delay->delay(500);
            }
//                qDebug() << "period";

            // If "automatically run", we have to press "start program" to run
            if(i == 0){
                select_job_status = true;
                motoman_communication.motomanSelectJob(cur_program.split(".JBI").at(0), cur_line);
//                        cur_program = list.at(0);
                start_pressed = true;
                timer6->start(50);
            }

//            qDebug() << "while";
            sys_del_activate = true;
            while(1){
                // Wait to swap from subprogram to next one called in the main one
                if(i == 0 && cur_program == list.at(program_index) && cur_line > max_line_list.at(program_index)){
                    motoman_communication.motomanSelectJob(PC_main_program.split(".JBI").at(0), /*line_list.at(line_call)*/ line_call);
                    timer6->start(50);
                }

                if(i > 0 && cur_program == list.at(i*4 + program_index) && sys_del_activate){
//                    qDebug() << "delete";
                    for(int j = 0; j < 4; j++){
                        motoman_communication.motomanFIleCOntrolConnect();
                        update_delete_status = true;
                        system_del = true;
                        sys_pro_del = list.at((i - 1)*4 + j + program_index);

//                            // Skip deleting main program
//                            if(sys_pro_del == PC_main_program) continue;

                        motoman_communication.motomanDeleteFile(sys_pro_del);
                        sys_del_activate = false;
                        delay->delay(1000);
                    }
                    if(left == 0 && i == count - 1) break;
                }

                // Wait until 2 programs finished running to move to next transferring
//                qDebug() << "program index: " << program_index;
//                qDebug() << list.at(1 + i*4 + program_index);
//                qDebug() << cur_program;
//                qDebug() << start_pressed;
                if(start_pressed && (cur_program == list.at(1 + i*4 + program_index))  && (cur_line > (max_line_list.at(1 + i*4 + program_index) - 10))){
                    break;
                }
                delay->delay(100);
//                qDebug() << "wait";
            }
        }
    }
}


void MainWindow::on_program_backup_currentIndexChanged(int index)
{
    if(index == 0){
        previous_mode = "without line";
    } else if(index == 1){
        previous_mode = "with line";
    }
}

void MainWindow::on_license_triggered(){
    dialog->exec();
}

void MainWindow::autossKeyManagementCallback(bool confirmation){
    Autoss_key_managment = confirmation;
//    qDebug() << "AUTOSSSSSSS: " << Autoss_key_managment;
}

void MainWindow::licenseActionTriggered(bool){
    dialog3->exec();
}

void MainWindow::daysleftCallback(int days, QString key, bool checkable){
//    qDebug() << "days calllll" << lock_status;
    if(lock_status){
        dialog->lockCount();
        return;
    }

    if(cheating_status){
        dialog->changeCount(0);
        dialog->lockCount();
        return;
    }
    dialog->unlockCount();

    if(!checkable){
        license_checkable = false;
        this->setWindowTitle("BKT - Robot Control Panel - Deactivated");
        license_activate = false;
        if(ui->menuHelp->actions().contains(license_action)){
            ui->menuHelp->removeAction(license_action);
        }
        return;
    }
    license_checkable = true;

    QDir dir;
    dir.setPath("C:\\Autoss\\Draft\\License");
    if(!dir.exists()){
        qDebug() << dir.mkpath("C:\\Autoss\\Draft\\License");
        SetFileAttributesW(L"C:\\Autoss", FILE_ATTRIBUTE_HIDDEN);
        SetFileAttributesW(L"C:\\Autoss\\Draft\\License", FILE_ATTRIBUTE_HIDDEN);
    }

//    qDebug() << days;
    if(days <= 0){
        qDebug() << "in if";
        this->setWindowTitle("BKT - Robot Control Panel - License Expired (Renew license to use)");
        QFile license_file("C:\\Autoss\\Draft\\License\\Key.txt");
        if(license_file.open(QIODevice::WriteOnly)){
//            qDebug() << "in file";
            QTextStream str(&license_file);
            if(key != license_file.readLine()) str << key;
            license_file.flush();
            license_file.close();
        }
        return;
    }
    license_activate = true;
    qDebug() << license_activate;
    QFile license_file("C:\\Autoss\\Draft\\License\\Key.txt");
    if(license_file.open(QIODevice::WriteOnly)){
//        qDebug() << "in file";
        QTextStream str(&license_file);
        str << key;
        license_file.flush();
        license_file.close();
    }

    if(Autoss_key_managment){
        if(days > 30){
            this->setWindowTitle("BKT - Robot Control Panel - Management key");
        } else this->setWindowTitle("BKT - Robot Control Panel - " + QString::number(days) + " days left (Management key)");
    } else this->setWindowTitle("BKT - Robot Control Panel - " + QString::number(days) + " days left");
}

void MainWindow::timer7Callback(){
//    qDebug() << "timer7 called";
//    qDebug() << license_checkable;
    QFile lock_file("C:\\Autoss\\Draft\\License\\Lock.txt");
    // If the Lock.txt already exists
    if(!lock_file.isOpen()){
        if(lock_file.open(QIODevice::ReadOnly)){
            if(!lock_file.readLine().contains("lock")){
//                qDebug() << "lock file opennnnnnnnnn";
                lock_status = false;
                cheating_status = !license.checkTimerCheating();
//                qDebug() << "check date time cheating: " << !(license.checkTimerCheating());
            }
            lock_file.flush();
            lock_file.close();
        } else{
            cheating_status = !license.checkTimerCheating();
//            qDebug() << "check date time cheating: " << !(license.checkTimerCheating());
        }
    }

    if(lock_status || cheating_status) {
//        qDebug() << "cheating status: " << cheating_status;
//        qDebug() << "locking status: " << lock_status;
        if(lock_status) this->setWindowTitle("BKT - Robot Control Panel - Locked (Contact us for unlocking the software)");
        else if(cheating_status) this->setWindowTitle("BKT - Robot Control Panel - Deactivated");

        license_activate = false;
        dialog->changeCount(0);
        dialog->lockCount();
        return;
    }
    dialog->unlockCount();

    bool check_status = false;
    if(!license_checkable){
//        qDebug() << "uncheckable";
        this->setWindowTitle("BKT - Robot Control Panel - Deactivated");
        license_activate = false;
    } else {
        QDir dir("C:\\Autoss\\Draft\\License");
        if(dir.exists()){
//            qDebug() << "exist";
            QFile license_file("C:\\Autoss\\Draft\\License\\Key.txt");
            if(license_file.open(QIODevice::ReadWrite)){
//                qDebug() << "in file";
                QString temp = license_file.readLine();

//                qDebug() << temp.size();
                if(temp.size() < 19){
                    this->setWindowTitle("BKT - Robot Control Panel - Deactivated");
                    if(dialog->isActiveWindow()) dialog->changeCount(0);
                    license_activate = false;
                    return;
                }

                int days = license.licenseProcess(temp);
                if(days <= 0){
                    this->setWindowTitle("BKT - Robot Control Panel - License Expired (Renew license to use)");
                    if(dialog->isActiveWindow()) dialog->changeCount(0);
                    QTextStream str(&license_file);
                    str << "";
                    license_activate = false;
                    license_file.flush();
                    license_file.close();
                    return;
                }
                license_file.flush();
                license_file.close();
                license_activate = true;
                if(Autoss_key_managment){
                    if(days > 30){
                        this->setWindowTitle("BKT - Robot Control Panel - Management key");
                    } else this->setWindowTitle("BKT - Robot Control Panel - " + QString::number(days) + " days left (Management key)");

                    // Create action to menu-bar for generating license
                    if(!ui->menuHelp->actions().contains(license_action)){
                        ui->menuHelp->addAction(license_action);
                    }
                } else{
                    this->setWindowTitle("BKT - Robot Control Panel - " + QString::number(days) + " days left");
                    if(ui->menuHelp->actions().contains(license_action)){
                        ui->menuHelp->removeAction(license_action);
                    }
                }
                if(dialog->isActiveWindow()) dialog->changeCount(1);
                check_status = true;
            }
        } else{
            this->setWindowTitle("BKT - Robot Control Panel - Deactivated");
            license_activate = false;
            if(dialog->isActiveWindow()) dialog->changeCount(0);
        }
    }

    if(!check_status && connection_status){
        if(timer->isActive()) timer->stop();
        if(timer2->isActive()) timer2->stop();
        if(timer3->isActive()) timer3->stop();
        if(timer4->isActive()) timer4->stop();
        if(timer5->isActive()) timer5->stop();
        if(timer6->isActive()) timer6->stop();

        dialog1->stop();
        delay->delay(200);
        ping_checkable = false;

        ConnectionSetup("off");
        motoman_communication.motomanDisconnect();
        ui->evet_log->appendPlainText("Disconnected");
        ui->evet_log->appendPlainText("");
        connection_status = false;
    }
    return;
}

void MainWindow::preCheckDateTime(){
    // Start lock status
    QFile lock_file("C:\\Autoss\\Draft\\License\\Lock.txt");
    if(lock_file.open(QIODevice::ReadOnly)){
        if(lock_file.readLine() == "lock"){
            this->setWindowTitle("BKT - Robot Control Panel - Locked (Contact us for unlocking the software)");
            lock_status = true;
            dialog->lockCount();
            lock_file.flush();
            lock_file.close();
            return;
        }
    }

    // Retrieve current day and compare
    // Return true if there was a system date cheating behaviour

    cheating_status = !license.checkTimerCheating();
//    qDebug() << "check date time cheating: " << !(license.checkTimerCheating());
    if(cheating_status){
        dialog->lockCount();
        QFile file("C:\\Autoss\\Draft\\License\\Crack_warning.txt");
        if(file.open(QIODevice::ReadOnly)){
//            qDebug() << "Innnnnnnnnnnnn: ";
            cheating_count = file.readLine().toInt();
            license_activate = false;
            dialog->changeCount(0);
            this->setWindowTitle("BKT - Robot Control Panel - Deactivated");
            cheating_count = cheating_count + 1;
//            qDebug() << "cheating count: " << cheating_count;
            file.flush();
            file.close();
            file.setFileName("C:\\Autoss\\Draft\\License\\Crack_warning.txt");
//            file.remove();
            file.open(QIODevice::WriteOnly);
            QTextStream str(&file);
            str << QString::number(cheating_count);
            file.flush();
            file.close();
            if(cheating_count == 2){
                QMessageBox::warning(this, "System Interrupt", "One more time will permanently lock the software");
            }

            if(cheating_count == 3) {
//                qDebug() << "<<<<<<< 3333333333";
                QMessageBox::critical(this, "System Interrupt", "Your software has been locked. Please contact us for unlock key!");
                file.setFileName("C:\\Autoss\\Draft\\License\\Lock.txt");
                file.open(QIODevice::WriteOnly);
                QTextStream str(&file);
                str << "lock";
                file.flush();
                file.close();
                lock_status = true;
                cheating_status = false;
                this->setWindowTitle("BKT - Robot Control Panel - Locked (Contact us for unlocking the software)");
            }

            if(cheating_count < 2){
                QMessageBox::warning(this, "System Interrupt", "System date has been changed");
            }
        }
    }
}

void MainWindow::on_exit_triggered(){
    if(timer->isActive()){
        timer->stop();
        delay->delay(200);
    }
    if(timer2->isActive()){
        timer2->stop();
        delay->delay(200);
    }
    if(timer3->isActive()){
        timer3->stop();
        delay->delay(200);
    }
    if(timer4->isActive()){
        timer4->stop();
        delay->delay(200);
    }
    if(timer5->isActive()){
        timer5->stop();
        delay->delay(200);
    }
    if(timer6->isActive()){
        timer6->stop();
        delay->delay(200);
    }
    if(timer7->isActive()){
        timer7->stop();
        delay->delay(200);
    }

    delete timer;
    delete timer2;
    delete timer3;
    delete timer4;
    delete timer5;
    delete timer6;
    delete timer7;
    delete temp;
    delete dialog;
    delete dialog1;
    delete dialog2;
    delete dialog3;
    delete dialog4;
    delete delay;

    upload_progress->deleteLater();
    download_progress->deleteLater();

    delete ui;
}


void MainWindow::on_uninstall_triggered(){
    if(connection_status){
        QMessageBox::warning(this, "Unistall", "Please disconnect to proceed uninstalling");
        return;
    }

    // Stop system timer
    if(timer->isActive()){
        timer->stop();
        delay->delay(200);
    }
    if(timer2->isActive()){
        timer2->stop();
        delay->delay(200);
    }
    if(timer3->isActive()){
        timer3->stop();
        delay->delay(200);
    }
    if(timer4->isActive()){
        timer4->stop();
        delay->delay(200);
    }
    if(timer5->isActive()){
        timer5->stop();
        delay->delay(200);
    }
    if(timer6->isActive()){
        timer6->stop();
        delay->delay(200);
    }
    if(timer7->isActive()){
        timer7->stop();
        delay->delay(200);
    }

//    delete timer;
//    delete timer2;
//    delete timer3;
//    delete timer4;
//    delete timer5;
//    delete timer6;
//    delete timer7;
//    if(temp->exists()) delete temp;
//    if(dialog != NULL) dialog->deleteLater();
//    if(dialog1 != NULL) dialog1->deleteLater();
//    if(dialog2 != NULL) dialog2->deleteLater();
//    if(dialog != Q_NULLPTR) delete dialog;
//    if(dialog1 != Q_NULLPTR) delete dialog1;
//    if(dialog2 != Q_NULLPTR) delete dialog2;
//    delete delay;
//    qDebug() << "removed";

    QFile file("C:\\Autoss\\Draft\\Settings\\save.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream remove_str(&file);
        QString save_dir = remove_str.readAll();
        file.flush();
        file.close();

        QDir file_remove;
        file_remove.setPath("C:\\Autoss");
        if(file_remove.removeRecursively()){
            qDebug() << "uninstall recursively";

            if(save_dir == (QDir::homePath() + "/Program")){
                file_remove.setPath(save_dir);
                if(!file_remove.removeRecursively()){
                    QMessageBox::warning(this, "Uninstall error", "Fail to remove save file Program");
                    this->close();
                    return;
                }
            }
            ui->evet_log->appendPlainText("Uninstalled successfully");
            ui->evet_log->appendPlainText("");
            QMessageBox::information(this, "Uninstall", "All of the relevant files are removed");

            this->close();
        }
    }
}


void MainWindow::on_about_triggered(){
    dialog2->show();
}

void MainWindow::on_RoboDK_clicked(){
    dialog4->show();
}

void MainWindow::on_Options_triggered(){
    dialog5->exec();
}

void MainWindow::on_Plugins_triggered(){
    dialog6->exec();
}

void MainWindow::loadPluginCallback(bool loadable){
    int row;
    plugin_list = dialog6->getPluginList(&row);

    if(!loadable){
        switch (row) {
        case 0:
            ui->menuTools->removeAction(pluginAction);
            break;
        case 1:
            ui->menuTools->removeAction(pluginAction2);
            break;
        case 2:
            ui->menuTools->removeAction(pluginAction3);
            break;
        case 3:
            ui->menuTools->removeAction(pluginAction4);
            break;
        case 4:
            ui->menuTools->removeAction(pluginAction5);
            break;
        case 5:
            ui->menuTools->removeAction(pluginAction6);
            break;
        case 6:
            ui->menuTools->removeAction(pluginAction7);
            break;
        }
        return;
    }

    QString dir = application_homepath + "/plugins/" + plugin_list.at(row);
    QString key_value;


    switch (row) {
    case 0:
        plugin_loader.setFileName(dir);
        key_value = plugin_loader.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin = plugin_loader.instance();
        if(plugin){
            pluginInterface = qobject_cast<PluginInterface *>(plugin);
            if(!pluginInterface){
                plugin_loader.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction = new QAction(key_value, this);
        connect(pluginAction, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered(bool)));
        ui->menuTools->addAction(pluginAction);
        break;
    case 1:
        plugin_loader2.setFileName(dir);
        key_value = plugin_loader2.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin2 = plugin_loader2.instance();
        if(plugin2){
            pluginInterface2 = qobject_cast<PluginInterface *>(plugin2);
            if(!pluginInterface2){
                plugin_loader2.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction2 = new QAction(key_value, this);
        connect(pluginAction2, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered2(bool)));
        ui->menuTools->addAction(pluginAction2);
        break;
    case 2:
        plugin_loader3.setFileName(dir);
        key_value = plugin_loader3.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin3 = plugin_loader3.instance();
        if(plugin3){
            pluginInterface3 = qobject_cast<PluginInterface *>(plugin3);
            if(!pluginInterface3){
                plugin_loader3.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction3 = new QAction(key_value, this);
        connect(pluginAction3, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered3(bool)));
        ui->menuTools->addAction(pluginAction3);
        break;
    case 3:
        plugin_loader4.setFileName(dir);
        key_value = plugin_loader4.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin4 = plugin_loader4.instance();
        if(plugin4){
            pluginInterface4 = qobject_cast<PluginInterface *>(plugin4);
            if(!pluginInterface4){
                plugin_loader4.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction4 = new QAction(key_value, this);
        connect(pluginAction4, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered4(bool)));
        ui->menuTools->addAction(pluginAction4);
        break;
    case 4:
        plugin_loader5.setFileName(dir);
        key_value = plugin_loader5.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin5 = plugin_loader5.instance();
        if(plugin5){
            pluginInterface5 = qobject_cast<PluginInterface *>(plugin5);
            if(!pluginInterface5){
                plugin_loader5.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction5 = new QAction(key_value, this);
        connect(pluginAction5, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered5(bool)));
        ui->menuTools->addAction(pluginAction5);
        break;
    case 5:
        plugin_loader6.setFileName(dir);
        key_value = plugin_loader6.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin6 = plugin_loader6.instance();
        if(plugin6){
            pluginInterface6 = qobject_cast<PluginInterface *>(plugin6);
            if(!pluginInterface6){
                plugin_loader6.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction6 = new QAction(key_value, this);
        connect(pluginAction6, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered6(bool)));
        ui->menuTools->addAction(pluginAction6);
        break;
    case 6:
        plugin_loader7.setFileName(dir);
        key_value = plugin_loader7.metaData().find("MetaData")->toObject().value("Keys").toArray().at(0).toString();
        plugin7 = plugin_loader7.instance();
        if(plugin7){
            pluginInterface7 = qobject_cast<PluginInterface *>(plugin7);
            if(!pluginInterface7){
                plugin_loader7.unload();
                qDebug() << "Failed to load plugin";
                return;
            }
            qDebug() << "Cast successfully";
        } else return;
        pluginAction7 = new QAction(key_value, this);
        connect(pluginAction7, SIGNAL(triggered(bool)),this, SLOT(pluginActionTriggered7(bool)));
        ui->menuTools->addAction(pluginAction7);
        break;
    }

//    char plugin_key[key_value.size()];
//    for(int i = 0; i < key_value.size(); i++){
//        plugin_key[i] = key_value.at(i).toLatin1();
//    }
//    Core::Command *cmd = Core::ActionManager::registerAction(plugin_action, plugin_key, Core::Context(Core::Constants::C_GLOBAL));
//    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addAction(cmd, "Tools");
//    QActionGroup *a;

}

void MainWindow::pluginActionTriggered(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface->implement();
}

void MainWindow::pluginActionTriggered2(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface2->implement();
}

void MainWindow::pluginActionTriggered3(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface3->implement();
}

void MainWindow::pluginActionTriggered4(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface4->implement();
}

void MainWindow::pluginActionTriggered5(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface5->implement();
}

void MainWindow::pluginActionTriggered6(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface6->implement();
}

void MainWindow::pluginActionTriggered7(bool){
    if(!license_activate){
        QMessageBox::information(this, "Connection command", "Please activate license to run the plugin");
        return;
    }
    pluginInterface7->implement();
}
