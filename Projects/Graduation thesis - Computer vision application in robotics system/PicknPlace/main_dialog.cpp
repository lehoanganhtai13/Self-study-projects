#include "main_dialog.h"
#include "ui_main_dialog.h"
#include "example.hpp"

void mouse(int event, int x, int y, int flags, void* param);

main_dialog::main_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::main_dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Pick and place");
    this->setFixedWidth(1115);
    this->setFixedHeight(700);

    module = align_camera;
    frame_rate_color = 30;
    preset = RS2_RS400_VISUAL_PRESET_MEDIUM_DENSITY;
    ui->depth_unit->setEnabled(false);
    depth_unit = 0.00001;
    stream_width = 640;
    stream_height = 480;

    camera_run = false;
    ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
    ui->inform->setVisible(true);
    ui->screen_2->setVisible(false);

    system_capture = false;
    connect(this, SIGNAL(runScript()), this, SLOT(runSriptCallback()));
    connect(this, SIGNAL(positionCalculate()), this, SLOT(positionCalculateCallback()));

    QDir plugin_folder("C:\\Autoss\\Draft\\Plugins\\PicknPlace");
    if(!plugin_folder.exists()){
        if(plugin_folder.mkdir("C:\\Autoss\\Draft\\Plugins\\PicknPlace")){
            QString plugins_folder_str = "C:\\Autoss\\Draft\\Plugins\\PicknPlace";
            LPCWSTR new_dir = (const wchar_t*) plugins_folder_str.utf16();
            SetFileAttributesW(new_dir, FILE_ATTRIBUTE_HIDDEN);

            QFile detect_model_path("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\detect_model.txt");
            if(detect_model_path.open(QIODevice::WriteOnly)){
                detect_model_path.flush();
                detect_model_path.close();

                QFile system_folder("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\system_folder.txt");
                if(system_folder.open(QIODevice::WriteOnly)){
                    system_folder.flush();
                    system_folder.close();

                    QFile calibrate_folder("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\calib_folder.txt");
                    if(calibrate_folder.open(QIODevice::WriteOnly)){
                        calibrate_folder.flush();
                        calibrate_folder.close();

                        qDebug() << "Created setting plugin folder successfully";
                    }
                }
            }
        } else{
            QMessageBox::critical(this, "Setting up plugin info", "Failed to create plugin folder");
        }
    } else {
        QFile file("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\detect_model.txt");
        if(file.open(QIODevice::ReadOnly)){
            QTextStream str(&file);
            QString tmp = str.readAll();
            QString model = tmp.split("/").last();
            ui->detect_model->setText(model);
            ui->detect_model->setAlignment(Qt::AlignLeft);
            ui->detect_model->setToolTip(tmp);
            file.flush();
            file.close();

            file.setFileName("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\system_folder.txt");
            if(file.open(QIODevice::ReadOnly)){
                str.setDevice(&file);
                tmp = str.readAll();
                model = tmp.split("/").last();
                ui->system_folder->setText(model);
                ui->system_folder->setAlignment(Qt::AlignLeft);
                ui->system_folder->setToolTip(tmp);
                file.flush();
                file.close();

                file.setFileName("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\calib_folder.txt");
                if(file.open(QIODevice::ReadOnly)){
                    str.setDevice(&file);
                    tmp = str.readAll();
                    QString folder = tmp.split("/").last();
                    ui->calibration_folder->setText(folder);
                    ui->calibration_folder->setAlignment(Qt::AlignLeft);
                    ui->calibration_folder->setToolTip(tmp);
                    file.flush();
                    file.close();

                    qDebug() << "Showed all plugin info";
                }
            }
        }

        QDir system_log("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\System log");
        if(!system_log.exists()){
            if(system_log.mkdir("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\System log")){
                QString system_log_str = "C:\\Autoss\\Draft\\Plugins\\PicknPlace";
                LPCWSTR system_log_dir = (const wchar_t*) system_log_str.utf16();
                SetFileAttributesW(system_log_dir, FILE_ATTRIBUTE_HIDDEN);
                system_dir = system_log_str;
            }
        } else qDebug() << "Exits";
    }
    testing_mode = 0;
    show_graph_mode = 0;
    run_test = false;

    offset = 32.5;
    ui->camera_name->setText("Intel Realsense");

    usb_timer = new QTimer(this);
    connect(usb_timer, SIGNAL(timeout()), this, SLOT(checkUSB()));
    connect(this, SIGNAL(destroyed(QObject*)), usb_timer, SLOT(stop()));
    usb_timer->start(200);

    calib = new Calibration;
    connect_status = false;

    test_marker = false;

    ui->screen_interactor->setDisabled(true);
    system_capture = false;
    request_frame = false;
    system_detecting = false;
    system_calculating = false;
    system_finish = false;

    working_area.clear();
}

main_dialog::~main_dialog()
{
    delete ui;

    usb_timer->stop();
    delete usb_timer;
}

frameset frames;
rs2::depth_frame capture_frame(frames);

// Find devices with specified streams
bool main_dialog::device_with_streams(vector<rs2_stream> stream_requests, string& out_serial){
    context ctx;
    auto devs = ctx.query_devices();
    vector <rs2_stream> unavailable_streams = stream_requests;
    for (auto dev : devs)
    {
        map<rs2_stream, bool> found_streams;
        for (auto& type : stream_requests)
        {
            found_streams[type] = false;
            for (auto& sensor : dev.query_sensors())
            {
                for (auto& profile : sensor.get_stream_profiles())
                {
                    if (profile.stream_type() == type)
                    {
                        found_streams[type] = true;
                        unavailable_streams.erase(remove(unavailable_streams.begin(), unavailable_streams.end(), type), unavailable_streams.end());
                        if (dev.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
                            out_serial = dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
                    }
                }
            }
        }
        // Check if all streams are found in current device
        bool found_all_streams = true;
        for (auto& stream : found_streams)
        {
            if (!stream.second)
            {
                found_all_streams = false;
                break;
            }
        }
        if (found_all_streams)
            return true;
    }
    // After scanning all devices, not all requested streams were found
    for (auto& type : unavailable_streams)
    {
        switch (type)
        {
        case RS2_STREAM_POSE:
        case RS2_STREAM_FISHEYE:
            cerr << "Connect T26X and rerun the demo" << std::endl;
            break;
        case RS2_STREAM_DEPTH:
            cerr << "The demo requires Realsense camera with DEPTH sensor" << std::endl;
            break;
        case RS2_STREAM_COLOR:
            cerr << "The demo requires Realsense camera with RGB sensor" << std::endl;
            break;
        default:
            throw runtime_error("The requested stream: " + std::to_string(type) + ", for the demo is not supported by connected devices!"); // stream type
        }
    }
    return false;
}

void main_dialog::turn_cam_on(){
    //Turn off show system frame
    showFrame = false;

    string serial;
//    if (!device_with_streams({ RS2_STREAM_COLOR,RS2_STREAM_DEPTH }, serial))
//        return;

    cfg.disable_all_streams();

    if(module == RGB_camera){
        if (!device_with_streams({RS2_STREAM_COLOR}, serial))
            return;
    } else if(module == stereo_camera){
        if (!device_with_streams({RS2_STREAM_DEPTH}, serial))
            return;
    } else if(module == align_camera){
        if (!device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial))
            return;
    }

    if (!serial.empty())
        cfg.enable_device(serial);

    if(module == RGB_camera){
        cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, frame_rate_color);
    } else if(module == stereo_camera){
//        cfg.enable_stream(RS2_STREAM_INFRARED, stream_width, stream_height, RS2_FORMAT_Y8, 10);
        cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    } else if(module == align_camera){
        cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
        cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);
    }

    auto profile = pipe.start(cfg);
    auto device = profile.get_device();
    QString dev_name = device.get_info(RS2_CAMERA_INFO_NAME);
    if(dev_name.contains("D415")){
        ui->camera_name->setText("Intel Realsense D415");
        offset = 35.0;
    } else if(dev_name.contains("D435")){
        ui->camera_name->setText("Intel Realsense D435");
        offset = 32.5;
    }

    qDebug() << "started..";
    ui->console->append("Started");

    // Set the device to High Density preset of the D400 stereoscopic cameras
    if(module == RGB_camera){
        intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();
//        auto sensor = profile.get_device().first<rs2::color_sensor>();
//        if (sensor && sensor.is<rs2::color_sensor>())
//        {
//            sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_HIGH_DENSITY);
//        }
    } else if(module == stereo_camera){
        // Get stream intrinsics data
        intr = profile.get_stream(RS2_STREAM_DEPTH).as<video_stream_profile>().get_intrinsics();
//        intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();

        auto sensor = profile.get_device().first<rs2::depth_sensor>();
        if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
            sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
            sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
            qDebug() << "Depth units:" << sensor.get_depth_scale();
        }
    } else if(module == align_camera){
        // Get stream intrinsics data
//        intr = profile.get_stream(RS2_STREAM_DEPTH).as<video_stream_profile>().get_intrinsics();
        intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();
        prof = profile;

        auto sensor = profile.get_device().first<rs2::depth_sensor>();
        if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
            sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
            sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
            qDebug() << "Depth units:" << sensor.get_depth_scale();
        }
    }
    qDebug() << "set preset..";
    ui->console->append("Set preset..");

    // Camera warmup - dropping several first frames to let auto-exposure stabilize
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }
    qDebug() << "waited..";
    ui->console->append("Waited..");

    cam = new Camera;
    cam->registerCamera(module, pipe, color_map, printer);
    connect(cam, SIGNAL(sendCameraRGBFrame(double,double,double)), this, SLOT(receiveCameraRGBFrame(double,double,double)));
    connect(cam, SIGNAL(sendCamerDepthFrame(double,double,double)), this, SLOT(receiveCamerDepthFrame(double,double,double)));
    connect(cam, SIGNAL(sendCameraRGBDFrame(double,double,double)), this, SLOT(receiveCameraRGBDFrame(double,double,double)));
    cam->start();
}

void main_dialog::receiveCameraRGBFrame(double width, double height, double frame_rate){
    cv::Mat frame;
    cam->getFrame(frame);

    QString resolution = QString::number(width) + " x " + QString::number(height);
    ui->resolution->setText(resolution);

    ui->fps->setText(QString::number(frame_rate));

    ui->screen->setStyleSheet("");
    ui->inform->setVisible(false);

    if(run_test && testing_mode == 0){
        if(!wait_frame){
            wait_frame = true;
            emit testingFrameSend(frame);
            return;
        }
    }

    img = cvMatToQPixmap(frame);

    int h, w;
    w = ui->screen->width();
    h = ui->screen->height();
    ui->screen->setPixmap(img.scaled(w, h));

    cam->wait_frame = false;
}

void main_dialog::receiveCamerDepthFrame(double width, double height, double frame_rate){
    cv::Mat frame;
    cam->getFrame(frame);

    QString resolution = QString::number(width) + " x " + QString::number(height);
    ui->resolution->setText(resolution);

    ui->fps->setText(QString::number(frame_rate));

    ui->screen->setStyleSheet("");
    ui->inform->setVisible(false);

    img = cvMatToQPixmap(frame);

    int h, w;
    w = ui->screen->width();
    h = ui->screen->height();
    ui->screen->setPixmap(img.scaled(w, h));
    delay->delay(10);

    cam->wait_frame = false;
}

void main_dialog::receiveCameraRGBDFrame(double width, double height, double frame_rate){
    cv::Mat frame;
    cam->getFrame(frame);
    cam->getDepthFrame(capture_frame);

    QString resolution = QString::number(width) + " x " + QString::number(height);
    ui->resolution->setText(resolution);

    ui->fps->setText(QString::number(frame_rate));

    ui->screen->setStyleSheet("");
    ui->inform->setVisible(false);

    if(system_capture && !run_robot){
        qDebug() << "Detecting";
        run_robot = false;
        system_finish = false;
        system_detecting = true;
        system_capture = false;
        request_frame = false;
        string model_path = ui->detect_model->toolTip().toStdString();
        std::cout << "Path: " << model_path << std::endl;
        if(!system_finish && !run_robot){
            sys->systemDetectRegister(frame, capture_frame, model_path, intr, working_area, ui->system_folder->toolTip());
            sys->start(QThread::HighestPriority);
        }
    }

    if(system_detecting && !system_capture){
        if(request_frame){
            string model_path = ui->detect_model->toolTip().toStdString();
            if(!system_finish && !run_robot){
                sys->systemDetectRegister(frame, capture_frame, model_path, intr, working_area, ui->system_folder->toolTip());
                sys->start(QThread::HighestPriority);
            }
            request_frame = false;
        }
    }

    if(system_calculating){
        system_detecting = false;

        if(!sys->isRunning()){
            sys->start(QThread::HighestPriority);
//            sys->run();
        }
    }

    if(showFrame){
        return;
    }

    if(run_test && testing_mode == 0){
        if(!wait_frame){
            wait_frame = true;
            emit testingFrameSend(frame);
            return;
        }
    } else if (run_test && testing_mode == 1){
        if(!wait_frame){
            wait_frame = true;
            emit testingFrameSend(frame);
            return;
        }
    }

    img = cvMatToQPixmap(frame);

    int h, w;
    w = ui->screen->width();
    h = ui->screen->height();
    ui->screen->setPixmap(img.scaled(w, h));

    cam->wait_frame = false;
}

void main_dialog::distance(int event, int u, int v, int flags, void* param){
    qDebug() << u << v << intr.ppx << intr.ppy << intr.fx << intr.fy << capture_frame.get_data_size();
    double x = (u - intr.ppx)/intr.fx;
    double y = (v - intr.ppy)/intr.fy;
    double z = capture_frame.get_distance(int(u), int(v))*1000.0;
    x = x*z;
    y = y*z;
//    double z1 = capture_frame.get_distance(int(u - 1), int(v))*1000.0;
//    double x1 = (u - 1 - 30 - intr.ppx)/intr.fx;
//    qDebug() << x1*z1 << z1 << x1*z;
    float pix[2];
    pix[0] = u;
    pix[1] = v;
    float point[3];
    rs2_deproject_pixel_to_point(point, &intr, pix, float(z));
    qDebug() << point[0] << point[1] << point[2];
    qDebug() << x << y << z;
}

void mouse(int event, int x, int y, int flags, void* param){
    main_dialog* dia = (main_dialog*)param;
    dia->distance(event, x, y, flags, 0);
}

void main_dialog::turn_cam_off(){
    cam->stop();
    pipe.stop();
    cfg.disable_all_streams();

    delay->delay(25);
    ui->screen->clear();
    ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
    ui->screen_2->clear();
    ui->screen_2->setStyleSheet("");
    ui->screen_2->setVisible(false);
    ui->inform->setVisible(true);
}

