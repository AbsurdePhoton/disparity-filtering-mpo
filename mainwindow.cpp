/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.2 - 2018/07/16
#
#-------------------------------------------------*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include "mpo.h"
#include "mat-image-tools.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Init default values
    ui->horizontalSlider_pre_filter_size->setValue(41);
    ui->horizontalSlider_pre_filter_cap->setValue(31);
    ui->horizontalSlider_SAD_window_size->setValue(9);
    ui->horizontalSlider_min_disparity->setValue(-64);
    ui->horizontalSlider_num_of_disparity->setValue(144);
    ui->horizontalSlider_texture_threshold->setValue(10);
    ui->horizontalSlider_uniqueness_ratio->setValue(15);
    ui->horizontalSlider_speckle_window_size->setValue(15);
    ui->horizontalSlider_speckle_window_size->setValue(15);
    ui->horizontalSlider_speckle_window_size->setValue(0);
    ui->horizontalSlider_speckle_range->setValue(0);
    ui->horizontalSlider_disp_12_max_diff->setValue(-1);
    // Adjust P1 and P2 values auto
    int value = ui->horizontalSlider_SAD_window_size->value();
    ui->horizontalSlider_P2->setValue(96*value*value);
    ui->horizontalSlider_P1->setValue(24*value*value);
    ui->doubleSpinBox_lambda->setValue(8000);
    ui->doubleSpinBox_sigma->setValue(1.5);
    // SGBM modes in the drop box
    ui->mode->addItem("SGBM", QVariant(0));
    ui->mode->addItem("HH", QVariant(1));
    ui->mode->addItem("SGBM_3WAY", QVariant(2));
    ui->mode->addItem("HH4", QVariant(3));
    ui->mode->setCurrentIndex(2);

    ui->label_depth_map->setBackgroundRole(QPalette::Base);
    ui->label_depth_map->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_depth_map->setScaledContents(true);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    computed = false; // depthmap not yet computed
    parameters = false; // XML file not loaded
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ShowDepthmap() {

    if (!computed)
        return;

    QPixmap D;
    if (ui->checkBox_fit->isChecked()) { // Fit to the display area
        D = Mat2QPixmapResized(disp_color, ui->scrollArea->width(), ui->scrollArea->height()); // Display depthmap
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Make scrollbars disappear
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else { // Zoom = 100%
        D = Mat2QPixmapResized(disp_color, disp_color.cols-18, disp_color.rows-18);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Make scrollbars appear
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
    ui->label_depth_map->setFixedWidth(D.width()); // Resize the depthmap label
    ui->label_depth_map->setFixedHeight(D.height());
    ui->scrollAreaWidgetContents->setFixedWidth(D.width()); // Resize the container
    ui->scrollAreaWidgetContents->setFixedHeight(D.height());
    ui->label_depth_map->setPixmap(D); // Set the image content
}

//// COMPUTE DEPTHMAP

void MainWindow::on_Compute_clicked() { // compute the depth map

    // check that both images have been loaded
    if (this->left_image.empty() || this->right_image.empty())
        return;

    // check that both images have the same size
    if (left_image.rows != right_image.rows || left_image.cols != right_image.cols) {
        ui->label_depth_map->setText("Can't compute depth map: left and right images should be the same size");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor); // Waiting cursor

    // Initialize variables to compute
    cv::String algo; // stereo matching method (bm or sgbm)
    if (ui->radioButton_BM->isChecked()) algo = "bm";
    if (ui->radioButton_SGBM->isChecked()) algo = "sgbm";
    int max_disp = ui->horizontalSlider_num_of_disparity->value();
    int wsize = ui->horizontalSlider_SAD_window_size->value(); // Window size (SAD)
    int mode = ui->mode->itemData(ui->mode->currentIndex()).toInt(); // SGBM 3-way mode
    // post-filtering - Lambda is a parameter defining the amount of regularization during filtering.
    // Larger values force filtered disparity map edges to adhere more to source image edges. Typical value is 8000
    double lambda = ui->doubleSpinBox_lambda->value();
    // post-filtering - SigmaColor is a parameter defining how sensitive the filtering process is to source image edges.
    // Large values can lead to disparity leakage through low-contrast edges.
    // Small values can make the filter too sensitive to noise and textures in the source image.
    // Typical values range from 0.8 to 2.0
    double sigma  = ui->doubleSpinBox_sigma->value();
    // used post-filtering (wls_conf or wls_no_conf)
    cv::String filter;
    if (ui->checkBox_confidence->isChecked()) filter = "wls_conf"; else filter = "wls_no_conf";

    // load_views
    cv::Mat left  = left_image;
    cv::Mat right = right_image;

    cv::Mat left_for_matcher, right_for_matcher;
    cv::Mat left_disp,right_disp;
    cv::Mat filtered_disp;
    cv::Mat conf_map = cv::Mat(left.rows,left.cols,CV_8U);
    conf_map = cv::Scalar(255);
    cv::Rect ROI;
    Ptr<cv::ximgproc::DisparityWLSFilter> wls_filter;

    if(filter=="wls_conf") // filtering with confidence (significantly better quality than wls_no_conf)
    {
        left_for_matcher  = left.clone();
        right_for_matcher = right.clone();

        if(algo=="bm")
        {
            Ptr<cv::StereoBM> left_matcher = cv::StereoBM::create(max_disp,wsize);
            left_matcher->setPreFilterSize(ui->horizontalSlider_pre_filter_size->value());  // must be an odd between 5 and 255
            left_matcher->setPreFilterCap(ui->horizontalSlider_pre_filter_cap->value());  // must be within 1 and 63
            left_matcher->setBlockSize(ui->horizontalSlider_SAD_window_size->value());  // must be odd, be within 5..255 and be not larger than image width or height
            left_matcher->setMinDisparity(ui->horizontalSlider_min_disparity->value()); // normally at 0
            left_matcher->setNumDisparities(ui->horizontalSlider_num_of_disparity->value());  // must be > 0 and divisible by 16
            left_matcher->setTextureThreshold(ui->horizontalSlider_texture_threshold->value());  // must be non-negative
            left_matcher->setUniquenessRatio(ui->horizontalSlider_uniqueness_ratio->value());  // must be non-negative
            left_matcher->setSpeckleWindowSize(ui->horizontalSlider_speckle_window_size->value()); // 0 to disable, set to 50-200
            left_matcher->setSpeckleRange(ui->horizontalSlider_speckle_range->value()); // 1 or 2 is good, 0 to disable
            left_matcher->setDisp12MaxDiff(ui->horizontalSlider_disp_12_max_diff->value()); // negative = disabled

            wls_filter = cv::ximgproc::createDisparityWLSFilter(left_matcher);
            Ptr<cv::StereoMatcher> right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

            cv::cvtColor(left_for_matcher,  left_for_matcher,  cv::COLOR_BGR2GRAY); // Convert to gray, needed by bm function
            cv::cvtColor(right_for_matcher, right_for_matcher, cv::COLOR_BGR2GRAY);

            left_matcher-> compute(left_for_matcher, right_for_matcher,left_disp);
            right_matcher->compute(right_for_matcher,left_for_matcher, right_disp);
        }
        else if(algo=="sgbm")
        {
            Ptr<cv::StereoSGBM> left_matcher  = cv::StereoSGBM::create(0,max_disp,wsize);
            left_matcher->setPreFilterCap(ui->horizontalSlider_pre_filter_size->value());  // must be within 1 and 63
            left_matcher->setBlockSize(ui->horizontalSlider_SAD_window_size->value());  // must be odd, be within 5..255 and be not larger than image width or height
            left_matcher->setMinDisparity(ui->horizontalSlider_min_disparity->value()); // normally at 0
            left_matcher->setNumDisparities(ui->horizontalSlider_num_of_disparity->value());  // must be > 0 and divisible by 16
            left_matcher->setUniquenessRatio(ui->horizontalSlider_uniqueness_ratio->value());  // must be non-negative
            left_matcher->setSpeckleWindowSize(ui->horizontalSlider_speckle_window_size->value()); // 0 to disable, set to 50-200
            left_matcher->setSpeckleRange(ui->horizontalSlider_speckle_range->value()); // 1 or 2 is good, 0 to disable
            left_matcher->setDisp12MaxDiff(ui->horizontalSlider_disp_12_max_diff->value()); // negative = disabled
            left_matcher->setP1(ui->horizontalSlider_P1->value());
            left_matcher->setP2(ui->horizontalSlider_P2->value());
            left_matcher->setMode(mode);

            wls_filter =  cv::ximgproc::createDisparityWLSFilter(left_matcher);
            Ptr<cv::StereoMatcher> right_matcher = cv::ximgproc::createRightMatcher(left_matcher);

            left_matcher-> compute(left_for_matcher, right_for_matcher,left_disp);
            right_matcher->compute(right_for_matcher,left_for_matcher, right_disp);
        }

        // filtering
        wls_filter->setLambda(lambda);
        wls_filter->setSigmaColor(sigma);
        wls_filter->filter(left_disp,left,filtered_disp,right_disp);

        conf_map = wls_filter->getConfidenceMap();

        ROI = wls_filter->getROI(); // Get the ROI that was used in the last filter call
    }
    else if(filter=="wls_no_conf")
    {
        /* There is no convenience function for the case of filtering with no confidence, so we
        will need to set the ROI and matcher parameters manually */

        left_for_matcher  = left.clone();
        right_for_matcher = right.clone();

        if(algo=="bm")
        {
            Ptr<cv::StereoBM> matcher  = cv::StereoBM::create(max_disp,wsize);
            matcher->setPreFilterSize(ui->horizontalSlider_pre_filter_size->value());  // must be an odd between 5 and 255
            matcher->setPreFilterCap(ui->horizontalSlider_pre_filter_cap->value());  // must be within 1 and 63
            matcher->setBlockSize(ui->horizontalSlider_SAD_window_size->value());  // must be odd, be within 5..255 and be not larger than image width or height
            matcher->setMinDisparity(ui->horizontalSlider_min_disparity->value()); // normally at 0
            matcher->setNumDisparities(ui->horizontalSlider_num_of_disparity->value());  // must be > 0 and divisible by 16
            matcher->setTextureThreshold(ui->horizontalSlider_texture_threshold->value());  // must be non-negative
            matcher->setUniquenessRatio(ui->horizontalSlider_uniqueness_ratio->value());  // must be non-negative
            matcher->setSpeckleWindowSize(ui->horizontalSlider_speckle_window_size->value()); // 0 to disable, set to 50-200
            matcher->setSpeckleRange(ui->horizontalSlider_speckle_range->value()); // 1 or 2 is good, 0 to disable
            matcher->setDisp12MaxDiff(ui->horizontalSlider_disp_12_max_diff->value()); // negative = disabled

            cv::cvtColor(left_for_matcher,  left_for_matcher, cv::COLOR_BGR2GRAY); // Convert to gray, needed by bm function
            cv::cvtColor(right_for_matcher, right_for_matcher, cv::COLOR_BGR2GRAY);

            ROI = computeROI(left_for_matcher.size(),matcher);

            wls_filter = cv::ximgproc::createDisparityWLSFilterGeneric(false);
            wls_filter->setDepthDiscontinuityRadius((int)ceil(0.33*wsize));

            matcher->compute(left_for_matcher,right_for_matcher,left_disp);
        }
        else if(algo=="sgbm")
        {
            Ptr<cv::StereoSGBM> matcher  = cv::StereoSGBM::create(0,max_disp,wsize);
            matcher->setPreFilterCap(ui->horizontalSlider_pre_filter_size->value());  // must be within 1 and 63
            matcher->setBlockSize(ui->horizontalSlider_SAD_window_size->value());  // must be odd, be within 5..255 and be not larger than image width or height
            matcher->setMinDisparity(ui->horizontalSlider_min_disparity->value()); // normally at 0
            matcher->setNumDisparities(ui->horizontalSlider_num_of_disparity->value());  // must be > 0 and divisible by 16
            matcher->setUniquenessRatio(ui->horizontalSlider_uniqueness_ratio->value());  // must be non-negative
            matcher->setSpeckleWindowSize(ui->horizontalSlider_speckle_window_size->value()); // 0 to disable, set to 50-200
            matcher->setSpeckleRange(ui->horizontalSlider_speckle_range->value()); // 1 or 2 is good, 0 to disable
            matcher->setDisp12MaxDiff(ui->horizontalSlider_disp_12_max_diff->value()); // negative = disabled
            matcher->setP1(ui->horizontalSlider_P1->value());
            matcher->setP2(ui->horizontalSlider_P2->value());
            matcher->setMode(mode);

            ROI = computeROI(left_for_matcher.size(),matcher);

            wls_filter = cv::ximgproc::createDisparityWLSFilterGeneric(false);
            wls_filter->setDepthDiscontinuityRadius((int)ceil(0.5*wsize));

            matcher->compute(left_for_matcher,right_for_matcher,left_disp);
        }

        // filtering
        wls_filter->setLambda(lambda);
        wls_filter->setSigmaColor(sigma);
        wls_filter->filter(left_disp,left,filtered_disp,Mat(),ROI);
    }

    // Visualization
    cv::Mat raw_disp_vis;
    getDisparityVis(left_disp,raw_disp_vis,1);
    Mat filtered_disp_vis;
    getDisparityVis(filtered_disp,filtered_disp_vis,1);

    computed = true; // Indicate that a depthmap exists

    // convert the image to a QPixmap and display it
    cv::cvtColor(filtered_disp_vis, disp_color, CV_GRAY2RGB); // convert to color, better for saving the file
    ShowDepthmap();

    // Reprojection 3D
    //Mat project3D;
    //cv::reprojectImageTo3D(filtered_disp, project3D, saveQ, true, CV_32F);
    //project3D.convertTo(project3D, CV_8U);
    //ui->label_3D->setPixmap(Mat2QPixmapResized(project3D, ui->label_3D->width(),ui->label_3D->height()));

    QApplication::restoreOverrideCursor(); // Restore cursor
}

Rect MainWindow::computeROI(Size2i src_sz, Ptr<StereoMatcher> matcher_instance) // used for filtering
{
    int min_disparity = matcher_instance->getMinDisparity();
    int num_disparities = matcher_instance->getNumDisparities();
    int block_size = matcher_instance->getBlockSize();

    int bs2 = block_size/2;
    int minD = min_disparity, maxD = min_disparity + num_disparities - 1;

    int xmin = maxD + bs2;
    int xmax = src_sz.width + minD - bs2;
    int ymin = bs2;
    int ymax = src_sz.height - bs2;

    Rect r(xmin, ymin, xmax - xmin, ymax - ymin);
    return r;
}

/////////////////// Sliders management (callbacks and constraints) //////////////////////

///// pre filter size
void MainWindow::on_horizontalSlider_pre_filter_size_valueChanged(int value)
{
    if ((value % 2) == 0) { // must be an odd number
        value -= 1;
        ui->horizontalSlider_pre_filter_size->setValue(value);
    }
}

///// SAD window size
void MainWindow::set_SADWindowSize() {
    int value = 255;  // max value allowed

    // we check that the value is not bigger than the size of the pictures
    if (! left_image.empty())
        value = std::min(value, std::min(left_image.cols, left_image.rows));
    if (! right_image.empty())
        value = std::min(value, std::min(right_image.cols, right_image.rows));

    // we check that the value is >= 5
    value = std::max(value, 5);

    ui->horizontalSlider_SAD_window_size->setMaximum(value);
}

void MainWindow::on_horizontalSlider_SAD_window_size_valueChanged(int value)
{
    if ((value % 2) == 0) { // must be an odd number
        value -= 1;
        ui->horizontalSlider_SAD_window_size->setValue(value);
    }

    // Adjust P1 and P2 values auto
    ui->horizontalSlider_P2->setValue(96*value*value);
    ui->horizontalSlider_P1->setValue(24*value*value);
}

///// Number of disparities
void MainWindow::on_horizontalSlider_num_of_disparity_sliderMoved(int value)
{
    set_num_of_disparity_slider_to_multiple_16(value); // allow only values that are divisible by 16
}

void MainWindow::on_horizontalSlider_num_of_disparity_valueChanged(int value)
{
    set_num_of_disparity_slider_to_multiple_16(value); //allow only values that are divisible by 16
}

void MainWindow::set_num_of_disparity_slider_to_multiple_16(int value) {
    if ((value % 16) != 0) {
        value -= (value % 16);
        ui->horizontalSlider_num_of_disparity->setValue(value);
    }
}

///// SGBM P1
void MainWindow::on_horizontalSlider_P1_valueChanged(int value)
{
    if (ui->horizontalSlider_P2->value() <= value) {
        value = ui->horizontalSlider_P2->value() - 1;
        ui->horizontalSlider_P1->setValue(value);
    }
}

///// SGBM P2
void MainWindow::on_horizontalSlider_P2_valueChanged(int value)
{
    if (value <= ui->horizontalSlider_P1->value()) {
        value = ui->horizontalSlider_P1->value() + 1;
        ui->horizontalSlider_P2->setValue(value);
    }
}

///// BM
void MainWindow::on_radioButton_BM_clicked ()
{
    if (ui->radioButton_BM->isChecked())
    {
        ui->horizontalSlider_P1->setVisible(false);
        ui->label_P1->setVisible(false);
        ui->label_11->setVisible(false);
        ui->horizontalSlider_P2->setVisible(false);
        ui->label_P2->setVisible(false);
        ui->label_12->setVisible(false);
        ui->mode->setVisible(false);
        ui->label_mode->setVisible(false);
    }
}

///// SGBM
void MainWindow::on_radioButton_SGBM_clicked ()
{
    if (ui->radioButton_SGBM->isChecked())
    {
        ui->horizontalSlider_P1->setVisible(true);
        ui->label_P1->setVisible(true);
        ui->label_11->setVisible(true);
        ui->horizontalSlider_P2->setVisible(true);
        ui->label_P2->setVisible(true);
        ui->label_12->setVisible(true);
        ui->mode->setVisible(true);
        ui->label_mode->setVisible(true);
    }
}

//// Unused
void MainWindow::on_horizontalSlider_pre_filter_cap_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_min_disparity_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_texture_threshold_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_uniqueness_ratio_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_speckle_window_size_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_speckle_range_valueChanged(int value) {};
void MainWindow::on_horizontalSlider_disp_12_max_diff_valueChanged(int value) {};

///// Other functions in the gui

void MainWindow::on_Camera_clicked () { // load stereo camera matrix

    int ref_width, ref_height, ref_size;
    double error;
    // Read calibration results from XML file
    QString filename = QFileDialog::getOpenFileName(this, "Select XML calibration file", ".", "XML parameters (*.xml *.XML)");
    if (filename.isNull())
        return;

    std::string filename_stereo = filename.toUtf8().constData(); // convert filename from QString to std::string

    // Read file
    FileStorage fs_stereo(filename_stereo, FileStorage::READ);
    fs_stereo["BoardWidth"] >> ref_width;
    fs_stereo["BoardHeight"] >> ref_height;
    fs_stereo["PatternSize"] >> ref_size;
    fs_stereo["KL"] >> KL;
    fs_stereo["KR"] >> KR;
    fs_stereo["DL"] >> DL;
    fs_stereo["DR"] >> DR;
    fs_stereo["R"] >> R;
    fs_stereo["T"] >> T;
    fs_stereo["E"] >> E;
    fs_stereo["F"] >> F;
    fs_stereo["ErrorStereo"] >> error;

    QMessageBox::information(this, "Calibration file loaded", "Average error : " + QString::number(error)); // Display useful info about matrix quality

    parameters = true;
}

void MainWindow::on_Rectify_clicked () {

    if (this->left_image.empty() || this->right_image.empty()) // check that both images have been loaded
            return;
    if (!parameters) // XML file not loaded
            return;

    // Rectify stereo
    Mat R1, R2, P1, P2, Q;
    int flags = CALIB_ZERO_DISPARITY;
    double alpha = ui->doubleSpinBox_alpha->value();
    cv::Size size = left_image.size();
    stereoRectify(KL, DL, KR, DR, size, R, T, R1, R2, P1, P2, Q, flags, alpha);

    // Init undistortion
    Mat map11, map12, map21, map22;
    initUndistortRectifyMap(KL, DL, R1, P1, size, CV_32FC2, map11, map12);
    initUndistortRectifyMap(KR, DR, R2, P2, size, CV_32FC2, map21, map22);

    // Remap images
    flags = INTER_LINEAR;
    int flags2 = BORDER_CONSTANT;
    remap(left_image, left_image, map11, map12, flags, flags2);
    remap(right_image, right_image, map21, map22, flags, flags2);

    // Display rectified images
    QPixmap L, R;
    L = Mat2QPixmapResized(left_image, ui->label_image_left->width(), ui->label_image_left->height());
    R = Mat2QPixmapResized(right_image, ui->label_image_right->width(), ui->label_image_right->height());
    ui->label_image_left->setPixmap(L);
    ui->label_image_right->setPixmap(R);

    rectified = true;
}

void MainWindow::on_MPO_clicked () {

    QString filename = QFileDialog::getOpenFileName(this, "Select MPO image file", ".", "MPO image (*.mpo *.MPO)");
    if (filename.isNull())
        return;
    basename = filename.toUtf8().constData(); // basename is used after to save other files
    basename = basename.substr(0, basename.size()-4);

    bool success;
    vector<cv::Mat> matImages(2);
    success = MPO2Mat(filename, matImages); // Load MPO file

    if (!success)
        return;

    // display images
    left_image = matImages[0];
    right_image = matImages[1];
    QPixmap L, R;
    L = Mat2QPixmapResized(left_image, ui->label_image_left->width(), ui->label_image_left->height());
    R = Mat2QPixmapResized(right_image, ui->label_image_right->width(), ui->label_image_right->height());
    ui->label_image_left->setPixmap(L);
    ui->label_image_right->setPixmap(R);
    ui->horizontalSlider_min_disparity->setMaximum(left_image.cols / 4);
    ui->horizontalSlider_min_disparity->setMinimum(-left_image.cols / 4);

    computed = false;
    rectified = false;
}

void MainWindow::on_Negative_clicked () { // negative colors of the depthmap

    if (this->disp_color.empty()) // check that depthmap exists
            return;

    cv::bitwise_not(disp_color,disp_color); // negative depthmap

    // display depthmap
    ShowDepthmap();
}

void MainWindow::on_Save_depthmap_clicked() { // save depthmap in PNG format

    if (this->disp_color.empty()) // check that depthmap exists
            return;

    QString filename = QFileDialog::getSaveFileName(this, "Save depthmap to image...", "./" + QString::fromStdString(basename + "-depthmap") + ".png", NULL);
    std::string filename_s = filename.toUtf8().constData();

    if (filename.isNull() || filename.isEmpty())
        return;

    cv::imwrite(filename_s, disp_color); // save depthmap
}

void MainWindow::on_Save_Images_clicked() { // save both rectified images to PNG format

    if (this->left_image.empty() || this->right_image.empty()) // check both images have been loaded
            return;

    QString filename = QFileDialog::getSaveFileName(this, "Save images (base name)...", "./" + QString::fromStdString(basename), NULL);

    if (filename.isNull() || filename.isEmpty())
        return;

    // save the images
    std::string filename_L = filename.toUtf8().constData() + std::string("-Rectified-L.png");
    cv::imwrite(filename_L, left_image);
    std::string filename_R = filename.toUtf8().constData() + std::string("-Rectified-R.png");
    cv::imwrite(filename_R, right_image);
}

void MainWindow::on_Rotate_clicked() { // rotates the left and right images by 90°

    if (this->left_image.empty() || this->right_image.empty()) // check both images have been loaded
            return;

    rotate(left_image, left_image, ROTATE_90_CLOCKWISE); // Others are ROTATE_180 and ROTATE_90_COUNTERCLOCKWISE
    rotate(right_image, right_image, ROTATE_90_CLOCKWISE);

    QPixmap L, R;
    L = Mat2QPixmapResized(left_image, ui->label_image_left->width(), ui->label_image_left->height());
    R = Mat2QPixmapResized(right_image, ui->label_image_right->width(), ui->label_image_right->height());
    ui->label_image_left->setPixmap(L);
    ui->label_image_right->setPixmap(R);
}

void MainWindow::on_Swap_clicked() { // Swap left and right image

    if (this->left_image.empty() || this->right_image.empty()) // check both images have been loaded
            return;

    Mat temp;
    temp = left_image;
    left_image = right_image;
    right_image = temp;

    QPixmap L, R;
    L = Mat2QPixmapResized(left_image, ui->label_image_left->width(), ui->label_image_left->height());
    R = Mat2QPixmapResized(right_image, ui->label_image_right->width(), ui->label_image_right->height());
    ui->label_image_left->setPixmap(L);
    ui->label_image_right->setPixmap(R);
}

void MainWindow::on_Normalize_clicked() { // normalize depthmap

    if (!computed)
        return;

    cv::normalize(disp_color, disp_color, 0, 255, NORM_MINMAX, -1);

     // display depthmap
     ShowDepthmap();
}

void MainWindow::on_checkBox_fit_clicked() // Loads left image
{
    ShowDepthmap();
}

void MainWindow::on_Left_clicked() // Loads left image
{
    QString filename = QFileDialog::getOpenFileName(this, "Select left picture file", ".", "Images (*.jpg *.JPG *.jpeg *.JPEG *.jp2 *.JP2 *.png *.PNG *.tif *.TIF *.tiff *.TIFF *.bmp *.BMP)");
    if (filename.isNull() || filename.isEmpty())
        return;
    basename = filename.toUtf8().constData(); // basename is used after to save other files
    basename = basename.substr(0, basename.size()-6);
    std::string filename_s = filename.toUtf8().constData();

    cv::Mat mat = cv::imread(filename_s, CV_LOAD_IMAGE_COLOR); // Load image
    ui->label_image_left->setPixmap(Mat2QPixmapResized(mat, ui->label_image_left->width(), ui->label_image_left->height())); // Display left image

    left_image = mat; // stores the image for further use

    set_SADWindowSize(); // adapt parameters automatically

    computed = false; // depthmap not yet computed
    rectified = false;

    ui->label_depth_map->setPixmap(QPixmap()); // Delete the depthmap image
    ui->label_depth_map->setText("Depthmap not yet computed"); // Text in the depthmap area
    ui->horizontalSlider_min_disparity->setMaximum(left_image.cols / 4);
    ui->horizontalSlider_min_disparity->setMinimum(-left_image.cols / 4);
}

void MainWindow::on_Right_clicked() // Loads right image
{
    QString filename = QFileDialog::getOpenFileName(this, "Select right picture file", ".", "Images (*.jpg *.JPG *.jpeg *.JPEG *.jp2 *.JP2 *.png *.PNG *.tif *.TIF *.tiff *.TIFF *.bmp *.BMP)");

    if (filename.isNull() || filename.isEmpty())
        return;
    basename = filename.toUtf8().constData();
    basename = basename.substr(0, basename.size()-6);
    std::string filename_s = filename.toUtf8().constData();

    cv::Mat mat = cv::imread(filename_s, CV_LOAD_IMAGE_COLOR); // Load image
    ui->label_image_right->setPixmap(Mat2QPixmapResized(mat, ui->label_image_right->width(), ui->label_image_right->height())); // Display right image

    right_image = mat; // stores the image for further use

    set_SADWindowSize(); // adapt parameters automatically

    computed = false; // depthmap not yet computed
    rectified = false;

    ui->label_depth_map->setPixmap(QPixmap()); // Delete the depthmap image
    ui->label_depth_map->setText("Depthmap not yet computed"); // Text in the depthmap area
    ui->horizontalSlider_min_disparity->setMaximum(right_image.cols / 4);
    ui->horizontalSlider_min_disparity->setMinimum(-right_image.cols / 4);
}

void MainWindow::on_Disparity_clicked() // Open disparity window
{
    if (left_image.empty() || right_image.empty()) // check both images have been loaded AND rectified
        return;

    if (!rectified) {
        int notRectified = QMessageBox::question(this, "Are you sure ?", "You didn't rectify the images with the camera parameters. Are you sure?", QMessageBox::Yes|QMessageBox::No);
        if (notRectified == QMessageBox::No)
            return;
    }

    Disparity *disp_form = new Disparity(this); // create form window
    disp_form->setLeftImage(left_image); // pass the left & right images variables
    disp_form->setRightImage(right_image);

    int result = disp_form->exec(); // execute the form
    if (result == QDialog::Rejected) // if cancel
        return;

    int min_disparity = disp_form->getBackgroundDisparity(); // get min disparity value
    ui->horizontalSlider_min_disparity->setValue(min_disparity); // set min disparity in the gui
    int num_disparity = std::abs(disp_form->getForegroundDisparity() - min_disparity); // get number of disparity value
    if ((num_disparity % 16) != 0) { // must be divisible by 16
        num_disparity -= (num_disparity % 16); // correct the value
        num_disparity += 16; // add 16 to be sure
    }
    ui->horizontalSlider_num_of_disparity->setValue(num_disparity); // set number of disparity in the gui
}

void MainWindow::on_Adjust_clicked() // Open adjust window
{
    if (left_image.empty() || right_image.empty()) // check both images have been loaded AND rectified
        return;

    if (!rectified) {
        int notRectified = QMessageBox::question(this, "Are you sure ?", "You didn't rectify the images with the camera parameters. Are you sure?", QMessageBox::Yes|QMessageBox::No);
        if (notRectified == QMessageBox::No)
            return;
    }

    Adjust *adj_form = new Adjust(this); // create form window
    adj_form->setLeftImage(left_image); // pass the left & right images variables
    adj_form->setRightImage(right_image);

    int result = adj_form->exec(); // execute the form
    if (result == QDialog::Rejected) // if cancel
        return;

    right_image = adj_form->getResult();
    ui->label_image_right->setPixmap(Mat2QPixmapResized(right_image, ui->label_image_right->width(), ui->label_image_right->height())); // Display right image
}
