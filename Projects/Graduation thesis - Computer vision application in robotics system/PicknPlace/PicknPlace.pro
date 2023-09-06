QT += gui widgets core printsupport opengl multimedia serialport

TEMPLATE = lib
CONFIG += plugin

TARGET = PicknPlace

CONFIG += c++14

# Fix error "No target architecture" by adding _AMD64_
DEFINES += _AMD64_

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#INCLUDEPATH += C:\OpenCV\opencv\release\install\include
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_calib3d460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_core460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_highgui460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_imgcodecs460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_imgproc460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_objdetect460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_video460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_videoio460.lib'
#LIBS += C:\OpenCV\opencv\release\bin\opencv_videoio_ffmpeg460_64.lib'
#LIBS += C:\OpenCV\opencv\release\bin\libopencv_dnn460.lib'

# OpenCV 4.5.1
#INCLUDEPATH += C:\OpenCV\opencv-4.5.1\build_vc14\install\include
#win32: CONFIG(release, debug|release) {
#LIBS += -LC:\OpenCV\opencv-4.5.1\build_vc14\install\x64\vc14\lib \
#        -lopencv_core451 \
#        -lopencv_dnn451 \
#        -lopencv_highgui451 \
#        -lopencv_imgproc451 \
#        -lopencv_imgcodecs451 \
#        -lopencv_videoio451 \
#        -lopencv_video451 \
#        -lopencv_cudaarithm451 \
#        -lopencv_cudafeatures2d451\
#        -lopencv_opencv_surface_matching451

#LIBS += -LC:\local\boost_1_64_0\lib64-msvc-14.1\ -llibboost_date_time-vc141-mt-1_64
#}
#else: win32: CONFIG(debug, debug|release) {
#LIBS += -LC:\OpenCV\opencv-4.5.1\build_vc14\install\x64\vc14\lib \
#        -lopencv_core451d \
#        -lopencv_dnn451d \
#        -lopencv_highgui451d \
#        -lopencv_imgproc451d \
#        -lopencv_imgcodecs451d \
#        -lopencv_videoio451d \
#        -lopencv_video451d \
#        -lopencv_cudaarithm451d \
#        -lopencv_cudafeatures2d451d\
#        -lopencv_surface_matching451d

#LIBS += -LC:\local\boost_1_64_0\lib64-msvc-14.1\ -llibboost_date_time-vc141-mt-gd-1_64
#}

# OpenCV 4.7.0
INCLUDEPATH += C:\OpenCV\opencv-4.7.0\build_vc14\install\include
win32: CONFIG(release, debug|release) {
LIBS += -LC:\OpenCV\opencv-4.7.0\build_vc14\install\x64\vc14\lib \
        -lopencv_core470 \
        -lopencv_dnn470 \
        -lopencv_highgui470 \
        -lopencv_imgproc470 \
        -lopencv_imgcodecs470 \
        -lopencv_videoio470 \
        -lopencv_video470 \
        -lopencv_cudaarithm470 \
        -lopencv_cudafeatures2d470\
        -lopencv_opencv_surface_matching470\
        -lopencv_aruco470 \
        -lopencv_objdetect470 \
        -lopencv_calib3d470

LIBS += -LC:\local\boost_1_64_0\lib64-msvc-14.1\ -llibboost_date_time-vc141-mt-1_64
}
else: win32: CONFIG(debug, debug|release) {
LIBS += -LC:\OpenCV\opencv-4.7.0\build_vc14\install\x64\vc14\lib \
        -lopencv_core470d \
        -lopencv_dnn470d \
        -lopencv_highgui470d \
        -lopencv_imgproc470d \
        -lopencv_imgcodecs470d \
        -lopencv_videoio470d \
        -lopencv_video470d \
        -lopencv_cudaarithm470d \
        -lopencv_cudafeatures2d470d\
        -lopencv_surface_matching470d\
        -lopencv_aruco470d \
        -lopencv_objdetect470d \
        -lopencv_calib3d470d

LIBS += -LC:\local\boost_1_64_0\lib64-msvc-14.1\ -llibboost_date_time-vc141-mt-gd-1_64
}

#INCLUDEPATH += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\include'
#LIBS += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\bin\x64\realsense2.lib''
INCLUDEPATH += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\include'
LIBS += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\lib\x64\realsense2.lib'

INCLUDEPATH += 'C:\Program Files (x86)\Intel RealSense SDK 2.0'
INCLUDEPATH += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\samples'
INCLUDEPATH += 'C:\Program Files (x86)\Intel RealSense SDK 2.0\third-party\glfw-imgui\include'
#INCLUDEPATH += 'D:\Study\402\do_an\Qt\PicknPlace\onnxruntime\include\onnxruntime\core\session'

INCLUDEPATH += D:\Download\Libusb\libusb-1.0.26-binaries\libusb-MinGW-x64\include\libusb-1.0
LIBS += -LD:\Download\Libusb\libusb-1.0.26-binaries\VS2015-x64\lib\ -llibusb-1.0


INCLUDEPATH += 'C:\Program Files\GLFW\include'
LIBS += 'C:\Program Files\GLFW\lib\glfw3dll.lib'