void main_dialog::on_camera_on_clicked()
{
    if(ui->camera_on->text() == "Camera On"){
        if(ui->test_marker->text() == "Running"){
            QMessageBox::warning(this, "Camera error", "Test marker mode is on..");
            ui->console->append("Camera error: Test marker mode is on..");
            return;
        }
        if(ui->calibrate->text() == "Calculate"){
            QMessageBox::warning(this, "Camera error", "Calibration mode is on..");
            ui->console->append("Camera error: Calibration mode is on..");
            return;
        }
        if(usb == usb_21 || usb == none){
            QMessageBox::warning(this, "Turn camera on error", "Make sure camera connected as usb 3.1");
            return;
        }
        if(module == stereo_camera || module == align_camera) ui->depth_unit->setEnabled(false);
        ui->module->setEnabled(false);
        ui->preset->setEnabled(false);
        ui->camera_on->setText("Camera Off");
        ui->camera_on->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
        turn_cam_on();
    } else if(ui->camera_on->text() == "Camera Off"){
        if(ui->test_model->text() == "Running"){
            QMessageBox::warning(this, "Camera error", "Failed to turn off camera since test mode is on");
            return;
        }
        if(ui->start->text() == "Stop"){
            QMessageBox::warning(this, "Camera error", "System is running..");
            ui->console->append("Camera error: System is running..");
            return;
        }
        turn_cam_off();
        ui->resolution->clear();
        ui->fps->clear();
        if(module == stereo_camera || module == align_camera) ui->depth_unit->setEnabled(true);
        ui->module->setEnabled(true);
        ui->preset->setEnabled(true);
        ui->camera_on->setText("Camera On");
        ui->camera_on->setStyleSheet("");
    }
}

QPixmap main_dialog::cvMatToQPixmap(const cv::Mat &inMat){
    switch ( inMat.type() ){
        // 8-bit, 4 channel
        case CV_8UC4:
        {
            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_ARGB32 );

            return QPixmap::fromImage(image);
        }

            // 8-bit, 3 channel
        case CV_8UC3:
        {
            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_RGB888 );

            return QPixmap::fromImage(image.rgbSwapped());
        }

            // 8-bit, 1 channel
        case CV_8UC1:
        {
        #if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
                QImage image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_Grayscale8 );
        #else
                static QVector<QRgb>  sColorTable;

                // only create our color table the first time
                if ( sColorTable.isEmpty() )
                {
                    sColorTable.resize( 256 );

                    for ( int i = 0; i < 256; ++i )
                    {
                        sColorTable[i] = qRgb( i, i, i );
                    }
                }

                QImage image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_Indexed8 );

                image.setColorTable( sColorTable );
        #endif

            return QPixmap::fromImage(image);
        }

        default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
    }
    return QPixmap();
}

void main_dialog::on_module_currentIndexChanged(int index){
    if(index == 0){
        module = align_camera;
        ui->depth_unit->setEnabled(true);
    } else if(index == 1){
        module = RGB_camera;
        ui->depth_unit->setEnabled(false);
    } else if(index == 2){
        module = stereo_camera;
        ui->depth_unit->setEnabled(true);
    }
}

void main_dialog::on_depth_unit_currentIndexChanged(int index){
    if(index == 0){
        depth_unit = 0.00001;
    } else if(index == 1){
        depth_unit = 0.0001;
    } else if(index == 2){
        depth_unit = 0.001;
    }
}

void main_dialog::on_preset_currentIndexChanged(int index){
    if(index == 0){
        preset = RS2_RS400_VISUAL_PRESET_MEDIUM_DENSITY;
    } else if(index == 1){
        preset = RS2_RS400_VISUAL_PRESET_HIGH_DENSITY;
    } else if(index == 2){
        preset = RS2_RS400_VISUAL_PRESET_HIGH_ACCURACY;
    } else if(index == 3){
        preset = RS2_RS400_VISUAL_PRESET_DEFAULT;
    }
}

void main_dialog::on_start_clicked(){
    if(ui->start->text() == "Start"){
        if(!(module == align_camera && ui->camera_on->text() == "Camera Off")){
            QMessageBox::warning(this, "Camera status", "RGBD camera is off");
            return;
        }
        ui->screen_2->setVisible(false);
        if(ui->setup->styleSheet() == "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));"){
            ui->screen_interactor->setDisabled(true);
            disconnect(ui->screen_interactor, SIGNAL(mouseClicked()), this, SLOT(mouseClicked()));
            disconnect(ui->screen_interactor, SIGNAL(mouseMove()), this, SLOT(mouseMove()));
            ui->screen_interactor->getRealMousePosition(working_area);
            for(int i = 0; i < working_area.size(); i++){
    //            std::cout << "Points old: " << working_area[i] << std::endl;
                working_area[i].x = double(working_area[i].x)/double(ui->screen_interactor->width())*double(stream_width);
                working_area[i].y = double(working_area[i].y)/double(ui->screen_interactor->height())*double(stream_height);
                std::cout << "Points: " << working_area[i] << std::endl;
            }
            ui->screen_interactor->clearLabel();
            ui->setup->setStyleSheet("");
            ui->setup->setDisabled(true);
            ui->console->append("Load workspace successfully");
        } else{
            if(working_area.size() < 4){
                working_area.reserve(4);
                working_area.push_back(cv::Point(0,0));
                working_area.push_back(cv::Point(640,0));
                working_area.push_back(cv::Point(640,480));
                working_area.push_back(cv::Point(0,480));
                qDebug() << "Working area is set by default";
                ui->console->append("Working area is set by default");
                ui->setup->setStyleSheet("");
                ui->setup->setDisabled(true);
            }
        }
        stop = false;

        // Clear old value
        ui->x->clear();
        ui->y->clear();
        ui->z->clear();
        ui->rx->clear();
        ui->ry->clear();
        ui->rz->clear();
        detected = false;

        sys = new uSystem;
        connect(sys, SIGNAL(requestNewFrame()), this, SLOT(requestNewFrameCallback()));
        connect(sys, SIGNAL(objectDetected()), this, SLOT(objectDetectedCallback()));
        connect(sys, SIGNAL(systemFinished()), this, SLOT(systemFinishedCallback()));
        connect(sys, SIGNAL(poseCalculated()), this, SLOT(poseCalculatedCallback()));
        connect(sys, SIGNAL(loadNetError()), this, SLOT(loadNetErrorCallback()));
        connect(sys, SIGNAL(wrongCalculate()), this, SLOT(wrongCalculateCallback()));
        connect(this, SIGNAL(runRobot()), this, SLOT(runRobotCallback()));

        QFile calib_file(ui->calibration_folder->toolTip() + "/" + "Calibration.csv");
        if(!calib_file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this, "Start system error", "Can't open calibration file");
            delete sys;
            return;
        }
        QString data = QString(calib_file.readAll());
        QStringList data_array = data.split(",");

        CalibrationMatrix calib_param;
        data = data_array.at(0);
        calib_param.x = data.toDouble();
        data = data_array.at(1);
        calib_param.y = data.toDouble();
        data = data_array.at(2);
        calib_param.z = data.toDouble();
        data = data_array.at(3);
        calib_param.rx = data.toDouble();
        data = data_array.at(4);
        calib_param.ry = data.toDouble();
        data = data_array.at(5);
        calib_param.rz = data.toDouble();

        sys->systemCalibrationRegister(calib_param);
        ui->console->append("Load calibration data successfully");

        QString param_file_path = ui->system_folder->toolTip() + "/" + "parameters.json";
        qDebug() << param_file_path;
        QFile param_file(param_file_path);
        if(!param_file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::warning(this, "Start system error", "Failed to initialize robot parameters");
            return;
        }
        QString data_param = param_file.readAll();
        param_file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data_param.toUtf8());
        QJsonObject file = doc.object();
        system_robt_param = readParameters(file, PickAndPlace);
        sys->systemRobotRegister(system_robt_param);

        connect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
        read_pos = false;
        read_joint = false;
        read_status = false;
        QTextStream str;
        if(!connect_status){
            QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
            if(!ip_file.open(QIODevice::ReadOnly)){
                QMessageBox::warning(this, "Start system error", " Failed to initialize!");
                return;
            }
            str.setDevice(&ip_file);
            QString ip = str.readAll();
            qDebug() << ip;
            ip_file.flush();
            ip_file.close();

            QHostAddress udp_address;
            quint16 udp_port = 10040;
            quint16 file_port = 10041;
            udp_address.setAddress(QHostAddress(ip).toIPv4Address());
            robot.motomanSetConnection(udp_address, udp_port, file_port);
            robot.motomanConnect();
            connect_status = true;
        }
        ui->console->append("Connect robot successfully");
        delay2->delay(200);
        robot.motomanOnServo();

        home.clear();
        home.append(120.0);
        home.append(-135.0);
        home.append(-90.0);
        home.append(-180.0);
        home.append(0.0);
        home.append(0.0);


        stm32 = new servo;
        stm32->connect_UART();
        connect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        ui->console->append("Connect UART successfully");


        showFrame = false;
        system_finish = false;
        run_robot = false;
        system_capture = true;

        ui->start->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
        ui->start->setText("Stop");
    } else if(ui->start->text() == "Stop"){
        stop = true;
        sys->stop();
        sys->exit();
        while(sys->isRunning()){
            delay3->delay(20);
        }
        delay->delay(1000);
        disconnect(sys, SIGNAL(requestNewFrame()), this, SLOT(requestNewFrameCallback()));
        disconnect(sys, SIGNAL(objectDetected()), this, SLOT(objectDetectedCallback()));
        disconnect(sys, SIGNAL(systemFinished()), this, SLOT(systemFinishedCallback()));
        disconnect(sys, SIGNAL(poseCalculated()), this, SLOT(poseCalculatedCallback()));
        disconnect(sys, SIGNAL(loadNetError()), this, SLOT(loadNetErrorCallback()));
        disconnect(sys, SIGNAL(wrongCalculate()), this, SLOT(wrongCalculateCallback()));
//        disconnect(this, SIGNAL(runRobot()), this, SLOT(runRobotCallback()));
        delete sys;

        system_capture = false;
        system_detecting = false;
        system_calculating = false;
        request_frame = false;
        system_finish = false;


        ui->start->setStyleSheet("");
        ui->start->setText("Start");
        ui->setup->setDisabled(false);

        robot.motomanOffServo();
        delay->delay(500);
        disconnect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));

        delay->delay(500);
        showFrame = false;
        cam->wait_frame = false;
        qDebug() << "Stop system";

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;
    }
}

void main_dialog::on_setup_clicked(){
    if(ui->setup->styleSheet() == "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));"){
        ui->setup->setStyleSheet("");
        ui->screen_interactor->clearLabel();
        disconnect(ui->screen_interactor, SIGNAL(mouseMove(int,int)), this, SLOT(mouseMove(int,int)));
        disconnect(ui->screen_interactor, SIGNAL(mouseClicked()), this, SLOT(mouseClicked()));
        ui->screen_interactor->setDisabled(true);
        return;
    }

    if(!(module == align_camera && ui->camera_on->text() == "Camera Off")){
        QMessageBox::warning(this, "Camera status", "RGBD camera is off");
        return;
    }
    ui->screen_interactor->setDisabled(false);
    ui->setup->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
    connect(ui->screen_interactor, SIGNAL(mouseMove(int,int)), this, SLOT(mouseMove(int,int)));
    connect(ui->screen_interactor, SIGNAL(mouseClicked()), this, SLOT(mouseClicked()));
}

void main_dialog::requestNewFrameCallback(){
    qDebug() << "Failed to detect. Send new frame";
    request_frame = true;
}

void main_dialog::objectDetectedCallback(){
    qDebug() << "Object detected";


    // Begin to calculate
    system_capture = false;
    system_detecting = false;    

    if(run_robot)
        return;

    system_calculating = true;
}

void main_dialog::systemFinishedCallback(){
    qDebug() << "System finished";
    stop = true;
    system_capture = false;
    system_detecting = false;
    system_calculating = false;
    system_finish = true;
    cam->wait_frame = false;
    ui->start->setStyleSheet("");
    ui->start->setText("Start");
    ui->setup->setDisabled(false);


    disconnect(sys, SIGNAL(requestNewFrame()), this, SLOT(requestNewFrameCallback()));
    disconnect(sys, SIGNAL(objectDetected()), this, SLOT(objectDetectedCallback()));
    disconnect(sys, SIGNAL(systemFinished()), this, SLOT(systemFinishedCallback()));
    disconnect(sys, SIGNAL(poseCalculated()), this, SLOT(poseCalculatedCallback()));
    disconnect(sys, SIGNAL(loadNetError()), this, SLOT(loadNetErrorCallback()));
    disconnect(sys, SIGNAL(wrongCalculate()), this, SLOT(wrongCalculateCallback()));
    disconnect(this, SIGNAL(runRobot()), this, SLOT(runRobotCallback()));

    delete sys;
    robot.motomanOffServo();
    delay->delay(500);
    robot.motomanDisconnect();
    connect_status = false;

    stm32->stopPing();
    stm32->disconnect_UART();
    disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
    delete stm32;
}

void main_dialog::poseCalculatedCallback(){
    if(!system_calculating)
        return;
    system_capture = false;
    system_detecting = false;

    qDebug() << "Pose's calculated";
    double x, y, z, rx, ry, rz;
    sys->getPoseEstimateResult(x, y, z, rx, ry, rz);
    ui->x->setText(QString::number(x));
    ui->y->setText(QString::number(y));
    ui->z->setText(QString::number(z));
    ui->rx->setText(QString::number(rx));
    ui->ry->setText(QString::number(ry));
    ui->rz->setText(QString::number(rz));

    grippingPoint.clear();
    grippingPoint.reserve(6);
    grippingPoint.append(x);
    grippingPoint.append(y);
    grippingPoint.append(z);
    grippingPoint.append(rx);
    grippingPoint.append(ry);
    grippingPoint.append(rz);

    approachingPoint = sys->getApproachingPoint();
    qDebug() << approachingPoint;

    sys->getImageDetected(system_img);

    cv::imwrite("D:\\Study\\402\\Luan_van\\Mask\\test3.png", system_img);
    img = cvMatToQPixmap(system_img);

    showFrame = true;

    int h, w;
    w = ui->screen->width();
    h = ui->screen->height();
    ui->screen->setPixmap(img.scaled(w, h));

//    cam->wait_frame = false;

    qDebug() << "before";
    emit runRobot();

}

void main_dialog::loadNetErrorCallback(){
    QMessageBox::critical(this, "Load model error", "Failed to load net");
    qDebug() << "Error while load net";
    system_capture = false;
}

void main_dialog::wrongCalculateCallback(){
    qDebug() << "False calculation";
    system_finish = false;
    system_capture = false;

    // Request new frame
    request_frame = true;

    system_calculating = false;
    system_detecting = true;

    cam->wait_frame = false;

}

