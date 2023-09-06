#include "calibrate.h"

Calibration::Calibration(QObject *parent)
    : QThread{parent}
{

}

void Calibration::registerInput(std::vector<vpHomogeneousMatrix> cMoi, std::vector<vpHomogeneousMatrix> wMei, int iteration, QString base2cam_log){
    cMo = cMoi;
    wMe = wMei;
    N = iteration;
    if(!base2cam_log.contains("none"))
        log_file.setFileName(base2cam_log);
}

vpHomogeneousMatrix Calibration::HandEyeTransformation(){
    return bMc;
}

void Calibration::calibrateLeastSquares(){
    std::vector<vpHomogeneousMatrix> oMc(N);
    std::vector<double> x(N), y(N), z(N), rx(N), ry(N), rz(N);

    QTextStream bTo_str;
    if(!log_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        qDebug() << "Failed to open log text";
    } else
        bTo_str.setDevice(&log_file);

    for(int i = 0; i < N; i++){
        cMo.at(i).inverse(oMc.at(i));
    }

    for(int i = 0; i < N; i++){
        vpHomogeneousMatrix bMc_temp = oMc.at(i)*wMe.at(i);
        vpRzyxVector brc(bMc_temp.getRotationMatrix());

        x.at(i) = bMc_temp[0][3];
        y.at(i) = bMc_temp[1][3];
        z.at(i) = bMc_temp[2][3];
        rx.at(i) = brc[2];
        ry.at(i) = brc[1];
        rz.at(i) = brc[0];

        bTo_str << x.at(i)*1000.0 << "," << y.at(i)*1000.0 << "," << z.at(i)*1000.0 << "," << rx.at(i)*180.0/M_PI << "," << ry.at(i)*180.0/M_PI << "," << rz.at(i)*180.0/M_PI << "\n";
    }
    bTo_str << "\n";
    log_file.flush();
    log_file.close();

    double x_brc, y_brc, z_brc, rx_brc, ry_brc, rz_brc;
    x_brc = 0;
    y_brc = 0;
    z_brc = 0;
    rx_brc = 0;
    ry_brc = 0;
    rz_brc = 0;
    for(int i = 0; i < N; i++){
        x_brc = x_brc + x.at(i);
        y_brc = y_brc + y.at(i);
        z_brc = z_brc + z.at(i);
        rx_brc = rx_brc + rx.at(i);
        ry_brc = ry_brc + ry.at(i);
        rz_brc = rz_brc + rz.at(i);
    }
    x_brc = x_brc/double(N);
    y_brc = y_brc/double(N);
    z_brc = z_brc/double(N);
    rx_brc = rx_brc/double(N);
    ry_brc = ry_brc/double(N);
    rz_brc = rz_brc/double(N);

    qDebug() << "Result:" << x_brc << "," << y_brc << "," << z_brc << "," << rx_brc << "," << ry_brc << "," << rz_brc;

}

