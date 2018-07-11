/*
 * File  : mat-image-tools.h
 * Author: AbsurdePhoton
 *
 * v1 2018/07/08
 *
 * Converts mat images to QPixmap or QImage
 * Set brightness and contrast to Mat image
 *
 */

#ifndef MAT2IMAGE_H
#define MAT2IMAGE_H

#include "opencv2/opencv.hpp"

using namespace cv;

QImage Mat2QImage(cv::Mat const& src);
QPixmap Mat2QPixmap(cv::Mat source);
QPixmap Mat2QPixmapResized(cv::Mat source, int max_width, int max_height);
QImage cvMatToQImage(const cv::Mat &inMat);
Mat BrightnessContrast(Mat source, double alpha, int beta);

#endif // MAT2IMAGE_H