void main_dialog::runRobotCallback(){
    if(!system_calculating)
        return;
    system_calculating = false;
    run_robot = true;
    cam->wait_frame = true;
    qDebug() << "frame" << cam->wait_frame;
    qDebug() << "Run robot";
//    qDebug() << system_capture << system_detecting << system_calculating << request_frame;


    stm32->release();
    qDebug() << "Release gripper";
    delay2->delay(200);
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }

    QVector<double> p0;
    p0.append(215.0);
    p0.append(-123.0);
    p0.append(-30.0);
    p0.append(-180.0);
    p0.append(0.0);
    p0.append(0.0);
    double return_speed = 80.0;

    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, return_speed + 30.0, &p0);
    qDebug() << "Initial point" << p0[0] << p0[1] << p0[2] << p0[3] << p0[4] << p0[5];

    delay->delay(500);
    status = false;
    while(!status){
        read_status = true;
        robot.motomanReadStatus();
        delay2->delay(20);
//        qDebug() << "Request read";
    }
    qDebug() << "Ready";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay->delay(500);

    double speed = 60.0;
    QVector<double> p1;
    p1.append(approachingPoint[0]);
    p1.append(approachingPoint[1]);
    p1.append(approachingPoint[2]);
    p1.append(approachingPoint[3]);
    p1.append(approachingPoint[4]);
    p1.append(approachingPoint[5]);

    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, speed, &p1);
    qDebug() << "Approaching" << approachingPoint[0] << approachingPoint[1] << approachingPoint[2] << approachingPoint[3] << approachingPoint[4] << approachingPoint[5];

    delay->delay(500);
    status = false;
    while(!status){
        read_status = true;
        robot.motomanReadStatus();
        delay2->delay(20);
//        qDebug() << "Request read";
    }
    qDebug() << "Ready";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay->delay(500);

    QVector<double> p2;
    p2.append(grippingPoint[0]);
    p2.append(grippingPoint[1]);
    p2.append(grippingPoint[2]);
    p2.append(grippingPoint[3]);
    p2.append(grippingPoint[4]);
    p2.append(grippingPoint[5]);

    speed = 30.0;
    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, speed, &p2);
    qDebug() << "Grip:" << grippingPoint[0] << grippingPoint[1] << grippingPoint[2] << grippingPoint[3] << grippingPoint[4] << grippingPoint[5];

    delay->delay(500);
    status = false;
    while(!status){
        read_status = true;
        robot.motomanReadStatus();
        connect_status = false;
        delay2->delay(20);
//        qDebug() << "Request read";
    }
    qDebug() << "Ready";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay->delay(500);

    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;  
    stm32->grab();
    qDebug() << "Close gripper";
    delay2->delay(1000);
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }

    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, speed + 50.0, &p0);
    qDebug() << "Initial point" << p0[0] << p0[1] << p0[2] << p0[3] << p0[4] << p0[5];

    delay->delay(500);
    status = false;
    while(!status){
        read_status = true;
        robot.motomanReadStatus();
        connect_status = false;
        delay2->delay(20);
//        qDebug() << "Request read";
    }
    qDebug() << "Ready";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay->delay(500);

    QVector<double> p3;
    p3.append(140.0);
    p3.append(-240.0);
    p3.append(-30.0);
    p3.append(-180.0);
    p3.append(0.0);
    p3.append(0.0);
    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, return_speed + 30.0, &p3);
    qDebug() << "Place" << p3[0] << p3[1] << p3[2] << p3[3] << p3[4] << p3[5];

    delay->delay(500);
    status = false;
    while(!status){
        read_status = true;
        robot.motomanReadStatus();
        connect_status = false;
        delay2->delay(20);
//        qDebug() << "Request read";
    }
    qDebug() << "Ready";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay->delay(200);

    if(!run_robot && (system_capture || system_detecting || system_calculating))
        return;
    stm32->release();
    qDebug() << "Release gripper";
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }
    delay2->delay(1000);

    showFrame = false;
    if(stop){
        delay->delay(100);
        robot.motomanDisconnect();
        connect_status = false;
        stop = false;
        run_robot = false;

        stm32->stopPing();
        stm32->disconnect_UART();
        disconnect(stm32, SIGNAL(pingFail()), this, SLOT(pingFailCallback()));
        delete stm32;

        return;
    }

    // Perform next detection
    if(!stop)
        system_capture = true;
    cam->wait_frame = false;
    run_robot = false;
}

void main_dialog::pingFailCallback(){
    qDebug() << " Ping failed";
    QMessageBox::warning(this, "Stm32 connect error", "Disconnected to UART...");
    double speed = 80.0;
    QVector<double> p;
    p.append(140.0);
    p.append(-240.0);
    p.append(-30.0);
    p.append(-180.0);
    p.append(0.0);
    p.append(0.0);
    robot.motomanMoveCartesian(CMD_DATA_MOVE_COORDINATE_TYPE_ROBOT, CMD_HEADER_MOVE_CARTESIAN_INSTANCE_LINK_ABS, CMD_DATA_MOVE_SPEED_TYPE_LINK_ABS, speed, &p);
    delay->delay(2000);
    on_start_clicked();
}

void main_dialog::on_Evaluate_model_clicked(){
    if(ui->Evaluate_model->text() == "Evaluate Model"){
        connect(&ev, SIGNAL(evaluateObject2Cam()), this, SLOT(evaluateObject2CamCallback()));
        connect(&ev, SIGNAL(evaluateObject2BasePosition()), this, SLOT(evaluateObject2BasePositionCallback()));
        connect(&ev, SIGNAL(evaluateObject2BaseRy()), this, SLOT(evaluateObject2BaseRyCallback()));
        connect(&ev, SIGNAL(evaluateObject2BaseRz()), this, SLOT(evaluateObject2BaseRzCallback()));
        ev.show();
        evaluate = false;
        evaluate_count = 0;
    } else{
        ui->screen->clear();
        ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
        ui->inform->setVisible(true);
        ui->Evaluate_model->setStyleSheet("");
        ui->Evaluate_model->setText("Evaluate Model");
        evaluate = false;
    }

}

void main_dialog::evaluateObject2CamCallback(){
    string serial;
    !device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial);
    if (!serial.empty())
        cfg.enable_device(serial);
    cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

    auto profile = pipe.start(cfg);
    intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();


    auto sensor = profile.get_device().first<rs2::depth_sensor>();
    if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
        sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
        sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
        qDebug() << "Depth units:" << sensor.get_depth_scale();
    }

    frameset frames;
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }

    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cameraMatrix.at<double>(0,0) = intr.fx;
    cameraMatrix.at<double>(0,2) = intr.ppx;
    cameraMatrix.at<double>(1,1) = intr.fy;
    cameraMatrix.at<double>(1,2) = intr.ppy;

    vector<double> distCoeffs(5);
    for(int i = 0; i < 5; i++)
        distCoeffs[i] = intr.coeffs[i];

    float markerLength = 0.054;
    // Set coordinate system
    cv::Mat objPoints(4, 1, CV_32FC3);
    objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerLength/2.f, markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerLength/2.f, -markerLength/2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerLength/2.f, -markerLength/2.f, 0);

    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_250);
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);

    cv::Mat img;

    frames = pipe.wait_for_frames();

    rs2::align align_to_color(RS2_STREAM_COLOR);
    auto aligned_frames = align_to_color.process(frames);

    auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map);
    auto dep_frame = aligned_frames.get_depth_frame();

    auto str_width = color_frame.as<video_frame>().get_width();
    auto str_height = color_frame.as<video_frame>().get_height();

    auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

    // Creating OpenCV Matrix from a color image
    cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);


    img = color;

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    detector.detectMarkers(img, corners, ids);

    // If at least one marker detected
    if (ids.size() <= 0){
        qDebug() << "Can't detect tag";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }
    cv::aruco::drawDetectedMarkers(img, corners, ids);

    int nMarkers = corners.size();
    std::vector<cv::Vec3d> rvecs(nMarkers), tvecs(nMarkers);

    // Calculate pose for each marker
    for (int i = 0; i < nMarkers; i++)
        cv::solvePnP(objPoints, corners.at(i), cameraMatrix, distCoeffs, rvecs.at(i), tvecs.at(i));

    // Draw axis for each marker
    for(unsigned int i = 0; i < ids.size(); i++)
        cv::drawFrameAxes(img, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.025);

    cv::Matx33d rvecs_new;
    cv::Rodrigues(rvecs, rvecs_new);

    double temp = pow(rvecs_new(2,1), 2.0) + pow(rvecs_new(2,2), 2.0);
    double Rx, Ry, Rz;
    if(temp != 0){
        Rx = atan2(rvecs_new(2,1),rvecs_new(2,2))*180.0/3.14;
        Ry = atan2(-rvecs_new(2,0),sqrt(temp))*180.0/M_PI;
        Rz = atan2(rvecs_new(1,0),rvecs_new(0,0))*180.0/M_PI;
    } else if(temp == 0){
        if(rvecs_new(2,0) == (double)-1){
            Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
            Ry = 90.0;
            Rz = 0;
        } else if(rvecs_new(2,0) == (double)1){
            Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
            Ry = -90.0;
            Rz = 0;
        }
    }

    for(unsigned int i = 0; i < ids.size(); i++){
        tvecs.at(i) = tvecs.at(i)*1000.0;
        //                        rvecs.at(i) = (rvecs.at(i)/M_PI)*180.0;
    }

    for(unsigned int i = 0; i < ids.size(); i++)
        std::cout << "Marker " << std::to_string(i+1) << std::endl << "Rotation: " << Rx << ", " << Ry << ", " << Rz << std::endl << "Translation: " << tvecs[i] << std::endl;

    // tagToCam
    vpRzyxVector cRt_v(Rz*M_PI/180.0, Ry*M_PI/180.0, Rx*M_PI/180.0);
    vpRotationMatrix cRt(cRt_v);
    vpTranslationVector cTt(tvecs[0][0]/1000.0, tvecs[0][1]/1000.0, tvecs[0][2]/1000.0);
    vpHomogeneousMatrix cMt(cTt, cRt);
    // objectTotag
    vpRzyxVector tRo_v(-90.0*M_PI/180.0, 0.0*M_PI/180.0, 180.0*M_PI/180.0);
    vpRotationMatrix tRo(tRo_v);
    double thread_length = 39.0;
    double y_offset = thread_length/2.0 - 10.0;
    vpTranslationVector tTo(-49.5/1000.0, y_offset/1000.0, 14.0/1000.0);
    vpHomogeneousMatrix tMo(tTo, tRo);
    // objectTocam
    vpHomogeneousMatrix cTo = cMt*tMo;
    vpRzyxVector cTo_v(cTo.getRotationMatrix());

    ev.getReference(cTo[0][3]*1000.0, cTo[1][3]*1000.0, cTo[2][3]*1000.0, cTo_v[2]*180.0/M_PI, cTo_v[1]*180.0/M_PI, cTo_v[0]*180.0/M_PI);

    string model_path = ui->detect_model->toolTip().toStdString();
    if(!evaluate){
        model.ReadModel(net, model_path, true);
        evaluate = true;
    }
    if(!model.Detect(img, net, result)){
        qDebug() << "Failed to detect object";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }
    model.DrawMaskSingleOnly(img, result[0], cv::Scalar(180,105,255), ui->system_folder->toolTip().toStdString() + "/" + "RGB.jpg");

    cv::Mat mask = result[0].boxMask;
    cv::imwrite(ui->system_folder->toolTip().toStdString() + "\\mask.png", mask);
    cv::Mat mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask.png");
    pose.createBackground(mask_bg, result[0].box.x, result[0].box.y, result[0].box.width, result[0].box.height, ui->system_folder->toolTip().toStdString());
    mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask_bg.png");
    std::vector<cv::Point> points;
    bool head_up;
    double rz = pose.run(mask_bg, ui->system_folder->toolTip(), points, 0, head_up);

    if(points.size() < 2){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }

    //    double u = result[0].box.x + result[0].box.width/2;
    //    double v = result[0].box.y + result[0].box.height/2;
    double u = (double(points[4].x) + double(points[3].x))/2.0;
    double v = (double(points[4].y) + double(points[3].y))/2.0;

    double z = dep_frame.get_distance(int(u), int(v))*1000.0;
    double x = (u - intr.ppx)/intr.fx*z;
    double y = (v - intr.ppy)/intr.fy*z;
    qDebug() << x << y << z;
    if(z < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }

    double u1 = u;
    double v1 = v;
    double z1 = z;
    double u2 = (double(points[3].x) + double(points[0].x))/2.0;
    double v2 = (double(points[3].y) + double(points[0].y))/2.0;
    double z2 = dep_frame.get_distance(int(u2), int(v2))*1000.0;
    if(z2 < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    double l = sqrt(pow(u1 - u2, 2.0) + pow(v1 - v2, 2.0));
    double ry = 0.0;
//    if(z1 > z2)
//        z1 = z1*1.002;
//    else if(z1 < z2)
//        z2 = z2*1.002;

    qDebug() << "u1-v1-z1:" << u1 << v1 << z1;
    qDebug() << "u2-v2-z2:" << u2 << v2 << z2;

//    if(head_up)
//        ry = atan2(z1 - z2,l)*180.0/M_PI;
//    else
//        ry = atan2(z2 - z1,l)*180.0/M_PI;
    ry = atan2(abs(z1 - z2),l*(1.0/2.6))*180.0/M_PI;
    if(z1 > z2)
        ry = - ry;

    if(ry > 50.0 || ry < -50.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    ev.getPredictedResult(x, y, z, 0, ry, rz);

    QPixmap screen = cvMatToQPixmap(img);
    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));
    ui->inform->setVisible(false);

    ev.stopWait();

    pipe.stop();
    cfg.disable_all_streams();

    double error_x = cTo[0][3]*1000.0 - x;
    double error_y = cTo[1][3]*1000.0 - y;
    double error_z = cTo[2][3]*1000.0 - z;
    qDebug() << "Error" << error_x << error_y << error_z;
    if(abs(error_x) > 4.0 || abs(error_y) > 4.0 || abs(error_z) > 6.0){
        qDebug() << "Error over";
    } else{
        QFile data_file(ui->system_folder->toolTip() + "/" + "objectTocam-12x40.csv");
        data_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        QTextStream str(&data_file);
        str << QString::number(x) << "," << QString::number(y) << "," << QString::number(z) << "," << QString::number(0.0) << "," << QString::number(ry) << "," << QString::number(rz) << "," << QString::number(cTo[0][3]*1000.0) << "," << QString::number(cTo[1][3]*1000.0) << "," << QString::number(cTo[2][3]*1000.0) << "," << QString::number(cTo_v[2]*180.0/M_PI) << "," << QString::number(cTo_v[1]*180.0/M_PI) << "," << QString::number(cTo_v[0]*180.0/M_PI) << "\n";
        data_file.flush();
        data_file.close();
    }

//    QFile data_file2(ui->system_folder->toolTip() + "/" + "objectTocam-2.csv");
//    data_file2.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
//    QTextStream str2(&data_file2);
//    str2 << QString::number(cTo[0][3]*1000.0) << "," << QString::number(cTo[1][3]*1000.0) << "," << QString::number(cTo[2][3]*1000.0) << "," << QString::number(cTo_v[2]*180.0/M_PI) << "," << QString::number(cTo_v[1]*180.0/M_PI) << "," << QString::number(cTo_v[0]*180.0/M_PI) << "\n";
//    data_file2.flush();
//    data_file2.close();

    ui->Evaluate_model->setText("Done");
    ui->Evaluate_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
}

