/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.3 - 2019/06/03
#
#-------------------------------------------------*/

#include "disparity.h"
#include "ui_disparity.h"

#include "mat-image-tools.h"

#include "opencv2/opencv.hpp"

using namespace cv;

Disparity::Disparity(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Disparity)
{
    ui->setupUi(this);

    background_disparity = 50;
    foreground_disparity = 200;

    ui->label_background_disparity->setBackgroundRole(QPalette::Base);
    ui->label_background_disparity->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_background_disparity->setScaledContents(true);

    ui->label_foreground_disparity->setBackgroundRole(QPalette::Base);
    ui->label_foreground_disparity->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_foreground_disparity->setScaledContents(true);

    ui->scrollArea_background->setBackgroundRole(QPalette::Dark);
    ui->scrollArea_background->setWidgetResizable(true);
    ui->scrollAreaWidgetContents_background->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->scrollArea_foreground->setBackgroundRole(QPalette::Dark);
    ui->scrollArea_foreground->setWidgetResizable(true);
    ui->scrollAreaWidgetContents_foreground->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
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
        shift = ShiftFrame(right_image, background_disparity, shift_right);
    else shift = ShiftFrame(right_image, -background_disparity, shift_left);

    cv::addWeighted(left_image, 0.5, shift, 0.5, 0, result, -1);

    QPixmap D;
    if (ui->checkBox_fit->isChecked()) { // Fit to the display area
        D = Mat2QPixmapResized(result, ui->scrollArea_background->width(), ui->scrollArea_background->height(), true);
        ui->scrollArea_background->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Make scrollbars appear
        ui->scrollArea_background->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else { // Zoom = 100%
        D = Mat2QPixmap(result);
        ui->scrollArea_background->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Make scrollbars appear
        ui->scrollArea_background->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }

    ui->label_background_disparity->setFixedWidth(D.width()); // Resize the depthmap label
    ui->label_background_disparity->setFixedHeight(D.height());
    ui->scrollAreaWidgetContents_background->setFixedWidth(D.width()); // Resize the container
    ui->scrollAreaWidgetContents_background->setFixedHeight(D.height());

    ui->label_background_disparity->setPixmap(D);
}

void Disparity::DisplayShiftedForeground() {
    if (left_image.empty() || right_image.empty()) // check both images have been loaded
        return;

    Mat shift, result;

    if (foreground_disparity >= 0) shift = ShiftFrame(right_image, foreground_disparity, shift_right);
        else shift = ShiftFrame(right_image, -foreground_disparity, shift_left);

    cv::addWeighted(left_image, 0.5, shift, 0.5, 0, result, -1);

    QPixmap D;
    if (ui->checkBox_fit->isChecked()) { // Fit to the display area
        D = Mat2QPixmapResized(result, ui->scrollArea_foreground->width(), ui->scrollArea_foreground->height(), true);
        ui->scrollArea_foreground->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Make scrollbars appear
        ui->scrollArea_foreground->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else // Zoom = 100%
    {
        D = Mat2QPixmap(result);
        ui->scrollArea_foreground->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Make scrollbars appear
        ui->scrollArea_foreground->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }

    ui->label_foreground_disparity->setFixedWidth(D.width()); // Resize the depthmap label
    ui->label_foreground_disparity->setFixedHeight(D.height());
    ui->scrollAreaWidgetContents_foreground->setFixedWidth(D.width()); // Resize the container
    ui->scrollAreaWidgetContents_foreground->setFixedHeight(D.height());

    ui->label_foreground_disparity->setPixmap(D);
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

void Disparity::on_checkBox_fit_clicked() // Loads left image
{
    DisplayShiftedBackground();
    DisplayShiftedForeground();
}
