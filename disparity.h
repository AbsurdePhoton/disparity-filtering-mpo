/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.1 - 2018/07/15
#
#-------------------------------------------------*/

#ifndef DISPARITY_H
#define DISPARITY_H

#include "opencv2/opencv.hpp"

#include <QDialog>

namespace Ui {
class Disparity;
}

class Disparity : public QDialog
{
    Q_OBJECT

public:
    explicit Disparity(QWidget *parent = 0);
    ~Disparity();

    void setLeftImage(cv::Mat left_img);
    void setRightImage(cv::Mat right_img);
    int getBackgroundDisparity();
    int getForegroundDisparity();

private slots:
    void on_horizontalSlider_background_disparity_sliderMoved(int value);
    void on_horizontalSlider_foreground_disparity_sliderMoved(int value);

private:
    Ui::Disparity *ui;

    cv::Mat left_image, right_image;
    int background_disparity, foreground_disparity;

    void DisplayShiftedBackground();
    void DisplayShiftedForeground();
};

#endif // DISPARITY_H