void main_dialog::evaluateObject2BasePositionCallback(){
    string serial;
    !device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial);
    if (!serial.empty())
        cfg.enable_device(serial);
    cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

    auto profile = pipe.start(cfg);
    intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();


    auto sensor = profile.get_device().first<rs2::depth_sensor>();
    if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
        sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
        sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
        qDebug() << "Depth units:" << sensor.get_depth_scale();
    }

    frameset frames;
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }

    frames = pipe.wait_for_frames();

    rs2::align align_to_color(RS2_STREAM_COLOR);
    auto aligned_frames = align_to_color.process(frames);

    auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map);
    auto dep_frame = aligned_frames.get_depth_frame();

    auto str_width = color_frame.as<video_frame>().get_width();
    auto str_height = color_frame.as<video_frame>().get_height();

    auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

    // Creating OpenCV Matrix from a color image
    cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

    cv::Mat img = color;

    string model_path = ui->detect_model->toolTip().toStdString();
    if(!evaluate){
        model.ReadModel(net, model_path, true);
        evaluate = true;
    }
    if(!model.Detect(img, net, result)){
        qDebug() << "Failed to detect object";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }
    model.DrawMaskSingleOnly(img, result[0], cv::Scalar(180,105,255), ui->system_folder->toolTip().toStdString() + "/" + "RGB.jpg");

    cv::Mat mask = result[0].boxMask;
    cv::imwrite(ui->system_folder->toolTip().toStdString() + "\\mask.png", mask);
    cv::Mat mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask.png");
    pose.createBackground(mask_bg, result[0].box.x, result[0].box.y, result[0].box.width, result[0].box.height, ui->system_folder->toolTip().toStdString());
    mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask_bg.png");
    std::vector<cv::Point> points;
    bool head_up;
    double rz = pose.run(mask_bg, ui->system_folder->toolTip(), points, 0, head_up);

    if(points.size() < 2){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    //    double u = result[0].box.x + result[0].box.width/2;
    //    double v = result[0].box.y + result[0].box.height/2;
    double u = (double(points[4].x) + double(points[3].x))/2.0;
    double v = (double(points[4].y) + double(points[3].y))/2.0;

    double z = dep_frame.get_distance(int(u), int(v))*1000.0;
    double x = (u - intr.ppx)/intr.fx*z;
    double y = (v - intr.ppy)/intr.fy*z;
    qDebug() << x << y << z;
    if(z < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    cv::circle(img, cv::Point(int(u), int(v)), 4, cv::Scalar(120,222,0));

    QFile calib_file(ui->calibration_folder->toolTip() + "/" + "Calibration.csv");
    if(!calib_file.open(QIODevice::ReadOnly)){
        return;
    }
    QString data = QString(calib_file.readAll());
    QStringList data_array = data.split(",");

    CalibrationMatrix calib_param;
    data = data_array.at(0);
    calib_param.x = data.toDouble();
    data = data_array.at(1);
    calib_param.y = data.toDouble();
    data = data_array.at(2);
    calib_param.z = data.toDouble();
    data = data_array.at(3);
    calib_param.rx = data.toDouble();
    data = data_array.at(4);
    calib_param.ry = data.toDouble();
    data = data_array.at(5);
    calib_param.rz = data.toDouble();

    // camTbase
    vpRzyxVector bRc_v(calib_param.rz*M_PI/180.0, calib_param.ry*M_PI/180.0, calib_param.rx*M_PI/180.0);
    vpRotationMatrix bRc(bRc_v);
    vpTranslationVector bTc(calib_param.x/1000.0, calib_param.y/1000.0, calib_param.z/1000.0);
    vpHomogeneousMatrix bMc(bTc, bRc);

    // objectTcam
    vpRzyxVector cRo_v(0.0*M_PI/180.0, 0.0*M_PI/180.0, 0.0*M_PI/180.0);
    vpRotationMatrix cRo(cRo_v);
    vpTranslationVector cTo(x/1000.0, y/1000.0, z/1000.0);
    vpHomogeneousMatrix cMo(cTo, cRo);

    // objectTbase
    vpHomogeneousMatrix bMo = bMc*cMo;
    vpRzyxVector bRo_v(bMo.getRotationMatrix());
    x = bMo[0][3]*1000.0;
    y = bMo[1][3]*1000.0;
    z = bMo[2][3]*1000.0;

    uSystem error;
    double e_x, e_y, e_z, e_ry;
    error.errorEvaluate(x, y, z, 0.0, 0.0, rz, e_x, e_y, e_z, e_ry, 3);
    if(e_x > 5.0)
        e_x = 5.0;
    else if(e_x < -5.0)
        e_x = -5.0;
    if(e_y > 5.0)
        e_y = 5.0;
    else if(e_y < -5.0)
        e_y = -5.0;
    qDebug() << "before:" << x << y;
    x = x + e_x;
    y = y - e_y;
    qDebug() << "after:" << x << y;
    ev.getPredictedResult(x, y, z, 0, 0, 0);

    QTextStream str_temp;
    if(!connect_status){
        QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
        if(!ip_file.open(QIODevice::ReadOnly)){
            return;
        }
        connect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
        str_temp.setDevice(&ip_file);
        QString ip = str_temp.readAll();
        qDebug() << ip;
        ip_file.flush();
        ip_file.close();

        QHostAddress udp_address;
        quint16 udp_port = 10040;
        quint16 file_port = 10041;
        udp_address.setAddress(QHostAddress(ip).toIPv4Address());
        robot.motomanSetConnection(udp_address, udp_port, file_port);
        robot.motomanConnect();
        connect_status = true;
        qDebug() << "Connected";
    }

    read_pos = true;
    Cartesian robot_pos;
    robot.motomanReadPosition();
    while(1){
        if(!read_pos){
            robot_pos.x = robot_position[0];
            robot_pos.y = robot_position[1];
            robot_pos.z = robot_position[2];
            robot_pos.rx = robot_position[3];
            robot_pos.ry = robot_position[4];
            robot_pos.rz = robot_position[5];
            break;
        }
        delay->delay(10);
    }
    robot.motomanDisconnect();
    disconnect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
    connect_status = false;

    ev.getReference(robot_pos.x, robot_pos.y, robot_pos.z, 0.0, 0.0, 0.0);

    QFile data_file(ui->system_folder->toolTip() + "/" + "x-y-z.csv");
    data_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    QTextStream str(&data_file);
    str << QString::number(x) << "," << QString::number(y) << "," << QString::number(z) << "," << QString::number(robot_pos.x) << "," << QString::number(robot_pos.y) << "," << QString::number(robot_pos.z) << "\n";
    data_file.flush();
    data_file.close();

    QPixmap screen = cvMatToQPixmap(img);
    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));
    ui->inform->setVisible(false);

    ev.stopWait();

    pipe.stop();
    cfg.disable_all_streams();

    ui->Evaluate_model->setText("Done");
    ui->Evaluate_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
}

void main_dialog::evaluateObject2BaseRzCallback(){
    string serial;
    !device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial);
    if (!serial.empty())
        cfg.enable_device(serial);
    cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

    auto profile = pipe.start(cfg);
    intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();


    auto sensor = profile.get_device().first<rs2::depth_sensor>();
    if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
        sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
        sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
        qDebug() << "Depth units:" << sensor.get_depth_scale();
    }

    frameset frames;
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }

    frames = pipe.wait_for_frames();

    rs2::align align_to_color(RS2_STREAM_COLOR);
    auto aligned_frames = align_to_color.process(frames);

    auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map);
    auto dep_frame = aligned_frames.get_depth_frame();

    auto str_width = color_frame.as<video_frame>().get_width();
    auto str_height = color_frame.as<video_frame>().get_height();

    auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

    // Creating OpenCV Matrix from a color image
    cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

    cv::Mat img = color;

    string model_path = ui->detect_model->toolTip().toStdString();
    if(!evaluate){
        model.ReadModel(net, model_path, true);
        evaluate = true;
    }
    if(!model.Detect(img, net, result)){
        qDebug() << "Failed to detect object";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }
    model.DrawMaskSingleOnly(img, result[0], cv::Scalar(180,105,255), ui->system_folder->toolTip().toStdString() + "/" + "RGB.jpg");

    cv::Mat mask = result[0].boxMask;
    cv::imwrite(ui->system_folder->toolTip().toStdString() + "\\mask.png", mask);
    cv::Mat mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask.png");
    pose.createBackground(mask_bg, result[0].box.x, result[0].box.y, result[0].box.width, result[0].box.height, ui->system_folder->toolTip().toStdString());
    mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask_bg.png");
    std::vector<cv::Point> points;
    bool head_up;
    double rz = pose.run(mask_bg, ui->system_folder->toolTip(), points, 0, head_up);

    if(points.size() < 2){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    QFile calib_file(ui->calibration_folder->toolTip() + "/" + "Calibration.csv");
    if(!calib_file.open(QIODevice::ReadOnly)){
        return;
    }
    QString data = QString(calib_file.readAll());
    QStringList data_array = data.split(",");

    CalibrationMatrix calib_param;
    data = data_array.at(0);
    calib_param.x = data.toDouble();
    data = data_array.at(1);
    calib_param.y = data.toDouble();
    data = data_array.at(2);
    calib_param.z = data.toDouble();
    data = data_array.at(3);
    calib_param.rx = data.toDouble();
    data = data_array.at(4);
    calib_param.ry = data.toDouble();
    data = data_array.at(5);
    calib_param.rz = data.toDouble();

    // camTbase
    vpRzyxVector bRc_v(calib_param.rz*M_PI/180.0, calib_param.ry*M_PI/180.0, calib_param.rx*M_PI/180.0);
    vpRotationMatrix bRc(bRc_v);
    vpTranslationVector bTc(calib_param.x/1000.0, calib_param.y/1000.0, calib_param.z/1000.0);
    vpHomogeneousMatrix bMc(bTc, bRc);

    // objectTcam
    vpRzyxVector cRo_v(rz*M_PI/180.0, 0.0*M_PI/180.0, 0.0*M_PI/180.0);
    vpRotationMatrix cRo(cRo_v);
    vpTranslationVector cTo(0.0/1000.0, 0.0/1000.0, 0.0/1000.0);
    vpHomogeneousMatrix cMo(cTo, cRo);

    // objectTbase
    vpHomogeneousMatrix bMo = bMc*cMo;
    vpRzyxVector bRo_v(bMo.getRotationMatrix());
    rz = bRo_v[0]*180.0/M_PI;

    ev.getPredictedResult(0, 0, 0, 0, 0, rz);

    QTextStream str_temp;
    if(!connect_status){
        QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
        if(!ip_file.open(QIODevice::ReadOnly)){
            return;
        }
        connect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
        str_temp.setDevice(&ip_file);
        QString ip = str_temp.readAll();
        qDebug() << ip;
        ip_file.flush();
        ip_file.close();

        QHostAddress udp_address;
        quint16 udp_port = 10040;
        quint16 file_port = 10041;
        udp_address.setAddress(QHostAddress(ip).toIPv4Address());
        robot.motomanSetConnection(udp_address, udp_port, file_port);
        robot.motomanConnect();
        connect_status = true;
        qDebug() << "Connected";
    }

    read_pos = true;
    Cartesian robot_pos;
    robot.motomanReadPosition();
    while(1){
        if(!read_pos){
            robot_pos.x = robot_position[0];
            robot_pos.y = robot_position[1];
            robot_pos.z = robot_position[2];
            robot_pos.rx = robot_position[3];
            robot_pos.ry = robot_position[4];
            robot_pos.rz = robot_position[5];
            break;
        }
        delay->delay(10);
    }
    robot.motomanDisconnect();
    disconnect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
    connect_status = false;

    ev.getReference(0.0, 0.0, 0.0, 0.0, 0.0, robot_pos.rz);

    QFile data_file(ui->system_folder->toolTip() + "/" + "Rz.csv");
    data_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    QTextStream str(&data_file);
    str << QString::number(rz) << "," << QString::number(robot_pos.rz) << "\n";
    data_file.flush();
    data_file.close();

    QPixmap screen = cvMatToQPixmap(img);
    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));
    ui->inform->setVisible(false);

    ev.stopWait();

    pipe.stop();
    cfg.disable_all_streams();

    ui->Evaluate_model->setText("Done");
    ui->Evaluate_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
}

void main_dialog::evaluateObject2BaseRyCallback(){
    string serial;
    !device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial);
    if (!serial.empty())
        cfg.enable_device(serial);
    cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

    auto profile = pipe.start(cfg);
    intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();


    auto sensor = profile.get_device().first<rs2::depth_sensor>();
    if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
        sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
        sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
        qDebug() << "Depth units:" << sensor.get_depth_scale();
    }

    frameset frames;
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }

    frames = pipe.wait_for_frames();

    rs2::align align_to_color(RS2_STREAM_COLOR);
    auto aligned_frames = align_to_color.process(frames);

    rs2::spatial_filter spat;
    spat.set_option(RS2_OPTION_HOLES_FILL, 5);

    auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map);
    auto dep_frame = aligned_frames.get_depth_frame();

    auto str_width = color_frame.as<video_frame>().get_width();
    auto str_height = color_frame.as<video_frame>().get_height();

    auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

    // Creating OpenCV Matrix from a color image
    cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

    cv::Mat img = color;

    string model_path = ui->detect_model->toolTip().toStdString();
    if(!evaluate){
        model.ReadModel(net, model_path, true);
        evaluate = true;
    }
    if(!model.Detect(img, net, result)){
        qDebug() << "Failed to detect object";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }
    model.DrawMaskSingleOnly(img, result[0], cv::Scalar(180,105,255), ui->system_folder->toolTip().toStdString() + "/" + "RGB.jpg");

    cv::Mat mask = result[0].boxMask;
    cv::imwrite(ui->system_folder->toolTip().toStdString() + "\\mask.png", mask);
    cv::Mat mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask.png");
    pose.createBackground(mask_bg, result[0].box.x, result[0].box.y, result[0].box.width, result[0].box.height, ui->system_folder->toolTip().toStdString());
    mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask_bg.png");
    std::vector<cv::Point> points;
    bool head_up;
    double rz = pose.run(mask_bg, ui->system_folder->toolTip(), points, 0, head_up);

    if(points.size() < 2){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    //    double u = result[0].box.x + result[0].box.width/2;
    //    double v = result[0].box.y + result[0].box.height/2;
    double u = (double(points[4].x) + double(points[3].x))/2.0;
    double v = (double(points[4].y) + double(points[3].y))/2.0;

    double z = dep_frame.get_distance(int(u), int(v))*1000.0;
    double x = (u - intr.ppx)/intr.fx*z;
    double y = (v - intr.ppy)/intr.fy*z;
    qDebug() << x << y << z;
    if(z < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    double u2 = (u + double(points[3].x))/2.0;
    double v2 = (v + double(points[3].y))/2.0;
    double z2 = dep_frame.get_distance(int(u2), int(v2))*1000.0;;
    double u1 = (double(points[3].x) + double(points[0].x))/2.0;
    u1 = (u1 + double(points[4].x))/2.0;
    double v1 = (double(points[3].y) + double(points[0].y))/2.0;
    v1 = (v1 + double(points[4].y))/2.0;
    double z1 = dep_frame.get_distance(int(u1), int(v1))*1000.0;
    if(z1 < 1.0 || z2 < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }
    qDebug() << "u1-v1-z1:" << u1 << v1 << z1;
    qDebug() << "u2-v2-z2:" << u2 << v2 << z2;

    double l = sqrt(pow(u1 - u2, 2.0) + pow(v1 - v2, 2.0));
    double ry = 0.0;
//    if(z1 > z2)
//        z1 = z1*1.002;
//    else if(z1 < z2)
//        z2 = z2*1.002;
    ry = atan2(abs(z1 - z2),l*(1.0/2.6))*180.0/M_PI;
    if(z1 > z2)
        ry = - ry;

    if(ry > 50.0 || ry < -50.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }
    qDebug() << "Done";

    QFile calib_file(ui->calibration_folder->toolTip() + "/" + "Calibration.csv");
    if(!calib_file.open(QIODevice::ReadOnly)){
        return;
    }
    QString data = QString(calib_file.readAll());
    QStringList data_array = data.split(",");

    CalibrationMatrix calib_param;
    data = data_array.at(0);
    calib_param.x = data.toDouble();
    data = data_array.at(1);
    calib_param.y = data.toDouble();
    data = data_array.at(2);
    calib_param.z = data.toDouble();
    data = data_array.at(3);
    calib_param.rx = data.toDouble();
    data = data_array.at(4);
    calib_param.ry = data.toDouble();
    data = data_array.at(5);
    calib_param.rz = data.toDouble();
    calib_file.close();

    // camTbase
    vpRzyxVector bRc_v(calib_param.rz*M_PI/180.0, calib_param.ry*M_PI/180.0, calib_param.rx*M_PI/180.0);
    vpRotationMatrix bRc(bRc_v);
    vpTranslationVector bTc(calib_param.x/1000.0, calib_param.y/1000.0, calib_param.z/1000.0);
    vpHomogeneousMatrix bMc(bTc, bRc);

    // objectTcam
    vpRzyxVector cRo_v(rz*M_PI/180.0, ry*M_PI/180.0, 0.0*M_PI/180.0);
    vpRotationMatrix cRo(cRo_v);
    vpTranslationVector cTo(0.0/1000.0, 0.0/1000.0, 0.0/1000.0);
    vpHomogeneousMatrix cMo(cTo, cRo);

    // objectTbase
    vpHomogeneousMatrix bMo = bMc*cMo;
    vpRzyxVector bRo_v(bMo.getRotationMatrix());
    ry = bRo_v[1]*180.0/M_PI;

    double modified_ry = ry;
    ev.getPredictedResult(0, 0, 0, 0, modified_ry, 0);

    cv::circle(img, cv::Point(int(u1), int(v1)), 4, cv::Scalar(120,234,100), 4);
    cv::circle(img, cv::Point(int(u2), int(v2)), 4, cv::Scalar(120,234,100), 4);
    QPixmap screen = cvMatToQPixmap(img);
    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));
    ui->inform->setVisible(false);

    ev.stopWait();

    pipe.stop();
    cfg.disable_all_streams();

    QTextStream str_temp;
    if(!connect_status){
        QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
        if(!ip_file.open(QIODevice::ReadOnly)){
            return;
        }
        connect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
        str_temp.setDevice(&ip_file);
        QString ip = str_temp.readAll();
        qDebug() << ip;
        ip_file.flush();
        ip_file.close();

        QHostAddress udp_address;
        quint16 udp_port = 10040;
        quint16 file_port = 10041;
        udp_address.setAddress(QHostAddress(ip).toIPv4Address());
        robot.motomanSetConnection(udp_address, udp_port, file_port);
        robot.motomanConnect();
        connect_status = true;
        qDebug() << "Connected";
    }

    read_pos = true;
    Cartesian robot_pos;
    robot.motomanReadPosition();
    while(1){
        if(!read_pos){
            robot_pos.x = robot_position[0];
            robot_pos.y = robot_position[1];
            robot_pos.z = robot_position[2];
            robot_pos.rx = robot_position[3];
            robot_pos.ry = robot_position[4];
            robot_pos.rz = robot_position[5];
            break;
        }
        delay->delay(10);
    }
    robot.motomanDisconnect();
    disconnect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
    connect_status = false;

    ev.getReference(0.0, 0.0, 0.0, 0.0, robot_pos.ry, 0.0);

    QFile data_file(ui->system_folder->toolTip() + "/" + "Ry.csv");
    data_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    QTextStream str(&data_file);
    str << QString::number(ry) << "," << QString::number(modified_ry) << "," << QString::number(robot_pos.ry) << "\n";
    data_file.flush();
    data_file.close();

    ui->Evaluate_model->setText("Done");
    ui->Evaluate_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
}

