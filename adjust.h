/*#-------------------------------------------------
#
#      Disparity and depth maps with OpenCV
#
#    by AbsurdePhoton - www.absurdephoton.fr
#
#                v1.3 - 2019/06/03
#
#-------------------------------------------------*/

#ifndef ADJUST_H
#define ADJUST_H

#include "opencv2/opencv.hpp"

#include <QDialog>

namespace Ui {
class Adjust;
}

class Adjust : public QDialog
{
    Q_OBJECT

public:
    explicit Adjust(QWidget *parent = 0);
    ~Adjust();

    void setLeftImage(cv::Mat left_img);
    void setRightImage(cv::Mat right_img);
    cv::Mat getResult();

private slots:
    void on_horizontalSlider_sliderMoved(int value);
    void on_verticalSlider_sliderMoved(int value);
    void on_checkBox_fit_clicked();

private:
    Ui::Adjust *ui;

    cv::Mat left_image, right_image, shifted;
    int decX, decY;

    void DisplayShifted();
};

#endif // ADJUST_H
