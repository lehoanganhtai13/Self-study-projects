#include "poseestimation.h"

PoseEstimation::PoseEstimation(QObject *parent)
    : QObject{parent}
{

}

void PoseEstimation::getLine(cv::Point p1, cv::Point p2, double &a, double &b){
    a = (double(p1.y) - double(p2.y))/(double(p1.x) - double(p2.x));
    b = (double(p1.y)*double(p2.x)/double(p1.x) - double(p2.y))/(double(p2.x)/double(p1.x) - 1.0);
    if(!(std::isinf(a) ||  std::isinf(b))){
        qDebug() << "Find line";
    } else{
        qDebug() << "No slope";
    }
}

void PoseEstimation::calculateDistance(cv::Mat img, int &distance){
    // Convert the image to grayscale
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

    // Apply a threshold
    cv::threshold(img, img, 127, 255, cv::THRESH_BINARY);

    img = cv::Scalar(255,255,255) - img;

    // Create vectors to store the x coordinates of non-zero pixels
    std::vector<int> white_only_x;

    // Find the locations of the non-zero elements in the image
    cv::Mat locations;
    cv::findNonZero(img, locations);

    // Iterate over the rows and columns of the image
    for (int i = 0; i < locations.total(); i++)
    {
        white_only_x.push_back(locations.at<cv::Point>(i).x);
    }

    // Find the minimum and maximum x coordinates
    int min_X = *std::min_element(white_only_x.begin(), white_only_x.end());
    int max_X = *std::max_element(white_only_x.begin(), white_only_x.end());

    // Calculate the distance
    distance = max_X - min_X;

    return;
}

bool PoseEstimation::isPointValid(cv::Mat img, cv::Point p, int choice, bool isLeft_Up){
    // Convert the image to grayscale
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

    // Apply a threshold
    cv::threshold(img, img, 127, 255, cv::THRESH_BINARY);

    img = cv::Scalar(255,255,255) - img;

    if(choice == 0){
        // Create vectors to store the x coordinates of non-zero pixels
        std::vector<int> white_only_x;

        // Find the locations of the non-zero elements in the image
        cv::Mat locations;
        cv::findNonZero(img, locations);

        // Iterate over the rows and columns of the image
        for (int i = 0; i < locations.total(); i++)
        {
            white_only_x.push_back(locations.at<cv::Point>(i).x);
        }

        if(isLeft_Up){
            int min = *std::min_element(white_only_x.begin(), white_only_x.end());
            if(p.x > min + 5)
                return true;
            else
                return false;
        } else{
            int max = *std::max_element(white_only_x.begin(), white_only_x.end());
            if(p.x < max - 5)
                return true;
            else
                return false;
        }
    } else if(choice == 1){
        // Create vectors to store the y coordinates of non-zero pixels
        std::vector<int> white_only_y;

        // Find the locations of the non-zero elements in the image
        cv::Mat locations;
        cv::findNonZero(img, locations);

        // Iterate over the rows and columns of the image
        for (int i = 0; i < locations.total(); i++)
        {
            white_only_y.push_back(locations.at<cv::Point>(i).y);
        }

        if(isLeft_Up){
            int max = *std::max_element(white_only_y.begin(), white_only_y.end());
            if(p.y < max - 5)
                return true;
            else
                return false;
        } else{
            int min = *std::min_element(white_only_y.begin(), white_only_y.end());
            if(p.y > min + 5)
                return true;
            else
                return false;
        }
    }
}

void PoseEstimation::createBackground(cv::Mat &img, double u, double v, double w, double h, std::string save_path){
    cv::Mat bg = cv::Mat::zeros(480, 640, CV_64F);
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    img.copyTo(bg(cv::Rect(u,v,w,h)));
    img = bg;
    cv::imwrite(save_path + "\\mask_bg.png", img);
    qDebug() << "Create background successfully";

    return;
}

void PoseEstimation::drawAxis(cv::Mat &img, cv::Point p, cv::Point q, cv::Scalar colour, const float scale = 0.2){
    double angle = atan2( (double) p.y - q.y, (double) p.x - q.x ); // angle in radians
    double hypotenuse = sqrt( (double) (p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));
    // Here we lengthen the arrow by a factor of scale
    q.x = (int) (p.x - scale * hypotenuse * cos(angle));
    q.y = (int) (p.y - scale * hypotenuse * sin(angle));
    cv::line(img, p, q, colour, 1, cv::LINE_AA);
    // create the arrow hooks
    p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
    cv::line(img, p, q, colour, 1, cv::LINE_AA);
    p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
    cv::line(img, p, q, colour, 1, cv::LINE_AA);
}