void main_dialog::evaluateObjectTimeCalculationCallback(){
    string serial;
    !device_with_streams({RS2_STREAM_COLOR, RS2_STREAM_DEPTH}, serial);
    if (!serial.empty())
        cfg.enable_device(serial);
    cfg.enable_stream(RS2_STREAM_DEPTH, stream_width, stream_height, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

    auto profile = pipe.start(cfg);
    intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();

    auto sensor = profile.get_device().first<rs2::depth_sensor>();
    if (sensor && sensor.is<rs2::depth_stereo_sensor>()){
        sensor.set_option(RS2_OPTION_VISUAL_PRESET, preset);
        sensor.set_option(RS2_OPTION_DEPTH_UNITS, depth_unit);
        qDebug() << "Depth units:" << sensor.get_depth_scale();
    }

    frameset frames;
    for(int i = 0; i < 15; i++)
    {
        //Wait for all configured streams to produce a frame
        frames = pipe.wait_for_frames();
    }

    frames = pipe.wait_for_frames();

    rs2::align align_to_color(RS2_STREAM_COLOR);
    auto aligned_frames = align_to_color.process(frames);

    auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map);
    auto dep_frame = aligned_frames.get_depth_frame();

    auto str_width = color_frame.as<video_frame>().get_width();
    auto str_height = color_frame.as<video_frame>().get_height();

    auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

    // Creating OpenCV Matrix from a color image
    cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

    cv::Mat img = color;

    string model_path = ui->detect_model->toolTip().toStdString();
    if(!evaluate){
        model.ReadModel(net, model_path, true);
        evaluate = true;
    }
    if(!model.Detect(img, net, result)){
        qDebug() << "Failed to detect object";
        pipe.stop();
        cfg.disable_all_streams();
        ev.stopWait();
        return;
    }
    model.DrawMaskSingleOnly(img, result[0], cv::Scalar(180,105,255), ui->system_folder->toolTip().toStdString() + "/" + "RGB.jpg");

    cv::Mat mask = result[0].boxMask;
    cv::imwrite(ui->system_folder->toolTip().toStdString() + "\\mask.png", mask);
    cv::Mat mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask.png");
    pose.createBackground(mask_bg, result[0].box.x, result[0].box.y, result[0].box.width, result[0].box.height, ui->system_folder->toolTip().toStdString());
    mask_bg = cv::imread(ui->system_folder->toolTip().toStdString() + "\\mask_bg.png");
    std::vector<cv::Point> points;
    bool head_up;
    double rz = pose.run(mask_bg, ui->system_folder->toolTip(), points, 0, head_up);

    if(points.size() < 2){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    //    double u = result[0].box.x + result[0].box.width/2;
    //    double v = result[0].box.y + result[0].box.height/2;
    double u = (double(points[4].x) + double(points[3].x))/2.0;
    double v = (double(points[4].y) + double(points[3].y))/2.0;

    double z = dep_frame.get_distance(int(u), int(v))*1000.0;
    double x = (u - intr.ppx)/intr.fx*z;
    double y = (v - intr.ppy)/intr.fy*z;
    qDebug() << x << y << z;
    if(z < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    double u1 = u;
    double v1 = v;
    double z1 = z;
    qDebug() << "u1-v1-z1:" << u1 << v1 << z1;
    double u2 = (double(points[3].x) + double(points[0].x))/2.0;
    double v2 = (double(points[3].y) + double(points[0].y))/2.0;
    double z2 = dep_frame.get_distance(int(u2), int(v2))*1000.0;
    if(z2 < 1.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }
    qDebug() << "u2-v2-z2:" << u2 << v2 << z2;

    double l = sqrt(pow(u1 - u2, 2.0) + pow(v1 - v2, 2.0));
    double ry = 0.0;
    //    if(z1 > z2)
    //        z1 = z1*1.002;
    //    else if(z1 < z2)
    //        z2 = z2*1.002;
    if(head_up)
        ry = atan2(z1 - z2,l)*180.0/M_PI;
    else
        ry = atan2(z2 - z1,l)*180.0/M_PI;

    if(ry > 30.0 || ry < -30.0){
        qDebug() << "Mis-detect";
        pipe.stop();
        cfg.disable_all_streams();
        return;
    }

    QFile calib_file(ui->calibration_folder->toolTip() + "/" + "Calibration.csv");
    if(!calib_file.open(QIODevice::ReadOnly)){
        return;
    }
    QString data = QString(calib_file.readAll());
    QStringList data_array = data.split(",");

    CalibrationMatrix calib_param;
    data = data_array.at(0);
    calib_param.x = data.toDouble();
    data = data_array.at(1);
    calib_param.y = data.toDouble();
    data = data_array.at(2);
    calib_param.z = data.toDouble();
    data = data_array.at(3);
    calib_param.rx = data.toDouble();
    data = data_array.at(4);
    calib_param.ry = data.toDouble();
    data = data_array.at(5);
    calib_param.rz = data.toDouble();

    // camTbase
    vpRzyxVector bRc_v(calib_param.rz*M_PI/180.0, calib_param.ry*M_PI/180.0, calib_param.rx*M_PI/180.0);
    vpRotationMatrix bRc(bRc_v);
    vpTranslationVector bTc(calib_param.x/1000.0, calib_param.y/1000.0, calib_param.z/1000.0);
    vpHomogeneousMatrix bMc(bTc, bRc);

    // objectTcam
    vpRzyxVector cRo_v(rz*M_PI/180.0, ry*M_PI/180.0, 0.0*M_PI/180.0);
    vpRotationMatrix cRo(cRo_v);
    vpTranslationVector cTo(0.0/1000.0, 0.0/1000.0, 0.0/1000.0);
    vpHomogeneousMatrix cMo(cTo, cRo);

    // objectTbase
    vpHomogeneousMatrix bMo = bMc*cMo;
    vpRzyxVector bRo_v(bMo.getRotationMatrix());
    ry = bRo_v[1]*180.0/M_PI;

    ev.getPredictedResult(0, 0, 0, 0, ry, 0);

    QPixmap screen = cvMatToQPixmap(img);
    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));
    ui->inform->setVisible(false);

    ev.stopWait();

    pipe.stop();
    cfg.disable_all_streams();

    ui->Evaluate_model->setText("Done");
    ui->Evaluate_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
}

void main_dialog::finished(int exitCode){
    if(exitCode == 0){
        proc->close();
        qDebug() << "Finished executing command";
        emit positionCalculate();
    } else qDebug() << "Failed to execute command";
}

void main_dialog::readyReadStandardOutput(){
    QStringList lines = QString::fromLocal8Bit(proc->readAllStandardOutput()).split("\r\n");
    for(QString &line : lines){
        if(line.size() == 0) continue;
        if(line.contains("[")) data_in = line;
        if(!line.contains("Detected 0")) detected = true;
        qDebug() << line;
    }
}

void main_dialog::runSriptCallback(){
    qDebug() << "Running script";
    QDir::setCurrent("D://Study//402//do_an//screw training//code");
    proc = new QProcess;
    connect(proc, SIGNAL(finished(int)), this, SLOT(finished(int)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    proc->start("python", QStringList() << "-u" << "D://Study//402//do_an//screw training//code//Call.py");
}

void main_dialog::positionCalculateCallback(){
    qDebug() << "calculate";
    cv::Rect roi(0, 0, 640, 480);
    if(detected){
        QStringList input = data_in.remove("[").remove("]").split(" ");
        x_min = QString(input.at(0)).toInt();
        y_min = QString(input.at(1)).toInt();
        x_max = QString(input.at(2)).toInt();
        y_max = QString(input.at(3)).toInt();

        float x_center = x_min + (x_max - x_min)/2;
        float y_center = y_min + (y_max - y_min)/2;
        qDebug() << x_center << y_center;

        float dist = capture_frame.get_distance((int)x_center, (int)y_center)*1000;
        qDebug() << dist;

        float fx = intr.fx;
        float fy = intr.fy;
        float ppx = intr.ppx;
        float ppy = intr.ppy;
        rs2_distortion distortion_model = intr.model;
        qDebug() << fx << fy << ppx << ppy << distortion_model << intr.coeffs;

        float x_temp = (x_center - ppx)/fx; // p136 document for offset distance of RGB module to the center
        float y_temp = (y_center - ppy)/fy;

        if(distortion_model == RS2_DISTORTION_INVERSE_BROWN_CONRADY){
            float r2 = x_temp*x_temp + y_temp*y_temp;
            float f = 1 + intr.coeffs[0]*r2 + intr.coeffs[1]*r2*r2 + intr.coeffs[4]*r2*r2*r2;
            float ux = x_temp*f + 2*intr.coeffs[2]*x_temp*y_temp + intr.coeffs[3]*(r2 + 2*x_temp*x_temp);
            float uy = y_temp*f + 2*intr.coeffs[3]*x_temp*y_temp + intr.coeffs[2]*(r2 + 2*y_temp*y_temp);
            x_temp = ux;
            y_temp = uy;
        }
        x = x_temp*dist - 35;
        y = y_temp*dist;
        qDebug() << x << y;
        z = dist;
        QString pos = "x: " + QString::number(x, 'g') + "    y: " + QString::number(y, 'g') + "    z: " + QString::number(z, 'g');
//        ui->position->setText(pos);

        roi = cv::Rect(x_min - 50, y_min - 50, (x_max - x_min) + 100, (y_max - y_min) + 100);
    }

    cv::Mat small_frame = cv::imread("D://Study//402//do_an//screw training//code//image//out.png", cv::IMREAD_COLOR);
    cv::Mat modified_frame = small_frame(roi);
    ui->screen_2->setPixmap(cvMatToQPixmap(modified_frame).scaled(ui->screen_2->width(), ui->screen_2->height()));
    ui->screen_2->setVisible(true);


    // Enable start button
    ui->start->setEnabled(true);
    ui->start->setStyleSheet("");
}

void main_dialog::yolov5_segmentation(){
    string img_path = "D://Study//401//do_an//screw training//code//image//input3.jpg";
    string model_path = "D://Study//402//Luan_van//Qt//model//yolov5-seg.onnx";
//    string img_path = "D://Study//402//Luan_van//Qt//yolov5-seg-opencv-onnxruntime-cpp//images//zidane.jpg";
//    string model_path = "D://Study//402//Luan_van//Qt//yolov5-seg-opencv-onnxruntime-cpp//models//yolov5s1-seg.onnx";
    YoloSeg test;
    Net net;
    if (test.ReadModel(net, model_path, false)) {
        if (test.ReadModel(net, model_path, true)) {
            qDebug() << "read net ok!";
        }
        else {
            return;
        }
        vector<cv::Scalar> color;
        srand(time(0));
        for (int i = 0; i < 80; i++) {
            int b = rand() % 256;
            int g = rand() % 256;
            int r = rand() % 256;
            color.push_back(cv::Scalar(b, g, r));
        }
        vector<OutputSeg> result;
        cv::Mat img = cv::imread(img_path);
        cv::Mat dst;
        cv::resize(img, dst, cv::Size(640,640), 0, 0, cv::INTER_AREA);
        clock_t t1, t2;
        if (test.Detect(dst, net, result)) {
            test.DrawPred(dst, result, color);

        }
        else {
            qDebug() << "Detect Failed!";
        }
        return;
    }
}

void main_dialog::on_select_detect_model_clicked(){
    QString dir;
    dir = QFileDialog::getOpenFileName(this, tr("Load model"), QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0), "ONNX files (*.onnx);; All files (*.*)");
    if(dir.size() == 0){
        QMessageBox::warning(this, "Load model error", "Can't find .onnx file");
        return;
    }
    if(!dir.contains("onnx")){
        QMessageBox::warning(this, "Load model error", "Selected file is not ONNX format");
        return;
    }

    QFile file("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\detect_model.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        str << dir;
        file.flush();
        file.close();

        QString model = dir.split("/").last();
        ui->detect_model->setText(model);
        ui->detect_model->setAlignment(Qt::AlignLeft);

        qDebug() << "Load model successfully";
        ui->detect_model->setToolTip(dir);
    } else {
        QMessageBox::critical(this, "Load model error", "Can't open file to backup info");
        return;
    }
}

void main_dialog::on_select_system_folder_clicked(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("System folder"));
    if(dir.size() == 0){
        QMessageBox::warning(this, "Select folder error", "Can't file folder");
        return;
    }

    QFile file("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\system_folder.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        str << dir;
        file.flush();
        file.close();

        QString model = dir.split("/").last();
        ui->system_folder->setText(model);
        ui->system_folder->setAlignment(Qt::AlignLeft);

        qDebug() << "Load model successfully";
        ui->system_folder->setToolTip(dir);
    } else {
        QMessageBox::critical(this, "Load model error", "Can't open file to backup info");
        return;
    }
}


