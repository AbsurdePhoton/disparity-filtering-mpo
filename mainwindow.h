/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.2 - 2018/07/16
#
#-------------------------------------------------*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <disparity.h>
#include <adjust.h>

#include <iostream>

#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/ximgproc/disparity_filter.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core/utility.hpp"
#include <QMainWindow>
#include <QFileDialog>

using namespace cv;
using namespace cv::ximgproc;
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Left_clicked();

    void on_Right_clicked();

    void on_MPO_clicked ();

    void on_Camera_clicked ();

    void on_Rectify_clicked ();

    void on_Swap_clicked ();

    void on_Normalize_clicked();

    void on_Rotate_clicked();

    void on_Negative_clicked();

    void on_Save_Images_clicked();

    void on_Save_depthmap_clicked(); // Write depthmap image file to png

    void ShowDepthmap();

    void on_checkBox_fit_clicked();

    void on_Disparity_clicked();

    void on_Adjust_clicked();

    void on_horizontalSlider_num_of_disparity_sliderMoved(int position);

    void on_horizontalSlider_num_of_disparity_valueChanged(int value);

    void on_horizontalSlider_SAD_window_size_valueChanged(int value);

    void on_horizontalSlider_pre_filter_size_valueChanged(int value);

    void on_horizontalSlider_pre_filter_cap_valueChanged(int value);

    void on_horizontalSlider_min_disparity_valueChanged(int value);

    void on_horizontalSlider_texture_threshold_valueChanged(int value);

    void on_horizontalSlider_uniqueness_ratio_valueChanged(int value);

    void on_horizontalSlider_speckle_window_size_valueChanged(int value);

    void on_horizontalSlider_speckle_range_valueChanged(int value);

    void on_horizontalSlider_disp_12_max_diff_valueChanged(int value);

    void on_horizontalSlider_P1_valueChanged(int value);

    void on_horizontalSlider_P2_valueChanged(int value);

    void on_radioButton_BM_clicked ();

    void on_radioButton_SGBM_clicked ();

    void on_Compute_clicked();  // compute depth map with OpenCV

private:
    // the UI object, to access the UI elements created with Qt Designer
    Ui::MainWindow *ui;

    cv::Mat left_image, right_image; // left and right images
    cv::Mat disp_color; // Processed stereo map
    Mat KR, KL, DL, DR, R, F, E;
    Vec3d T;
    std::string basename;

    // the object that holds the parameters for the block-matching algorithm
    Ptr<cv::StereoBM> bmState = cv::StereoBM::create(16, 2);
    Ptr<cv::StereoSGBM> sgbmState = cv::StereoSGBM::create(-32, 144, 5);

    // To know if a depthmap has been computed
    bool computed;
    bool parameters;
    bool rectified;

    // functions to manage constraints on sliders
    void set_SADWindowSize();  // manage max value of SADWindowSize slider
    void set_num_of_disparity_slider_to_multiple_16(int position);

    Rect computeROI(Size2i src_sz, Ptr<StereoMatcher> matcher_instance);
};

#endif // MAINWINDOW_H