bool PoseEstimation::findPointHead(cv::Mat img, cv::Point center, cv::Point &pointHead, bool isHeadUp, double headSize, double tailSize){
    // Convert the image to grayscale
    cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

    // Apply a threshold
    cv::threshold(img, img, 127, 255, cv::THRESH_BINARY);

    img = cv::Scalar(255,255,255) - img;

    // Create vectors to store the x coordinates of non-zero pixels
    std::vector<int> white_only_x;
    std::vector<int> white_only_y;

    // Find the locations of the non-zero elements in the image
    cv::Mat locations;
    cv::findNonZero(img, locations);

    qDebug() << "Head" << isHeadUp;
    // Iterate over the rows and columns of the image
    for (int i = 0; i < locations.total(); i++)
    {
        white_only_x.push_back(locations.at<cv::Point>(i).x);
        white_only_y.push_back(locations.at<cv::Point>(i).y);
    }

    int bounding_y_pos = center.x - int(headSize/2.0) + int((headSize - tailSize)/4.0);
    int start_point = center.y;
    if(isHeadUp){
        int max = *std::max_element(white_only_y.begin(), white_only_y.end());
//        qDebug() << "Max" << max;
        bool found = false;
        int move_distance = 0;
        while(!found){
            move_distance = move_distance + 1;
            start_point = start_point - move_distance;
//            qDebug() << "Cur" << start_point;
            int i = 0;
            while(i < white_only_y.size()){
//                qDebug() << white_only_y.size() << "kien gay" << i;
                if(white_only_y.at(i) == start_point && white_only_x.at(i) == bounding_y_pos){
                    start_point = start_point + 2;
                    found = true;
                    break;
                }
                i = i + 1;
            }
            if(move_distance == 500)
                return false;
        }
    } else{
        int min = *std::min_element(white_only_y.begin(), white_only_y.end());
//        qDebug() << "Min" << min;
        bool found = false;
        int move_distance = 0;
        while(!found){
            move_distance = move_distance + 1;
            start_point = start_point + move_distance;
//            qDebug() << "Cur" << start_point;
            int i = 0;
            while(i < white_only_y.size()){
//                qDebug() << white_only_y.size() << "kien gay" << i;
                if(white_only_y.at(i) == start_point && white_only_x.at(i) == bounding_y_pos){
                    start_point = start_point - 2;
                    found = true;
                    break;
                }
                i = i + 1;
            }
            if(move_distance == 500)
                return false;
        }
    }
    pointHead = cv::Point(center.x, start_point);
    cv::circle(img, pointHead, 4, cv::Scalar(255,12,12));
    return true;
}