void main_dialog::on_select_calib_folder_clicked(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Calibrate folder"));
    if(dir.size() == 0){
        QMessageBox::warning(this, "Select folder error", "Can't file folder");
        return;
    }

    QFile file("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\calib_folder.txt");
    if(file.open(QIODevice::WriteOnly)){
        QTextStream str(&file);
        str << dir;
        file.flush();
        file.close();

        QString folder = dir.split("/").last();
        ui->calibration_folder->setText(folder);
        ui->calibration_folder->setAlignment(Qt::AlignLeft);

        qDebug() << "Select folder successfully";
        ui->calibration_folder->setToolTip(dir);
    } else {
        QMessageBox::critical(this, "Select folder error", "Can't open file to backup info");
        return;
    }
}


void main_dialog::on_test_mode_currentIndexChanged(int index){
    testing_mode = index;
    qDebug() << "Testing mode:" << testing_mode;

    /*
     *  1: position (x,y)
     *  2: position (x,y,z)
     *  3: pose
     *  4: position (x,y,z) and pose
     */
}


void main_dialog::on_test_model_clicked(){
    if(ui->test_model->text() == "Test Model"){
        if(usb == usb_21 || usb == none){
            QMessageBox::warning(this, "Test model error", "Camera is not connected");
            ui->console->append("Test model error: Camera is not connected..");
            return;
        }
        if(ui->test_marker->text() == "Running"){
            QMessageBox::warning(this, "Test model error", "Test marker mode is on..");
            ui->console->append("Test model error: Test marker mode is on..");
            return;
        }
        if(ui->calibrate->text() == "Calculate"){
            QMessageBox::warning(this, "Test model error", "Calibration mode is on..");
            ui->console->append("Test model error: Calibration mode is on..");
            return;
        }
        if(ui->start->text() == "Stop"){
            QMessageBox::warning(this, "Test model error", "System is running..");
            ui->console->append("Test model error: System is running..");
            return;
        }

        ui->test_model->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");
        ui->test_model->setText("Running");
        ui->test_mode->setEnabled(false);
        ui->select_detect_model->setEnabled(false);
        ui->select_system_folder->setEnabled(false);
        ui->select_calib_folder->setEnabled(false);


        string model_path = ui->detect_model->toolTip().toStdString();
        QString temp = ui->detect_model->text().split("-").last();
        QString sz = temp.split(".").first();
        int size = sz.toInt();
        model.ChangeImageSize(size);
        qDebug() << "Model:" << ui->detect_model->text() << "--imgsz" << size;
        QString str_console = "Model: " + ui->detect_model->text() + " --imgsz " + QString::number(size);
        ui->console->append(str_console);
        if (model.ReadModel(net, model_path, true)) {
            qDebug() << "Read net successfully";
            ui->console->append("Read net successfully");
        }
        else {
            QMessageBox::critical(this, "Load model error", "Failed to load net");
            run_test = false;
            ui->test_model->setStyleSheet("");
            ui->test_model->setText("Test Model");
            ui->screen->clear();
            ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
            ui->inform->setVisible(true);
            ui->test_mode->setEnabled(true);
            ui->select_detect_model->setEnabled(true);
            ui->select_system_folder->setEnabled(true);
            ui->select_calib_folder->setEnabled(true);
            return;
        }
        srand(time(0));
        for (int i = 0; i < 1; i++) {
            int b = rand() % 256;
            int g = rand() % 256;
            int r = rand() % 256;
            color.clear();
            color.push_back(cv::Scalar(b, g, r));
        }

        switch (testing_mode) {
        case 0:
            if(ui->camera_on->text() == "Camera On"){
                cv::Mat img;
                cv::VideoCapture cap(0);

                run_test = true;
                ui->screen->clear();
                ui->inform->setVisible(false);

                auto tmp = std::chrono::steady_clock::now();
                while(run_test){
                    auto start = std::chrono::steady_clock::now();
                    cap >> img;
                    detecting = false;
                    if(model.Detect(img, net, result)){
                        model.DrawPred(img, result, color);
                        auto end = std::chrono::steady_clock::now();
                        tmp = end;
                        double inference_time = (double) (std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count()) / 1e9 * 1e3;
                        if(result.size() > 1){
                            qDebug() << "Detected" << result.size() << "Bolt M10x25 objects in" << inference_time << "ms";
                            QString str_console = "Detected " + QString::number(result.size()) + " Bolt M10x25 objects, " + QString::number(inference_time) + "ms";
                            ui->console->append(str_console);
                        } else {
                            qDebug() << "Detected" << result.size() << "Bolt M10x25 object in" << inference_time << "ms";
                            QString str_console = "Detected " + QString::number(result.size()) + " Bolt M10x25 object, " + QString::number(inference_time) + "ms";
                            ui->console->append(str_console);
                        }
                        detecting = true;
                    } else {
                        qDebug() << "No detection";
                        ui->console->append("No detection");
                    }

                    QPixmap frames = cvMatToQPixmap(img);

                    ui->screen->setPixmap(frames.scaled(ui->screen->width(), ui->screen->height()));
                    delay->delay(1);
                    auto end_convert = std::chrono::steady_clock::now();
                    double convert_time = (double) (std::chrono::duration_cast <std::chrono::nanoseconds> (end_convert - tmp).count()) / 1e9 * 1e3;
                    if(detecting)
                        qDebug() << "Conversion time:" << convert_time << "ms";
                }
            } else {
                run_test = true;
                ui->screen->clear();
                ui->inform->setVisible(false);
                connect(this, SIGNAL(testingFrameSend(cv::Mat)), this, SLOT(receiveTestingFrame(cv::Mat)));
                wait_frame = false;
            }
            break;
        case 1:
            if(!(module == align_camera && ui->camera_on->text() == "Camera Off")){
                QMessageBox::warning(this, "Camera status", "RGBD camera is off");
                run_test = false;
                ui->test_model->setStyleSheet("");
                ui->test_model->setText("Test Model");
                ui->screen->clear();
                ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
                ui->inform->setVisible(true);
                ui->test_mode->setEnabled(true);
                ui->select_detect_model->setEnabled(true);
                ui->select_system_folder->setEnabled(true);
                ui->select_calib_folder->setEnabled(true);
                return;
            }

            run_test = true;
            ui->screen->clear();
            ui->inform->setVisible(false);
            connect(this, SIGNAL(testingFrameSend(cv::Mat)), this, SLOT(receiveTestingFrame(cv::Mat)));
            wait_frame = false;

            break;
        default:
            break;
        }
    } else {
        run_test = false;
        ui->test_model->setStyleSheet("");
        ui->test_model->setText("Test Model");
        ui->screen->clear();
        ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
        ui->inform->setVisible(true);
        ui->test_mode->setEnabled(true);
        ui->select_detect_model->setEnabled(true);
        ui->select_system_folder->setEnabled(true);
        ui->select_calib_folder->setEnabled(true);
        if(ui->camera_on->text() == "Camera Off") ui->fps->setText("30");

    }

}

void main_dialog::receiveTestingFrame(cv::Mat frame){
    auto start = std::chrono::steady_clock::now();
    double inference_time;
//    qDebug() << frame.rows << frame.cols;
    if(model.Detect(frame, net, result)){
        model.DrawPred(frame, result, color);
        string label;
        int baseLine;
        if(testing_mode == 1){
            for(int i = 0; i < result.size(); i++){
                struct Point3D p = intrinsicsCalculation(capture_frame, i);
                label = "(" + std::to_string(p.x) + "," + std::to_string(p.y) + "," + std::to_string(p.z) + ")";
                qDebug() << p.x << p.y << p.z;
                cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, 1, &baseLine);
                int top = cv::max(result[i].box.y, labelSize.height);
                cv::rectangle(frame, cv::Point(result[i].box.x - 1, top - 13 - 2 - labelSize.height - 2), cv::Point(result[i].box.x - 1 + 2 + labelSize.width + 2, top - 13), color[result[i].id], cv::FILLED);
                cv::putText(frame, label, cv::Point(result[i].box.x - 1 + 2, top - 13 - 2), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cv::Scalar(255, 255, 255), 1);
            }
        }
        auto end = std::chrono::steady_clock::now();
        inference_time = (double) (std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count()) / 1e9 * 1e3;
        if(result.size() > 1){
            qDebug() << "Detected" << result.size() << "Bolt M10x25 objects in" << inference_time << "ms";
            QString str_console = "Detected " + QString::number(result.size()) + " Bolt M10x25 objects, " + QString::number(inference_time) + "ms";
            ui->console->append(str_console);
            qDebug() << result[0].box.height << result[0].box.width << result[0].box.x << result[0].box.y;
        } else {
            qDebug() << "Detected" << result.size() << "Bolt M10x25 object in" << inference_time << "ms";
            QString str_console = "Detected " + QString::number(result.size()) + " Bolt M10x25 object, " + QString::number(inference_time) + "ms";
            ui->console->append(str_console);
            qDebug() << result[0].box.height << result[0].box.width << result[0].box.x << result[0].box.y;
        }
        ui->fps->setText(QString::number((1/inference_time)*1000));
    } else {
        auto end = std::chrono::steady_clock::now();
        inference_time = (double) (std::chrono::duration_cast <std::chrono::nanoseconds> (end - start).count()) / 1e9 * 1e3;
        qDebug() << "No detection";
        ui->console->append("No detection");
        ui->fps->setText(QString::number((1/inference_time)*1000));
    }

    QPixmap frames = cvMatToQPixmap(frame);

    ui->screen->clear();
    ui->screen->setPixmap(frames.scaled(ui->screen->width(), ui->screen->height()));

    delay->delay(1);
    wait_frame = false;
    cam->wait_frame = false;
}

struct Point3D main_dialog::intrinsicsCalculation(rs2::depth_frame frame, int idx){
    double u = result[idx].box.x + result[idx].box.width/2;
    double v = result[idx].box.y + result[idx].box.height/2;
    qDebug() << u << v;

    double x = (u - intr.ppx)/intr.fx;
    double y = (v - intr.ppy)/intr.fy;
    double z = frame.get_distance(int(u), int(v))*1000.0;
    qDebug() << x << y << z;

//    if(intr.model == RS2_DISTORTION_INVERSE_BROWN_CONRADY){
//        qDebug() << "Distorted";
//        float r2 = x*x + y*y;
//        float f = 1 + intr.coeffs[0]*r2 + intr.coeffs[1]*r2*r2 + intr.coeffs[4]*r2*r2*r2;
//        float ux = x*f + 2*intr.coeffs[2]*x*y + intr.coeffs[3]*(r2 + 2*x*x);
//        float uy = y*f + 2*intr.coeffs[3]*x*y + intr.coeffs[2]*(r2 + 2*y*y);
//        x = ux;
//        y = uy;
//        qDebug() << x << y << z;
//    }

    struct Point3D p;
//    p.x = x*z-offset;
    p.x = x*z;
    p.y = y*z;
    p.z = z;
    return p;
}

void main_dialog::checkUSBSpeed(libusb_device **devs){
    libusb_device *dev;
    int i = 0, j = 0;
    uint8_t path[8];

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");
            return;
        }
        uint16_t VendorID = 0x8086;
        uint16_t ProductID = 0x0AD3;

        if(desc.idVendor == VendorID){
//            printf("%04x:%04x (bus %d, device %d)\n", desc.idVendor, desc.idProduct, libusb_get_bus_number(dev), libusb_get_device_address(dev));
//            qDebug() << libusb_get_device_speed(dev);
            if(libusb_get_device_speed(dev) == 3) usb = usb_21;
            else if(libusb_get_device_speed(dev) == 4) usb = usb_31;
            break;
        } else usb = none;
    }
}

void main_dialog::checkUSB(){
    libusb_device **devs;
    int r;
    ssize_t cnt;

    r = libusb_init(NULL);
    if (r < 0)
        return;

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0){
        libusb_exit(NULL);
        return;
    }

    checkUSBSpeed(devs);
    libusb_free_device_list(devs, 1);

    libusb_exit(NULL);

    if(usb == none){
        ui->usb_type->clear();
        ui->usb->clear();
        ui->camera_name->setText("Intel Realsense");
    } else if(usb == usb_21){
        QPixmap img(":/image/usb21.png");
        ui->usb_type->setPixmap(img.scaled(ui->usb_type->width(), ui->usb_type->height()));
        ui->usb->setText("2.1");
        ui->usb->setStyleSheet("color: rgb(147, 164, 179);");
    } else if(usb == usb_31){
        QPixmap img(":/image/usb31.png");
        ui->usb_type->setPixmap(img.scaled(ui->usb_type->width(), ui->usb_type->height()));
        ui->usb->setText("3.1");
        ui->usb->setStyleSheet("color: rgb(147, 179, 149);");
    }

    return;
}

void main_dialog::on_main_dialog_rejected(){
    usb_timer->stop();
    delete usb_timer;
}

