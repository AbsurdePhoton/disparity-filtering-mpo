/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.1 - 2018/07/15
#
#-------------------------------------------------*/

#include "disparity.h"
#include "ui_disparity.h"

#include "mat-image-tools.h"

#include "opencv2/opencv.hpp"

Disparity::Disparity(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Disparity)
{
    ui->setupUi(this);

    background_disparity = 50;
    foreground_disparity = 200;
}

Disparity::~Disparity()
{
    delete ui;
}

void Disparity::DisplayShiftedBackground() {
    if (left_image.empty() || right_image.empty()) // check both images have been loaded
        return;

    Mat shift, result;

    if (background_disparity >= 0)
        shift = shiftFrame(right_image, background_disparity, ShiftRight);
    else shift = shiftFrame(right_image, -background_disparity, ShiftLeft);

    cv::addWeighted(left_image, 0.5, shift, 0.5, 0, result, -1);
    ui->label_background_disparity->setPixmap(Mat2QPixmapResized(result, ui->label_background_disparity->width(), ui->label_background_disparity->height()));
}

void Disparity::DisplayShiftedForeground() {
    if (left_image.empty() || right_image.empty()) // check both images have been loaded
        return;

    Mat shift, result;

    if (foreground_disparity >= 0) shift = shiftFrame(right_image, foreground_disparity, ShiftRight);
        else shift = shiftFrame(right_image, -foreground_disparity, ShiftLeft);

    cv::addWeighted(left_image, 0.5, shift, 0.5, 0, result, -1);
    ui->label_foreground_disparity->setPixmap(Mat2QPixmapResized(result, ui->label_foreground_disparity->width(), ui->label_foreground_disparity->height()));
}

void Disparity::setLeftImage(cv::Mat left_img) {
    left_image = left_img;
    DisplayShiftedBackground();
    DisplayShiftedForeground();
    ui->horizontalSlider_background_disparity->setMaximum(left_image.cols / 4);
    ui->horizontalSlider_background_disparity->setMinimum(-left_image.cols / 4);
    ui->horizontalSlider_background_disparity->setValue(background_disparity);
    ui->horizontalSlider_foreground_disparity->setMaximum(left_image.cols / 4);
    ui->horizontalSlider_foreground_disparity->setMinimum(-left_image.cols / 4);
    ui->horizontalSlider_foreground_disparity->setValue(foreground_disparity);
}

void Disparity::setRightImage(cv::Mat right_img) {
    right_image = right_img;
    DisplayShiftedBackground();
    DisplayShiftedForeground();
}

int Disparity::getBackgroundDisparity() {
    return background_disparity;
}

int Disparity::getForegroundDisparity() {
    return foreground_disparity;
}

void Disparity::on_horizontalSlider_background_disparity_sliderMoved(int value) {
    background_disparity = value;
    DisplayShiftedBackground();
    qApp->processEvents();
}

void Disparity::on_horizontalSlider_foreground_disparity_sliderMoved(int value) {
    foreground_disparity = value;
    DisplayShiftedForeground();
    qApp->processEvents();
}
