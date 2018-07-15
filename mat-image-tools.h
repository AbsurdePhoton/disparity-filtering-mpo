/*
 * File  : mat-image-tools.h
 * Author: AbsurdePhoton
 *
 * v1.1 2018/07/15
 *
 * Converts mat images to QPixmap or QImage
 * Set brightness and contrast to Mat image
 *
 */

#ifndef MAT2IMAGE_H
#define MAT2IMAGE_H

#include "opencv2/opencv.hpp"

using namespace cv;

enum Direction{ShiftUp=1, ShiftRight, ShiftDown, ShiftLeft};

QImage Mat2QImage(cv::Mat const& src);
QPixmap Mat2QPixmap(cv::Mat source);
QPixmap Mat2QPixmapResized(cv::Mat source, int max_width, int max_height);
QImage cvMatToQImage(const cv::Mat &inMat);
Mat BrightnessContrast(Mat source, double alpha, int beta);
Mat shiftFrame(cv::Mat frame, int pixels, Direction direction);

#endif // MAT2IMAGE_H