void main_dialog::on_calibrate_clicked(){
    if(ui->calibrate->text() == "Calibrate Camera"){
        if(ui->test_marker->text() == "Running"){
            QMessageBox::warning(this, "Calibrate error", "Test marker mode is on..");
            ui->console->append("Calibrate error: Test marker mode is on..");
            return;
        }
        if(ui->camera_on->text() == "Camera Off"){
            QMessageBox::warning(this, "Calibrate error", "Camera is turn on..");
            ui->console->append("Calibrate error: Camera is turn on..");
            return;
        }
        if(ui->start->text() == "Stop"){
            QMessageBox::warning(this, "Calibrate error", "System is running..");
            ui->console->append("Camera error: System is running..");
            return;
        }
        if(usb == usb_21 || usb == none){
            QMessageBox::warning(this, "Calibrate error", "Camera is not connected");
            ui->console->append("Calibrate error: Camera is not connected..");
            return;
        }

        ui->get_pose->setVisible(false);

        cancel_calib = false;
        QString console_data;
        calib_progress = new QProgressDialog("Initialize system", "Cancel", 0, 100);
        calib_progress->setWindowTitle("Camera calibration");
        calib_progress->setMaximumSize(250,92);
        calib_progress->setMinimumSize(250,92);
        calib_progress->setVisible(true);
        connect(calib_progress, SIGNAL(canceled()), this, SLOT(cancelCalib()));


        QTextStream str;
        if(!connect_status){
            QFile ip_file("C:\\Autoss\\Draft\\Robot_info\\ip.txt");
            if(!ip_file.open(QIODevice::ReadOnly)){
                QMessageBox::warning(this, "Calibrate error", " Failed to initialize!");
                return;
            }
            str.setDevice(&ip_file);
            QString ip = str.readAll();
            qDebug() << ip;
            ip_file.flush();
            ip_file.close();

            QHostAddress udp_address;
            quint16 udp_port = 10040;
            quint16 file_port = 10041;
            udp_address.setAddress(QHostAddress(ip).toIPv4Address());
            robot.motomanSetConnection(udp_address, udp_port, file_port);
            robot.motomanConnect();
            connect_status = true;
        }

        if(cancel_calib){
            cancel_calib = false;
            return;
        }

        QFile calib_folder("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\calib_folder.txt");
        if(!calib_folder.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this, "Calibrate error", " Failed to initialize!");
            return;
        }
        str.setDevice(&calib_folder);
        QString calib_path = str.readAll();
        calib_folder.flush();
        calib_folder.close();

        if(cancel_calib){
            cancel_calib = false;
            return;
        }

        read_pos = false;
        read_joint = false;
        connect(&robot, SIGNAL(dataRecieveUISignal(quint8,bool,QStringList)), this, SLOT(updateReceiveUICallback(quint8,bool,QStringList)));
        delay->delay(100);

        if(cancel_calib){
            cancel_calib = false;
            return;
        }

        // Turn servo on
//        robot.motomanOnServo();
//        delay->delay(1000);
        calib_progress->setValue(20);

        // Initialize camera
        ui->console->append("Initialize camera..");
        cfg.disable_all_streams();

        string serial;
        if (!device_with_streams({RS2_STREAM_COLOR}, serial))
            return;
        cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, frame_rate_color);
        auto profile = pipe.start(cfg);
        intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();
        for(int h = 0; h < 50; h++){
            frames = pipe.wait_for_frames();
            delay->delay(20);
        }
        calib_progress->setValue(40);

        if(cancel_calib){
            robot.motomanOffServo();
            pipe.stop();
            cfg.disable_all_streams();
            cancel_calib = false;
            return;
        }

        std::vector<vpHomogeneousMatrix> cMo(N); // camera to object transformation
        std::vector<vpHomogeneousMatrix> wMe(N); // world to hand (end-effector) transformation
        vpHomogeneousMatrix bMc; // base to eye (camera) transformation

        // Initialize bMc
        bMc.buildFrom(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        calib_progress->setValue(60);

        if(cancel_calib){
            robot.motomanOffServo();
            pipe.stop();
            cfg.disable_all_streams();
            calib_progress->cancel();
            cancel_calib = false;
            return;
        }

        // Initialize paramaters
        ui->console->append("Initialize parameters..");
        QString param_file_path = calib_path + "/" + "parameters.json";
        qDebug() << param_file_path;
        QFile param_file(param_file_path);
        if(!param_file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::warning(this, "Calibrate error", "Failed to initialize robot parameters");
            return;
        }
        QString data = param_file.readAll();
        param_file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        QJsonObject file = doc.object();
        DH_table calib_param = readParameters(file, Calibrate);
        DH_table robot_param = readParameters(file, PickAndPlace);
        calib_progress->setValue(80);

        if(cancel_calib){
            robot.motomanOffServo();
            pipe.stop();
            cfg.disable_all_streams();
            calib_progress->cancel();
            cancel_calib = false;
            return;
        }

        // Initialize log file
        QDateTime time = QDateTime::currentDateTime();
        QDir folder;
        folder.mkdir(calib_path + "/" + time.toString("dd_MM_yy_hh_mm_ss"));
        folder.setPath(calib_path + "/" + time.toString("dd_MM_yy_hh_mm_ss"));
        QFile bTo_file(folder.path() + "/base2object.csv");
        QTextStream bTo_str;
        if(!bTo_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
            qDebug() << "Failed to open log text";
            ui->console->append("Failed to open base2cam log text");
        } else
            bTo_str.setDevice(&bTo_file);

        QFile cTo_file(folder.path() + "/cam2object.csv");
        QTextStream cTo_str;
        if(!cTo_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
            qDebug() << "Failed to open log text";
            ui->console->append("Failed to open base2cam log text");
        } else
            cTo_str.setDevice(&cTo_file);

        QString bTc_file(folder.path() + "/base2cam.csv");

        // Initialize image folder
        QString img_dir = calib_path + "/" + "Img";
        QDir dir(img_dir);
        if(!dir.exists()){
             dir.mkdir(img_dir);
        }

        // Initialize for marker pose detection
        cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
        cameraMatrix.at<double>(0,0) = intr.fx;
        cameraMatrix.at<double>(0,2) = intr.ppx;
        cameraMatrix.at<double>(1,1) = intr.fy;
        cameraMatrix.at<double>(1,2) = intr.ppy;


        vector<double> distCoeffs(5);
        for(int i = 0; i < 5; i++)
            distCoeffs[i] = intr.coeffs[i];

        float markerLength = 0.066;
        // Set coordinate system
        cv::Mat objPoints(4, 1, CV_32FC3);
        objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerLength/2.f, markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerLength/2.f, markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerLength/2.f, -markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerLength/2.f, -markerLength/2.f, 0);
        calib_progress->setValue(100);
        delay->delay(100);

        if(cancel_calib){
            pipe.stop();
            cfg.disable_all_streams();
            bTo_file.flush();
            bTo_file.close();
            cTo_file.flush();
            cTo_file.close();
            return;
        }

        calib_progress = new QProgressDialog("Retrieving Pose..", "Cancel", 0, 100);
        calib_progress->setWindowTitle("Camera calibration");
        calib_progress->setMaximumSize(250,92);
        calib_progress->setMinimumSize(250,92);
        calib_progress->setVisible(true);
        connect(calib_progress, SIGNAL(canceled()), this, SLOT(cancelCalib()));

        // Pose of marker with respect to camera
        cv::Vec3d oRc, oTc;

        bool detect_marker_pose = false;
        get_pose = false;
        ui->calibrate->setVisible(false);
        ui->calibrate->setText("Calculate");
        ui->get_pose->setVisible(true);

        int pose_cal = 1;
        while(pose_cal <= N){
            if(get_pose){
                read_pos = true;
                Cartesian org, robot_pos;
                robot.motomanReadPosition();
                while(1){
                    if(!read_pos){
                        robot_pos.x = robot_position[0];
                        robot_pos.y = robot_position[1];
                        robot_pos.z = robot_position[2];
                        robot_pos.rx = robot_position[3];
                        robot_pos.ry = robot_position[4];
                        robot_pos.rz = robot_position[5];
                        break;
                    }
                    delay2->delay(25);
                }
                if(pose_cal == 1){
                    org.x = robot_position[0];
                    org.y = robot_position[1];
                    org.z = robot_position[2];
                    org.rx = robot_position[3];
                    org.ry = robot_position[4];
                    org.rz = robot_position[5];
                }

                frames = pipe.wait_for_frames();
                frame color_frame = frames.get_color_frame().apply_filter(color_map);
                cv::Mat color(cv::Size(stream_width, stream_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

                if(cancel_calib){
                    calib->registerInput(cMo, wMe, pose_cal - 1, bTc_file);
                    ui->calibrate->setVisible(true);
                    ui->get_pose->setVisible(false);

                    pipe.stop();
                    cfg.disable_all_streams();
                    bTo_file.flush();
                    bTo_file.close();
                    cTo_file.flush();
                    cTo_file.close();
                    cancel_calib = false;
                    calib_progress->cancel();
                    return;
                }

                detect_marker_pose = detectPoseMarker(color, cameraMatrix, distCoeffs, objPoints, oRc, oTc);

//                if(oRc[0] < 0.0)
//                    oRc[0] = oRc[0]*(-1.0);

//                if(pose_cal > 1 && (abs(robot_pos.rx - org.rx) > 1.0 || abs(robot_pos.ry - org.ry) > 1.0 || abs(robot_pos.rz - org.rz) > 1.0) && oRc[1] < 0.0)
//                    oRc[1] = oRc[1]*(-1.0);
//                else if(pose_cal > 1 && abs(robot_pos.rz - org.rz) > 1.0 && oRc[1] > 0.0)
//                    oRc[1] = oRc[1]*(-1.0);

//                if(pose_cal > 1 && (abs(robot_pos.rx - org.rx) > 1.0 || abs(robot_pos.ry - org.ry) > 1.0 || abs(robot_pos.rz - org.rz) > 1.0) && robot_pos.ry < org.ry){
//                    if(oRc[2] < 0.0)
//                        oRc[2] = oRc[2]*(-1.0);
//                } else if(pose_cal > 1 && (abs(robot_pos.rx - org.rx) > 1.0 || abs(robot_pos.ry - org.ry) > 1.0 || abs(robot_pos.rz - org.rz) > 1.0) && robot_pos.ry >= org.ry){
//                    if(oRc[2] > 0.0)
//                        oRc[2] = oRc[2]*(-1.0);
//                }

                if(!detect_marker_pose){
                    qDebug() << "\nCan't detect";
                    ui->console->append("\n\nFailed to detect marker pose\nCalculating next pose..\n");
                } else{
                    QPixmap screen = cvMatToQPixmap(color);
                    ui->inform->setVisible(false);
                    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));

                    cv::imwrite(folder.path().toStdString() + "/" + std::to_string(pose_cal) + ".jpg", color);

                    vpRzyxVector rpy_v(vpMath::rad(robot_pos.rz), vpMath::rad(robot_pos.ry), vpMath::rad(robot_pos.rx));
                    vpRotationMatrix rpy(rpy_v);
                    vpTranslationVector t_v(robot_pos.x/1000.0, robot_pos.y/1000.0, robot_pos.z/1000.0);
                    wMe.at(pose_cal - 1).buildFrom(t_v, rpy);

                    rpy_v.buildFrom(vpMath::rad(oRc[2]), vpMath::rad(oRc[1]), vpMath::rad(oRc[0]));
                    rpy.buildFrom(rpy_v);
                    t_v.buildFrom(oTc[0]/1000.0, oTc[1]/1000.0, oTc[2]/1000.0);
                    cMo.at(pose_cal - 1).buildFrom(t_v, rpy);

                    qDebug() << "\nPose" << pose_cal;

                    qDebug() << "Robot pose:" << robot_pos.x << "," << robot_pos.y << "," << robot_pos.z << "," << robot_pos.rx << "," << robot_pos.ry << "," << robot_pos.rz;
                    std::cout << wMe.at(pose_cal - 1) << std::endl;
                    delay->delay(50);
                    bTo_str << robot_pos.x << "," << robot_pos.y << "," << robot_pos.z << "," << robot_pos.rx << "," << robot_pos.ry << "," << robot_pos.rz << "\n";

                    qDebug() << "Camera pose:" << oTc[0] << "," << oTc[1] << "," << oTc[2] << "," << oRc[0] << "," << oRc[1] << "," << oRc[2];
                    std::cout << cMo.at(pose_cal - 1) << std::endl;
                    delay->delay(50);
                    cTo_str << oTc[0] << "," << oTc[1] << "," << oTc[2] << "," << oRc[0] << "," << oRc[1] << "," << oRc[2] << "\n";

                    delay->delay(50);
                    calib_progress->setValue(int(double(pose_cal)/double(N)*100.0));

                    pose_cal = pose_cal + 1;
                }
                delay->delay(25);
                get_pose = false;
            }
            delay->delay(20);
        }
        bTo_str << "\n";
        cTo_str << "\n";
        bTo_file.flush();
        bTo_file.close();
        cTo_file.flush();
        cTo_file.close();

        calib->registerInput(cMo, wMe, N, bTc_file);
        ui->calibrate->setVisible(true);
        ui->get_pose->setVisible(false);

        pipe.stop();
        cfg.disable_all_streams();
        ui->screen->clear();
        ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
        ui->inform->setVisible(true);

        bTo_file.flush();
        bTo_file.close();
        cTo_file.flush();
        cTo_file.close();

        return;
    }

    if(ui->calibrate->text() == "Calculate"){
        calib->calibrateLeastSquares();
        delay->delay(50);
        calib->start(QThread::HighPriority);
        ui->calibrate->setText("Calibrate Camera");
        while(!calib->isFinished()){
            if(cancel_calib){
                calib->quit();
                return;
            }
            delay->delay(25);
        }
    }
}

void main_dialog::updateReceiveUICallback(quint8 control_mode, bool general_response, QStringList job_list_received){
    if(read_pos){
        robot_position = robot.motomanUpdatePosition();
        read_pos = false;
    }

    if(read_joint){
        robot_joint = robot.motomanUpdateDegree();
        read_joint = false;
    }

    if(read_status){
//        qDebug() << "Call";
        quint8 running_status = robot.motomanUpdateStatus();
        if(running_status)
            status = false;
        else
            status = true;
//        qDebug() << "Status:" << status;
        read_status = false;
    }
}

DH_table main_dialog::readParameters(QJsonObject file, mode Mode){
    DH_table param;
    QJsonObject DH = file.value("DH table").toObject();
    QJsonObject robot = file.value("Robot").toObject();
    QJsonObject calib = file.value("Calibration").toObject();

    param.a1 = DH.value("a1").toDouble();
    param.a2 = DH.value("a2").toDouble();
    param.a3 = DH.value("a3").toDouble();
    param.d4 = DH.value("d4").toDouble();
    param.d6 = DH.value("d6").toDouble();
    param.alpha[0] = DH.value("alpha1").toDouble();
    param.alpha[1] = DH.value("alpha2").toDouble();
    param.alpha[2] = DH.value("alpha3").toDouble();
    param.alpha[3] = DH.value("alpha4").toDouble();
    param.alpha[4] = DH.value("alpha5").toDouble();
    param.alpha[5] = DH.value("alpha6").toDouble();
    param.theta[0] = DH.value("theta1").toDouble();
    param.theta[1] = DH.value("theta2").toDouble();
    param.theta[2] = DH.value("theta3").toDouble();
    param.theta[3] = DH.value("theta4").toDouble();
    param.theta[4] = DH.value("theta5").toDouble();
    param.theta[5] = DH.value("theta6").toDouble();

    if(Mode == PickAndPlace){
        param.tool_length = robot.value("tool length").toDouble();
        param.rx = robot.value("Rx").toDouble();
        param.ry = robot.value("Ry").toDouble();
        param.rz = robot.value("Rz").toDouble();
        param.rx_er = robot.value("Rx error").toDouble();
        param.ry_er = robot.value("Ry error").toDouble();
        param.rz_er = robot.value("Rz error").toDouble();
    } else if(Mode == Calibrate){
        param.tool_length = calib.value("tool length").toDouble();
        param.rx = calib.value("Rx").toDouble();
        param.ry = calib.value("Ry").toDouble();
        param.rz = calib.value("Rz").toDouble();
        param.rx_er = calib.value("Rx error").toDouble();
        param.ry_er = calib.value("Ry error").toDouble();
        param.rz_er = calib.value("Rz error").toDouble();
    }

    return param;
}

