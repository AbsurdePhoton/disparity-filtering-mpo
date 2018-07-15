/*
 * File  : mat-image-tools.cpp
 * Author: AbsurdePhoton
 *
 * v1.1 2018/07/15
 *
 * Converts mat images to QPixmap or QImage
 * Set brightness and contrast to Mat image
 *
 */

#include <QPixmap>

#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

enum Direction{ShiftUp=1, ShiftRight, ShiftDown, ShiftLeft};

QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp;
     cv::cvtColor(src, temp,CV_BGR2RGB);
     QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     dest.bits(); // enforce deep copy of QImage::QImage ( const uchar * data, int width, int height, Format format )
     return dest;
}

QPixmap Mat2QPixmap(cv::Mat source)
{
    QImage i = Mat2QImage(source);
    QPixmap p = QPixmap::fromImage(i, Qt::AutoColor);
    return p;
}

QPixmap Mat2QPixmapResized(cv::Mat source, int max_width, int max_height)
{
    QImage i = Mat2QImage(source);
    QPixmap p = QPixmap::fromImage(i, Qt::AutoColor);
    return p.scaled(max_width, max_height, Qt::KeepAspectRatio);
}

Mat BrightnessContrast(Mat source, double alpha, int beta) {
    Mat image = Mat::zeros(source.size(), source.type());

    /// Do the operation new_image(i,j) = alpha*image(i,j) + beta

    for (int y = 0; y < source.rows; y++)
    {
    for (int x = 0; x < source.cols; x++)
    {
    for (int c = 0; c < 3; c++)
    {
    image.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(alpha*(source.at<Vec3b>(y, x)[c]) + beta);
    }
    }
    }
    return image;
}

QImage cvMatToQImage(const cv::Mat &inMat)
{
    switch (inMat.type())
    {
        // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image(inMat.data,
            inMat.cols, inMat.rows,
            static_cast<int>(inMat.step),
            QImage::Format_ARGB32);

        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image(inMat.data,
            inMat.cols, inMat.rows,
            static_cast<int>(inMat.step),
            QImage::Format_RGB888);

        return image.rgbSwapped();
    }

    // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION >= 0x050500

        // From Qt 5.5
        QImage image(inMat.data, inMat.cols, inMat.rows,
                     static_cast<int>(inMat.step),
                     QImage::Format_Grayscale8);
#else
        static QVector<QRgb>  sColorTable;

        // only create our color table the first time
        if (sColorTable.isEmpty())
        {
            sColorTable.resize(256);
            for (int i = 0; i < 256; ++i)
            {
                sColorTable[i] = qRgb(i, i, i);
            }
        }

        QImage image(inMat.data,
            inMat.cols, inMat.rows,
            static_cast<int>(inMat.step),
            QImage::Format_Indexed8);

        image.setColorTable(sColorTable);
#endif
    }
    }

    return QImage();
}

Mat shiftFrame(cv::Mat frame, int pixels, Direction direction)
{
    //create a same sized temporary Mat with all the pixels flagged as invalid (-1)
    Mat temp = Mat::zeros(frame.size(), frame.type());

    switch (direction)
    {
    case(ShiftUp) :
        frame(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, 0, temp.cols, temp.rows - pixels)));
        break;
    case(ShiftRight) :
        frame(cv::Rect(0, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)));
        break;
    case(ShiftDown) :
        frame(cv::Rect(0, 0, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)));
        break;
    case(ShiftLeft) :
        frame(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(0, 0, frame.cols - pixels, frame.rows)));
        break;
    default:
        std::cout << "Shift direction is not set properly" << std::endl;
    }
    return temp;
}