INCLUDEPATH += 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include\GL'
LIBS += 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\glut64.lib'
LIBS += 'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\glut32.lib'

INCLUDEPATH += C:\visp-ws\visp-build-vc14\install\include
LIBS += -LC:\visp-ws\visp-build-vc14\install\x64\vc14\lib \
        -lvisp_ar351d \
        -lvisp_blob351d \
        -lvisp_core351d \
        -lvisp_detection351d \
        -lvisp_gui351d \
        -lvisp_imgproc351d \
        -lvisp_io351d \
        -lvisp_klt351d \
        -lvisp_mbt351d \
        -lvisp_me351d \
        -lvisp_robot351d \
        -lvisp_sensor351d \
        -lvisp_tt351d \
        -lvisp_tt_mi351d \
        -lvisp_vision351d \
        -lvisp_visual_features351d \
        -lvisp_vs351d

INCLUDEPATH += 'C:\Program Files\PCL 1.8.1\3rdParty\Boost\include\boost-1_64'
INCLUDEPATH += 'C:\Program Files\PCL 1.8.1\3rdParty\Eigen\eigen3'
INCLUDEPATH += 'C:\Program Files\PCL 1.8.1\3rdParty\FLANN\include'
INCLUDEPATH += 'C:\Program Files\PCL 1.8.1\3rdParty\VTK\include\vtk-8.0'
INCLUDEPATH += 'C:\Program Files\PCL 1.8.1\include\pcl-1.8'

LIBS += 'C:\Program Files\VTK\lib\QVTKWidgetPlugin.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkalglib-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkChartsCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonColor-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonComputationalGeometry-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonDataModel-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonExecutionModel-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonMath-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonMisc-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonSystem-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkCommonTransforms-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkDICOMParser-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkDomainsChemistry-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkDomainsChemistryOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkexoIIc-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkexpat-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersAMR-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersExtraction-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersFlowPaths-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersGeneral-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersGeneric-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersGeometry-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersHybrid-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersHyperTree-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersImaging-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersModeling-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersParallel-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersParallelImaging-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersPoints-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersProgrammable-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersSelection-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersSMP-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersSources-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersStatistics-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersTexture-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersTopology-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkFiltersVerdict-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkfreetype-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkGeovisCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkgl2ps-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkglew-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkGUISupportQt-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkGUISupportQtOpenGL-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkGUISupportQtSQL-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkhdf5-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkhdf5_hl-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingColor-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingFourier-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingGeneral-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingHybrid-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingMath-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingMorphological-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingSources-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingStatistics-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkImagingStencil-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkInfovisCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkInfovisLayout-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkInteractionImage-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkInteractionStyle-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkInteractionWidgets-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOAMR-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOEnSight-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOExodus-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOExport-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOExportOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOGeometry-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOImage-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOImport-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOInfovis-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOLegacy-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOLSDyna-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOMINC-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOMovie-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIONetCDF-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOParallel-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOParallelXML-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOPLY-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOSQL-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOTecplotTable-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOVideo-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOXML-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkIOXMLParser-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkjpeg-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkjsoncpp-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtklibharu-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtklibxml2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtklz4-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkmetaio-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkNetCDF-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtknetcdf_c++.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkoggtheora-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkParallelCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkpng-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkproj4-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingAnnotation-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingContext2D-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingContextOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingFreeType-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingGL2PSOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingImage-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingLabel-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingLOD-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingQt-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingVolume-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkRenderingVolumeOpenGL2-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtksys-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtktiff-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkverdict-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkViewsContext2D-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkViewsCore-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkViewsInfovis-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkViewsQt-8.0.lib'
LIBS += 'C:\Program Files\VTK\lib\vtkzlib-8.0.lib'

LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_common_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_features_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_filters_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_io_ply_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_io_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_kdtree_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_keypoints_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_ml_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_octree_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_outofcore_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_people_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_recognition_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_registration_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_sample_consensus_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_search_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_segmentation_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_stereo_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_surface_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_tracking_release.lib'
LIBS += 'C:\Program Files\PCL 1.8.1\lib\pcl_visualization_release.lib'

SOURCES += \
    calibrate.cpp \
    camera.cpp \
    delay_timer.cpp \
    evaluate_dialog.cpp \
    labelinteractor.cpp \
    motoman_command.cpp \
    motoman_communication.cpp \
    poseestimation.cpp \
    qcustomplot.cpp \
    main_dialog.cpp \
    picknplace.cpp \
    servo.cpp \
    udp.cpp \
    usystem.cpp \
    yolo_seg.cpp

HEADERS += \
    PluginInterface.h \
    calibrate.h \
    camera.h \
    evaluate_dialog.h \
    labelinteractor.h \
    motoman_command.h \
    motoman_communication.h \
    poseestimation.h \
    qcustomplot.h \
    delay_timer.h \
    main_dialog.h \
    picknplace.h \
    servo.h \
    udp.h \
    usystem.h \
    yolo_seg.h

DISTFILES += PicknPlace.json

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/accessible
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    evaluate_dialog.ui \
    main_dialog.ui

RESOURCES += \
    resource.qrc