double PoseEstimation::getOrientation(const std::vector<cv::Point> &pts, cv::Mat &img, std::vector<cv::Point> &center, bool &headUp){
    cv::Mat clone_img = img.clone();

    //Construct a buffer used by the pca analysis
    int sz = static_cast<int>(pts.size());
    cv::Mat data_pts = cv::Mat(sz, 2, CV_64F);
    for (int i = 0; i < data_pts.rows; i++)
    {
        data_pts.at<double>(i, 0) = pts[i].x;
        data_pts.at<double>(i, 1) = pts[i].y;
    }
    //Perform PCA analysis
    cv::PCA pca_analysis(data_pts, cv::Mat(), cv::PCA::DATA_AS_ROW);
    //Store the center of the object
    cv::Point cntr = cv::Point(static_cast<int>(pca_analysis.mean.at<double>(0, 0)),
                               static_cast<int>(pca_analysis.mean.at<double>(0, 1)));
    //Store the eigenvalues and eigenvectors
    std::vector<cv::Point2d> eigen_vecs(2);
    std::vector<double> eigen_val(2);
    for (int i = 0; i < 2; i++)
    {
        eigen_vecs[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                    pca_analysis.eigenvectors.at<double>(i, 1));
        eigen_val[i] = pca_analysis.eigenvalues.at<double>(i);
    }
    // Draw the principal components
    cv::circle(img, cntr, 3, cv::Scalar(255, 0, 255), 2);
    cv::Point p1 = cntr + 0.02 * cv::Point(static_cast<int>(eigen_vecs[0].x * eigen_val[0]), static_cast<int>(eigen_vecs[0].y * eigen_val[0]));
    cv::Point p2 = cntr - 0.02 * cv::Point(static_cast<int>(eigen_vecs[1].x * eigen_val[1]), static_cast<int>(eigen_vecs[1].y * eigen_val[1]));
    double a1, a2, b1, b2;
    getLine(cntr, p1, a1, b1);
    qDebug() << "y1 =" << QString::number(a1) << "x1 +" << QString::number(b1);
    getLine(cntr, p2, a2, b2);
    qDebug() << "y2 =" << QString::number(a2) << "x2 +" << QString::number(b2);
    int x = cntr.x - 10;
    int y = a1*x + b1;
    cv::Point p3(x, y);
    y = cntr.y + 10;
    x = int((y - b2)/a2);
    cv::Point p4(x,y);

    double angle = atan2(eigen_vecs[0].y, eigen_vecs[0].x); // orientation in radians
    angle = -angle*180.0/M_PI;

    double h = clone_img.rows;
    double w = clone_img.cols;

    double rot_angle = 0;
    if (angle < 0 && angle > -90) {
        rot_angle = -90 - angle;
    } else if (angle < -90 && angle > -180) {
        rot_angle = -(angle + 90);
    } else if (angle > 0 && angle < 90) {
        rot_angle = 90 - angle;
    } else if (angle > 90 && angle < 180) {
        rot_angle = -(angle - 90);
    }

    cv::Mat rotated_img;
    cv::Mat M = cv::getRotationMatrix2D(cntr, rot_angle, 1.0);
    cv::warpAffine(clone_img, rotated_img, M, cv::Size(w, h), cv::INTER_CUBIC, cv::BORDER_REPLICATE);
    cv::Mat top_img = rotated_img(cv::Rect(0, 0, w, cntr.y));
    cv::Mat bottom_img = rotated_img(cv::Rect(0, cntr.y, w, h - cntr.y));

    int top_distance, bot_distance;
    calculateDistance(top_img, top_distance);
    calculateDistance(bottom_img, bot_distance);
    qDebug() << "Distance:" << top_distance << bot_distance;

    qDebug() << "Origin Rz:" << angle;
    qDebug() << "Rotated angle:" << rot_angle;

    cv::Point other_point;
    double headSize = 0;
    double tailSize = 0;
    if (angle < 0) {
        if (bot_distance > top_distance) {
            qDebug() << "Head is below";
            drawAxis(img, cntr, p1, cv::Scalar(0, 0, 255), 1.5);  // X axis
            drawAxis(img, cntr, p2, cv::Scalar(0, 255, 0), 12);    // Y axis
            headUp = false;
            headSize = bot_distance;
            tailSize = top_distance;
            qDebug() << "1";
        } else {
            qDebug() << "Head is top";
            angle = angle + 180.0;
            drawAxis(img, cntr, p3, cv::Scalar(0, 0, 255), 1.5);  // X axis
            drawAxis(img, cntr, p4, cv::Scalar(0, 255, 0), 4);    // Y axis
            headUp = true;
            headSize = top_distance;
            tailSize = bot_distance;
            qDebug() << "2";
        }
    } else {
        if (bot_distance < top_distance) {
            qDebug() << "Head is top";
            drawAxis(img, cntr, p1, cv::Scalar(0, 0, 255), 1.5);  // X axis
            drawAxis(img, cntr, p2, cv::Scalar(0, 255, 0), 12);    // Y axis
            headUp = true;
            headSize = top_distance;
            tailSize = bot_distance;
            qDebug() << "3";
        } else {
            qDebug() << "Head is below";
            angle =  -(180.0 - angle);
            drawAxis(img, cntr, p3, cv::Scalar(0, 0, 255), 1.5);  // X axis
            drawAxis(img, cntr, p4, cv::Scalar(0, 255, 0), 4);    // Y axis
            headUp = false;
            headSize = bot_distance;
            tailSize = top_distance;
            qDebug() << "4";
        }
    }

    cv::Point p5 = cv::Point(cntr.x - 25, int(a1*(cntr.x - 25) + b1));
    cv::Point p6 = cv::Point(cntr.x + 25, int(a1*(cntr.x + 25) + b1));
    cv::Point p7 = cv::Point(int((cntr.y - 25 - b1)/a1), cntr.y - 25);
    cv::Point p8 = cv::Point(int((cntr.y + 25 - b1)/a1), cntr.y + 25);
    cv::Point p9 = cv::Point(cntr.x, cntr.y - 25);
    cv::Point p10 = cv::Point(cntr.x, cntr.y + 25);

    angle = -angle;
    qDebug() << "Modified Rz:" << angle;

    bool point_valid = true;
    int offset_point = 0;
    cv::Point temp_point;
    // In case if object is nearly horizontal
    if(-55.0 < angle && angle < 55.0){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(cntr.x - offset_point, int(a1*(cntr.x - offset_point) + b1));
            point_valid = isPointValid(clone_img, temp_point, 0, true);
        }
        other_point = temp_point;
//        other_point = p5;
    } else if((125.0 < angle) || ( angle < -125.0)){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(cntr.x + offset_point, int(a1*(cntr.x + offset_point) + b1));
            point_valid = isPointValid(clone_img, temp_point, 0, false);
        }
        other_point = temp_point;
