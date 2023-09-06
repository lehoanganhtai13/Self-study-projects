#include "camera.h"

Camera::Camera(QObject *parent)
    : QThread{parent}
{

}

frameset cam_frames;
depth_frame outputDepthFrame(cam_frames);

void Camera::registerCamera(camera_module module, pipeline &p, colorizer color, rates_printer pr){
    cam_module = module;
    pipe = p;
    color_map = color;
    printer = pr;
}

void Camera::getFrame(cv::Mat &frame){
    frame = outputFrame;
}

void Camera::getDepthFrame(depth_frame &dep_frame){
    dep_frame = outputDepthFrame;
}

void Camera::run(){
    camera_run = true;

    while(camera_run){
        cam_frames = pipe.wait_for_frames();
        if(cam_module == RGB_camera){
            //Get each frame
            frame color_frame = cam_frames.get_color_frame().apply_filter(color_map).apply_filter(printer);

            auto str_width = color_frame.as<video_frame>().get_width();
            auto str_height = color_frame.as<video_frame>().get_height();

            auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

            // Creating OpenCV Matrix from a color image
            cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

            outputFrame = color;
            emit sendCameraRGBFrame(str_width, str_height, frame_rate);

            wait_frame = true;
            while(wait_frame){
                cv::waitKey(5);
            }
        } else if(cam_module == stereo_camera){
            //Get each frame
//            frame ir_frame = frames.first(RS2_STREAM_INFRARED);
            auto dep_frame = cam_frames.get_depth_frame().apply_filter(color_map).apply_filter(printer);
//            frame depth_frame = frames.first(RS2_STREAM_DEPTH, RS2_FORMAT_RGB8);

            auto str_width = dep_frame.as<video_frame>().get_width();
            auto str_height = dep_frame.as<video_frame>().get_height();

            auto frame_rate = dep_frame.as<video_frame>().get_profile().fps();

            // Creating OpenCV matrix from IR image
            cv::Mat dep(cv::Size(str_width, str_height), CV_8UC3, (void*)dep_frame.get_data(), cv::Mat::AUTO_STEP);

            // Apply Histogram Equalization
//            equalizeHist( dep, dep );
            cv::applyColorMap(dep, dep, cv::COLORMAP_JET);

            outputFrame = dep;
            emit sendCamerDepthFrame(str_width, str_height, frame_rate);

            wait_frame = true;
            while(wait_frame){
                cv::waitKey(5);
            }
        } else if(cam_module == align_camera){
            rs2::align align_to_color(RS2_STREAM_COLOR);
            auto aligned_frames = align_to_color.process(cam_frames);
//            rs2::align align_to_depth(RS2_STREAM_DEPTH);
//            auto aligned_frames = align_to_depth.process(cam_frames);

            rs2::spatial_filter spat;
            spat.set_option(RS2_OPTION_HOLES_FILL, 5);

            auto color_frame = aligned_frames.get_color_frame().apply_filter(color_map)/*.apply_filter(spat)*/;
            auto dep_frame = aligned_frames.get_depth_frame();
//            qDebug() << dep_frame.get_width() << dep_frame.get_height();

            auto str_width = color_frame.as<video_frame>().get_width();
            auto str_height = color_frame.as<video_frame>().get_height();

            auto frame_rate = color_frame.as<video_frame>().get_profile().fps();

            // Creating OpenCV Matrix from a color image
            cv::Mat color(cv::Size(str_width, str_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

            outputFrame = color;
            outputDepthFrame = dep_frame;
            emit sendCameraRGBDFrame(str_width, str_height, frame_rate);

            wait_frame = true;
            while(wait_frame){
                cv::waitKey(20);
//                qDebug() << "wait frame";
            }
        }
    }
}

void Camera::stop(){
    camera_run = false;
}