bool main_dialog::detectPoseMarker(cv::Mat &img, cv::Mat cameraMatrix, vector<double> distCoeffs, cv::Mat objPoints, cv::Vec3d &oRc, cv::Vec3d &oTc){
    cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
    cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_250);
    cv::aruco::ArucoDetector detector(dictionary, detectorParams);

    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners;
    detector.detectMarkers(img, corners, ids);

    if (ids.size() > 0){
        qDebug() << "id size:" << ids.size();
//        cv::aruco::drawDetectedMarkers(img, corners, ids);

        int nMarkers = corners.size();
        std::vector<cv::Vec3d> rvecs(nMarkers), tvecs(nMarkers);

        // Calculate pose for each marker
        for (int i = 0; i < nMarkers; i++)
            cv::solvePnP(objPoints, corners.at(i), cameraMatrix, distCoeffs, rvecs.at(i), tvecs.at(i));

        for (int i = 0; i < nMarkers; i++)
            cv::drawFrameAxes(img, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.025);

        cv::Matx33d rvecs_new;
        cv::Rodrigues(rvecs, rvecs_new);

        double temp = pow(rvecs_new(2,1), 2.0) + pow(rvecs_new(2,2), 2.0);
        double Rx, Ry, Rz;
        if(temp != 0){
            Rx = atan2(rvecs_new(2,1),rvecs_new(2,2))*180.0/3.14;
            Ry = atan2(-rvecs_new(2,0),sqrt(temp))*180.0/M_PI;
            Rz = atan2(rvecs_new(1,0),rvecs_new(0,0))*180.0/M_PI;
        } else if(temp == 0){
            if(rvecs_new(2,0) == (double)-1){
                Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
                Ry = 90.0;
                Rz = 0;
            } else if(rvecs_new(2,0) == (double)1){
                Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
                Ry = -90.0;
                Rz = 0;
            }
        }

        for(unsigned int i = 0; i < ids.size(); i++){
            tvecs.at(i) = tvecs.at(i)*1000.0;
        }

        cv::Vec3d rvec_new(Rx, Ry, Rz);
        oRc = rvec_new;
        oTc = tvecs.at(0);

        return true;
    } else
        return false;
}

void main_dialog::on_test_calib_clicked(){
    QString folder = "D:\\Study\\402\\Luan_van\\Calibration\\System\\27_03_23_15_31_46";
//    int N = 60;
    std::vector<vpHomogeneousMatrix> cMo, bMe;
    vpHomogeneousMatrix temp;
    QFile object2cam_f(folder + "\\cam2object.csv");
    int count = 1;
    if(object2cam_f.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream str(&object2cam_f);
        QString data;
        while(!str.atEnd()){
            data = str.readLine();
            if(!data.contains(","))
                break;
//            qDebug() << data;
        double x, y, z, rx, ry, rz;
        QStringList data_v = data.split(",");
        data = data_v.at(0);
        x = data.toDouble();
        data = data_v.at(1);
        y = data.toDouble();
        data = data_v.at(2);
        z = data.toDouble();
        data = data_v.at(3);
        rx = data.toDouble();
        data = data_v.at(4);
        ry = data.toDouble();
        data = data_v.at(5);
        rz = data.toDouble();
//        qDebug() << x << y << z << rx << ry << rz;

        vpRzyxVector rpy_v(vpMath::rad(rz), vpMath::rad(ry), vpMath::rad(rx));
        vpRotationMatrix rpy(rpy_v);
        vpTranslationVector t_v(x/1000.0, y/1000.0, z/1000.0);
        temp.buildFrom(t_v, rpy);
        cMo.push_back(temp);
        count = count + 1;
        }
        qDebug() << "End of file:" << count - 1;
        object2cam_f.close();

        QFile gripper2base_f(folder + "\\base2object.csv");
        count = 1;
        if(gripper2base_f.open(QIODevice::ReadOnly|QIODevice::Text)){
            str.setDevice(&gripper2base_f);
            QString data;
            while(!str.atEnd()){
                data = str.readLine();
                if(!data.contains(","))
                    break;
    //            qDebug() << data;
            double x, y, z, rx, ry, rz;
            QStringList data_v = data.split(",");
            data = data_v.at(0);
            x = data.toDouble();
            data = data_v.at(1);
            y = data.toDouble();
            data = data_v.at(2);
            z = data.toDouble();
            data = data_v.at(3);
            rx = data.toDouble();
            data = data_v.at(4);
            ry = data.toDouble();
            data = data_v.at(5);
            rz = data.toDouble();
    //        qDebug() << x << y << z << rx << ry << rz;

            vpRzyxVector rpy_v(vpMath::rad(rz), vpMath::rad(ry), vpMath::rad(rx));
            vpRotationMatrix rpy(rpy_v);
            vpTranslationVector t_v(x/1000.0, y/1000.0, z/1000.0);
            temp.buildFrom(t_v, rpy);
            bMe.push_back(temp);
            count = count + 1;
            }
            qDebug() << "End of file:" << count - 1;
            gripper2base_f.close();

            calib->registerInput(cMo, bMe, count - 1, "none");
            calib->start(QThread::HighPriority);
            while(!calib->isFinished()){
                if(cancel_calib){
                    calib->quit();
                    qDebug() << "Exit";
                    return;
                }
                delay->delay(25);
            }
            qDebug() << "Finished";
        }
    }

//    vpRzyxVector b;
//    b.buildFrom(vpMath::rad(50.0),vpMath::rad(40.0),vpMath::rad(30.0));
//    vpRotationMatrix c;
//    c.buildFrom(b);
//    std::cout << "rxyz: " << c << std::endl;
//    vpHomogeneousMatrix a;
//    a.buildFrom(0.5, 0.3, 0.1, 30.0, 40.0, 50.0);
//    std::cout << "visp: " << a << std::endl;
//    qDebug() << "Robot" << robot.motomanTransformationMatrix(500.0, 300.0, 100.0, 30.0, 40.0, 50.0);

//    QFile calib_folder("C:\\Autoss\\Draft\\Plugins\\PicknPlace\\calib_folder.txt");
//    if(!calib_folder.open(QIODevice::ReadOnly)){
//        QMessageBox::warning(this, "Calibrate error", " Failed to initialize!");
//        return;
//    }
//    QTextStream str(&calib_folder);
//    QString calib_path = str.readAll();
//    calib_folder.flush();
//    calib_folder.close();

//    QString param_file_path = calib_path + "/" + "parameters.json";
//    qDebug() << param_file_path;
//    QFile param_file(param_file_path);
//    if(!param_file.open(QIODevice::ReadOnly | QIODevice::Text)){
//        QMessageBox::warning(this, "Calibrate error", "Failed to initialize robot parameters");
//        return;
//    }
//    QString data = param_file.readAll();
//    param_file.close();
//    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
//    QJsonObject file = doc.object();
//    DH_table calib_param = readParameters(file, Calibrate);
//    DH_table robot_param = readParameters(file, PickAndPlace);

//    QHostAddress udp_address;
//    quint16 udp_port = 10040;
//    quint16 file_port = 10041;
//    udp_address.setAddress(QHostAddress("192.168.1.14").toIPv4Address());
//    robot.motomanSetConnection(udp_address, udp_port, file_port);
//    robot.motomanConnect();
//    connect_status = true;
//    delay->delay(50);
//    robot.motomanReadPosition();
//    delay->delay(100);
//    QVector<double> pos = robot.motomanUpdatePosition();
//    qDebug() << pos[0] << pos[1] << pos[2] << pos[3] << pos[4] << pos[5];

//    delay->delay(50);
//    robot.motomanReadPulse();
//    delay->delay(100);
//    QVector<double> deg = robot.motomanUpdateDegree();
//    Joint input;
//    for(int i = 0; i < 6; i++)
//        input.theta[i] = deg[i];
//    qDebug() << input.theta[0] << input.theta[1] << input.theta[2] << input.theta[3] << input.theta[4] << input.theta[5];

//    Cartesian pos_in;
//    pos_in.x = pos[0];
//    pos_in.y = pos[1];
//    pos_in.z = pos[2];
//    pos_in.rx = pos[3];
//    pos_in.ry = pos[4];
//    pos_in.rz = pos[5];
//    Joint joint_out = robot.motomanInverseKinematics(pos_in, calib_param, input);
//    qDebug() << joint_out.theta[0] << joint_out.theta[1] << joint_out.theta[2] << joint_out.theta[3] << joint_out.theta[4] << joint_out.theta[5];

//    Cartesian result = robot.motomanForwardKinematics(input, calib_param);
//    qDebug() << result.x << result.y << result.z << result.rx << result.ry << result.rz;


//    std::vector<vpHomogeneousMatrix> A(2);
//    vpHomogeneousMatrix A1, A2;
//    A.at(0).buildFrom(1.0,2.0,3.0,4.0,5.0,6.0);
//    A.at(1).buildFrom(2.0,3.0,4.0,5.0,6.0,7.0);

//    const vpRotationMatrix r1 = A.at(0).getRotationMatrix();
//    const vpTranslationVector t1 = A.at(0).getTranslationVector();
//    std::cout << r1 << std::endl;
//    std::cout << t1 << std::endl;

//    A.at(0).inverse(A1);
//    const vpRotationMatrix r2 = A1.getRotationMatrix();
//    const vpTranslationVector t2 = A1.getTranslationVector();
//    std::cout << r2 << "\n";
//    std::cout << t2 << "\n\n";

//    A.at(0) = A1;
//    const vpRotationMatrix r3 = A.at(0).getRotationMatrix();
//    const vpTranslationVector t3 = A.at(0).getTranslationVector();

//    std::cout << "Result:" << std::endl;
//    std::cout << A.at(0).getRotationMatrix() << std::endl << A.at(0).getTranslationVector() << std::endl;
}

void main_dialog::finishCalibCallback(bool finished, int ret){
    finishCalib = finished;
    error = ret;
//    qDebug() << "ret" << ret << error;
}

void main_dialog::cancelCalib(){
    cancel_calib = true;
}

void main_dialog::mouseMove(int x, int y){
    qDebug() << "Mouse:" << x << y;
}

void main_dialog::mouseClicked(){
    int x, y;
    ui->screen_interactor->getMousePosition(x, y);
    qDebug() << "Mouse clicked:" << x << y;
}

void main_dialog::on_test_marker_clicked(){
    if(ui->test_marker->text() == "Test Marker"){
        if(ui->camera_on->text() == "Camera Off"){
            QMessageBox::warning(this, "Test marker error", "Camera is turn on..");
            ui->console->append("Test marker error: Camera is turn on..");
            return;
        }
        if(ui->calibrate->text() == "Calculate"){
            QMessageBox::warning(this, "Test marker error", "Calibration mode is on..");
            ui->console->append("Test marker error: Calibration mode is on..");
            return;
        }
        if(ui->start->text() == "Stop"){
            QMessageBox::warning(this, "Test marker error", "System is running..");
            ui->console->append("Camera error: System is running..");
            return;
        }
        if(usb == usb_21 || usb == none){
            QMessageBox::warning(this, "Test marker error", "Camera is not connected");
            ui->console->append("Test marker error: Camera is not connected..");
            return;
        }

        ui->test_marker->setText("Running");
        ui->test_marker->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(118, 255, 173, 255));");

        string serial;
        !device_with_streams({RS2_STREAM_COLOR}, serial);
        if (!serial.empty())
            cfg.enable_device(serial);
        cfg.enable_stream(RS2_STREAM_COLOR, stream_width, stream_height, RS2_FORMAT_BGR8, 30);

        auto profile = pipe.start(cfg);
        intr = profile.get_stream(RS2_STREAM_COLOR).as<video_stream_profile>().get_intrinsics();
        pipe.stop();
        cfg.disable_all_streams();

        cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
        cameraMatrix.at<double>(0,0) = intr.fx;
        cameraMatrix.at<double>(0,2) = intr.ppx;
        cameraMatrix.at<double>(1,1) = intr.fy;
        cameraMatrix.at<double>(1,2) = intr.ppy;

        vector<double> distCoeffs(5);
        for(int i = 0; i < 5; i++)
            distCoeffs[i] = intr.coeffs[i];

        float markerLength = 0.066;
        // Set coordinate system
        cv::Mat objPoints(4, 1, CV_32FC3);
        objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerLength/2.f, markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerLength/2.f, markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerLength/2.f, -markerLength/2.f, 0);
        objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerLength/2.f, -markerLength/2.f, 0);

        cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
        cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_250);
        cv::aruco::ArucoDetector detector(dictionary, detectorParams);

        cv::VideoCapture vid(2);
        cv::Mat img;

        ui->inform->setVisible(false);
        test_marker = true;
        if(vid.isOpened()){
            while(1){
                vid >> img;

                std::vector<int> ids;
                std::vector<std::vector<cv::Point2f>> corners;
                detector.detectMarkers(img, corners, ids);

                // If at least one marker detected
                if (ids.size() > 0){
                    cv::aruco::drawDetectedMarkers(img, corners, ids);

                    int nMarkers = corners.size();
                    std::vector<cv::Vec3d> rvecs(nMarkers), tvecs(nMarkers);

                    // Calculate pose for each marker
                    for (int i = 0; i < nMarkers; i++)
                        cv::solvePnP(objPoints, corners.at(i), cameraMatrix, distCoeffs, rvecs.at(i), tvecs.at(i));

                    // Draw axis for each marker
                    for(unsigned int i = 0; i < ids.size(); i++)
                        cv::drawFrameAxes(img, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.025);

                    QPixmap screen = cvMatToQPixmap(img);
                    ui->screen->setPixmap(screen.scaled(ui->screen->width(), ui->screen->height()));

                    cv::Matx33d rvecs_new;
                    cv::Rodrigues(rvecs, rvecs_new);

                    double temp = pow(rvecs_new(2,1), 2.0) + pow(rvecs_new(2,2), 2.0);
                    double Rx, Ry, Rz;
                    if(temp != 0){
                        Rx = atan2(rvecs_new(2,1),rvecs_new(2,2))*180.0/3.14;
                        Ry = atan2(-rvecs_new(2,0),sqrt(temp))*180.0/M_PI;
                        Rz = atan2(rvecs_new(1,0),rvecs_new(0,0))*180.0/M_PI;
                    } else if(temp == 0){
                        if(rvecs_new(2,0) == (double)-1){
                            Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
                            Ry = 90.0;
                            Rz = 0;
                        } else if(rvecs_new(2,0) == (double)1){
                            Rx = atan2(rvecs_new(0,1),rvecs_new(1,1))*180.0/M_PI;
                            Ry = -90.0;
                            Rz = 0;
                        }
                    }

                    for(unsigned int i = 0; i < ids.size(); i++){
                        tvecs.at(i) = tvecs.at(i)*1000.0;
//                        rvecs.at(i) = (rvecs.at(i)/M_PI)*180.0;
                    }

                    for(unsigned int i = 0; i < ids.size(); i++){
                        std::cout << "Marker " << std::to_string(i+1) << std::endl << "Rotation: " << Rx << ", " << Ry << ", " << Rz << std::endl << "Translation: " << tvecs[i] << std::endl;
                        QString marker_info = "Marker ID " + QString::number(ids[i]) + ": [" + QString::number(tvecs[i][0]) + "  " + QString::number(tvecs[i][1]) + "  " + QString::number(tvecs[i][2]) + "  " + QString::number(Rx) + "  " + QString::number(Ry) + "  " + QString::number(Rz) + "]" + "\n";
                        ui->console->append(marker_info);
                    }

                }

                if(!test_marker){
                    vid.release();
                    ui->screen->clear();
                    ui->screen->setStyleSheet("background-color: rgb(104, 104, 104);");
                    ui->inform->setVisible(true);
                    break;
                }
                delay->delay(10);
            }
        }
    } else if(ui->test_marker->text() == "Running"){
        test_marker = false;
        delay->delay(100);
        ui->test_marker->setText("Test Marker");
        ui->test_marker->setStyleSheet("");
    }
}


void main_dialog::on_get_pose_clicked(){
    get_pose = true;
    qDebug() << "get pose";
}
