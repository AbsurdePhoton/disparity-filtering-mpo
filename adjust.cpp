/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.3 - 2019/06/03
#
#-------------------------------------------------*/

#include "adjust.h"
#include "ui_adjust.h"

#include "mat-image-tools.h"

#include "opencv2/opencv.hpp"

using namespace cv;

Adjust::Adjust(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Adjust)
{
    ui->setupUi(this);

    decX = 0;
    decY = 0;

    ui->label_view->setBackgroundRole(QPalette::Base);
    ui->label_view->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_view->setScaledContents(true);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

Adjust::~Adjust()
{
    delete ui;
}

void Adjust::DisplayShifted() {
    if (left_image.empty() || right_image.empty()) // check both images have been loaded
        return;

    Mat shiftX, shiftY, result;

    if (decX < 0) shiftX = ShiftFrame(right_image, std::abs(decX), shift_left);
    else if (decX > 0) shiftX = ShiftFrame(right_image, std::abs(decX), shift_right);
        else shiftX = right_image;

    if (decY < 0) shiftY = ShiftFrame(shiftX, std::abs(decY), shift_up);
    else if (decY > 0) shiftY = ShiftFrame(shiftX, std::abs(decY), shift_down);
        else shiftY = shiftX;

    shifted = shiftY;

    cv::addWeighted(left_image, 0.5, shiftY, 0.5, 0, result, -1);

    QPixmap D;
    if (ui->checkBox_fit->isChecked()) { // Fit to the display area
        D = Mat2QPixmapResized(result, ui->scrollArea->width(), ui->scrollArea->height(), true);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Make scrollbars appear
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else // Zoom = 100%
    {
        D = Mat2QPixmap(result);
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Make scrollbars appear
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }

    ui->label_view->setFixedWidth(D.width()); // Resize the depthmap label
    ui->label_view->setFixedHeight(D.height());
    ui->scrollAreaWidgetContents->setFixedWidth(D.width()); // Resize the container
    ui->scrollAreaWidgetContents->setFixedHeight(D.height());

    ui->label_view->setPixmap(D);
}

void Adjust::setLeftImage(cv::Mat left_img) {
    left_image = left_img;
    DisplayShifted();
}

void Adjust::setRightImage(cv::Mat right_img) {
    right_image = right_img;
    DisplayShifted();
}

void Adjust::on_horizontalSlider_sliderMoved(int value) {
    decX = value;
    DisplayShifted();
    qApp->processEvents();
}

void Adjust::on_verticalSlider_sliderMoved(int value) {
    decY = value;
    DisplayShifted();
    qApp->processEvents();
}

void Adjust::on_checkBox_fit_clicked() // Loads left image
{
    DisplayShifted();
}

cv::Mat Adjust::getResult() {
    return shifted;
}