//        other_point = p6;
    }
    // In case if object is nearly vertical
    else if((-125 <= angle &&  angle < -95) || (-85 < angle && angle <= -55)){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(int((cntr.y + offset_point - b1)/a1), cntr.y + offset_point);
            point_valid = isPointValid(clone_img, temp_point, 1, true);
        }
        other_point = temp_point;
//        other_point = p8;
    } else if((55 <= angle && angle < 85) || (95 < angle && angle <= 125)){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(int((cntr.y - offset_point - b1)/a1), cntr.y - offset_point);
            point_valid = isPointValid(clone_img, temp_point, 1, false);
        }
        other_point = temp_point;
//        other_point = p7;
    } else if(-95 <= angle && angle <= -85){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(cntr.x, cntr.y + offset_point);
            point_valid = isPointValid(clone_img, temp_point, 1, true);
        }
        other_point = temp_point;
//        other_point = p10;
    } else if(85 <= angle && angle <= 95){
        while(point_valid){
            offset_point = offset_point + 1;
            temp_point = cv::Point(cntr.x, cntr.y - offset_point);
            point_valid = isPointValid(clone_img, temp_point, 1, false);
        }
        other_point = temp_point;
//        other_point = p9;
    }

    std::ostringstream  str;
    str << std::setprecision(2) << std::fixed << angle;
    std::string label = " Rotation angle: " + str.str();
    double offset = 30;
    cv::rectangle(img, cv::Point(cntr.x, (cntr.y-25) - offset), cv::Point(cntr.x + 250, (cntr.y + 10) - offset), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::putText(img, label, cv::Point(cntr.x, cntr.y - offset), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);

    // Find Head point
    cv::Point headPoint;
    bool found = findPointHead(rotated_img, cntr, headPoint, headUp, headSize, tailSize);
    if(!found){
        center.reserve(1);
        center.push_back(cntr);
        return 0.0;
    }
    qDebug() << "Head point origin:" << headPoint.x << headPoint.y;
    int u_head, v_head;
    double delta_y = abs(cntr.y - headPoint.y);
    if(headUp){
        if(angle > -90.0)
            u_head = cntr.x + abs(cos(abs(angle*M_PI/180.0))*delta_y);
        else if(angle < -90.0)
            u_head = cntr.x - abs(cos(abs(angle*M_PI/180.0))*delta_y);
        else if(angle == -90.0)
            u_head = headPoint.x;
        v_head = cntr.y - abs(sin(abs(angle*M_PI/180.0))*delta_y);
    } else{
        if(angle < 90.0)
            u_head = cntr.x + abs(cos(abs(angle*M_PI/180.0))*delta_y);
        else if(angle > 90.0)
            u_head = cntr.x - abs(cos(abs(angle*M_PI/180.0))*delta_y);
        else if(angle == 90.0)
            u_head = headPoint.x;
        v_head = cntr.y + abs(sin(abs(angle*M_PI/180.0))*delta_y);
    }
    cv::Point modified_headPoint = cv::Point(u_head, v_head);
    qDebug() << "Head point mod:" << modified_headPoint.x << modified_headPoint.y;

    center.reserve(5);
    center.push_back(cntr);
    center.push_back(p1);   // X
    center.push_back(p2);   // Y
    center.push_back(other_point);  // Other point to find Rx
    center.push_back(modified_headPoint);
    return angle;
}

double PoseEstimation::run(cv::Mat &mask, QString folder_dir, std::vector<cv::Point> &center, int idx, bool &headUp){
    mask = cv::Scalar(255, 255, 255) - mask;

    // Convert image to grayscale
    cv::Mat gray;
    cv::cvtColor(mask, gray, cv::COLOR_BGR2GRAY);

    // Find all the contours in the thresholded images
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(gray, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

    double Rz;
    for (size_t i = 0; i < contours.size(); i++){
        // Calculate the area of each contour
        double area = cv::contourArea(contours[i]);
        // Ignore contours that are too small or too large
        if (area < 600 || 100000 < area) continue;
        // Draw each contour only for visualisation purposes
        cv::drawContours(mask, contours, static_cast<int>(i), cv::Scalar(0, 0, 255), 2);
        // Find the orientation of each shape
        Rz = getOrientation(contours[i], mask, center, headUp);
        if(center.size() < 4)
            return -1000.0;
    }

    QDir save_dir(folder_dir);
    if(!save_dir.exists())
        save_dir.mkpath(folder_dir);
    std::string file_dir = folder_dir.toStdString() + "/" + std::to_string(idx) + ".jpg";
    cv::imwrite(file_dir, mask);
    std::cout << "Save dir: " << file_dir << std::endl;

    return Rz;
}