void Calibration::calibrateHandEye(){
    vpHomogeneousMatrix temp, cMb;
    std::vector<vpHomogeneousMatrix> oMc(N), eMb(N);
    std::vector<cv::Mat> R_cam2target, R_base2gripper, R_target2cam;
    std::vector<cv::Mat> t_cam2target, t_base2gripper, t_target2cam;

    for(int i = 0; i < N; i++){
        cMo.at(i).inverse(temp);
        oMc.at(i) = temp;

        wMe.at(i).inverse(temp);
        eMb.at(i) = temp;
    }

    vpRotationMatrix vpR;
    vpTranslationVector vpt;
    cv::Matx33d R;
    cv::Matx31d t;
    R.zeros();
    t.zeros();

    R_cam2target.reserve(N);
    t_cam2target.reserve(N);

    for(int i = 0; i < N; i++){
//        vpR = cMo.at(i).getRotationMatrix();
//        R(0,0) = vpR[0][0];
//        R(0,1) = vpR[0][1];
//        R(0,2) = vpR[0][2];
//        R(1,0) = vpR[1][0];
//        R(1,1) = vpR[1][1];
//        R(1,2) = vpR[1][2];
//        R(2,0) = vpR[2][0];
//        R(2,1) = vpR[2][1];
//        R(2,2) = vpR[2][2];
//        R_cam2target.push_back(cv::Mat(R));

//        vpt = cMo.at(i).getTranslationVector();
//        t(0) = vpt[0];
//        t(1) = vpt[1];
//        t(2) = vpt[2];
//        t_cam2target.push_back(cv::Mat(t));

        vpR = cMo.at(i).getRotationMatrix();
        R(0,0) = vpR[0][0];
        R(0,1) = vpR[0][1];
        R(0,2) = vpR[0][2];
        R(1,0) = vpR[1][0];
        R(1,1) = vpR[1][1];
        R(1,2) = vpR[1][2];
        R(2,0) = vpR[2][0];
        R(2,1) = vpR[2][1];
        R(2,2) = vpR[2][2];
        R_target2cam.push_back(cv::Mat(R));
//        std::cout << R << std::endl;

        vpt = cMo.at(i).getTranslationVector();
        t(0) = vpt[0];
        t(1) = vpt[1];
        t(2) = vpt[2];
        t_target2cam.push_back(cv::Mat(t));
    }

    std::cout << endl;
//    for(int i = 0; i < N; i++)
//        std::cout << R_cam2target.at(i) << std::endl;
//    for(int i = 0; i < N; i++)
//        std::cout << t_cam2target.at(i).t() << std::endl;
    //        std::cout << R_cam2target.at(i) << std::endl;
    std::cout << cMo.at(N - 1) << std::endl;

    R_base2gripper.reserve(N);
    t_base2gripper.reserve(N);
    for(int i = 0; i < N; i++){
        vpR = eMb.at(i).getRotationMatrix();
        R(0,0) = vpR[0][0];
        R(0,1) = vpR[0][1];
        R(0,2) = vpR[0][2];
        R(1,0) = vpR[1][0];
        R(1,1) = vpR[1][1];
        R(1,2) = vpR[1][2];
        R(2,0) = vpR[2][0];
        R(2,1) = vpR[2][1];
        R(2,2) = vpR[2][2];
        R_base2gripper.push_back(cv::Mat(R));

        vpt = eMb.at(i).getTranslationVector();
        t(0) = vpt[0];
        t(1) = vpt[1];
        t(2) = vpt[2];
        t_base2gripper.push_back(cv::Mat(t));
    }

    std::cout << endl;
//    for(int i = 0; i < N; i++)
//        std::cout << R_gripper2base.at(i) << std::endl;
//    for(int i = 0; i < N; i++)
//        std::cout << t_gripper2base.at(i).t() << std::endl;

    std::vector<cv::HandEyeCalibrationMethod> methods = {cv::CALIB_HAND_EYE_PARK,
                                                         cv::CALIB_HAND_EYE_HORAUD};

    int ret = 1;
    for(auto method : methods){
        cv::Matx33d R_cam2base;
        cv::Matx31d t_cam2base;
        cv::calibrateHandEye(R_base2gripper, t_base2gripper, R_target2cam, t_target2cam, R_cam2base, t_cam2base, method);

        if(cv::checkRange(R_cam2base))
            if(cv::checkRange(t_cam2base))
                ret = 0;

        qDebug() << ret;
        if(ret == 0){
            qDebug() << "Hand-eye calibration succeed.";

            vpR[0][0] = R_cam2base(0,0);
            vpR[0][1] = R_cam2base(0,1);
            vpR[0][2] = R_cam2base(0,2);
            vpR[1][0] = R_cam2base(1,0);
            vpR[1][1] = R_cam2base(1,1);
            vpR[1][2] = R_cam2base(1,2);
            vpR[2][0] = R_cam2base(2,0);
            vpR[2][1] = R_cam2base(2,1);
            vpR[2][2] = R_cam2base(2,2);

            vpt[0] = t_cam2base(0);
            vpt[1] = t_cam2base(1);
            vpt[2] = t_cam2base(2);


            const vpRotationMatrix vpR_new = vpR;
            const vpTranslationVector vpt_new = vpt;


            bMc.buildFrom(vpt_new, vpR_new);
            vpRzyxVector erc(bMc.getRotationMatrix());
            qDebug() << bMc[0][3]*1000.0 << bMc[1][3]*1000.0 << bMc[2][3]*1000.0 << vpMath::deg(erc[2]) << vpMath::deg(erc[1]) << vpMath::deg(erc[0]);

//            bMc.inverse(cMb);
//            erc.buildFrom(bMc.getRotationMatrix());
//            qDebug() << cMb[0][3]*1000.0 << cMb[1][3]*1000.0 << cMb[2][3]*1000.0 << vpMath::deg(erc[2]) << vpMath::deg(erc[1]) << vpMath::deg(erc[0]);

        }
        else{
            qDebug() << "Failed to calibrate";
        }
    }

//    ret = vpHandEyeCalibration::calibrate(cMo, eMb, bMc);
    if(ret == 0){
//        qDebug() << "VISP Hand-eye calibration succeed.";
        vpRzyxVector erc(bMc.getRotationMatrix());
        bMc.inverse(cMb);
        vpRzyxVector erc2(cMb.getRotationMatrix());
        std::cout << "bMc: " << bMc[0][3]*1000.0 << ", " << bMc[1][3]*1000.0 << ", " << bMc[2][3]*1000.0 << ", " << vpMath::deg(erc[2]) << ", " << vpMath::deg(erc[1]) << ", " << vpMath::deg(erc[0]) << std::endl;
//        std::cout << "cMb: " << cMb[0][3]*1000.0 << ", " << cMb[1][3]*1000.0 << ", " << cMb[2][3]*1000.0 << ", " << vpMath::deg(erc2[2]) << ", " << vpMath::deg(erc2[1]) << ", " << vpMath::deg(erc2[0]) << std::endl;
        vpHomogeneousMatrix bMo = bMc*cMo.at(N - 1);
        vpRzyxVector erc3(bMo.getRotationMatrix());
        std::cout << "Validate: " << bMo[0][3]*1000.0 << ", " << bMo[1][3]*1000.0 << ", " << bMo[2][3]*1000.0 << ", " << vpMath::deg(erc3[2]) << ", " << vpMath::deg(erc3[1]) << ", " << vpMath::deg(erc3[0]) << std::endl;
    }
    else{
        qDebug() << "Failed to calibrate";
    }

//    emit finishCalculate(true, ret);
}

void Calibration::run(){
    calibrateHandEye();
}
