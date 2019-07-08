/*
 * File  : mpo.h
 * Author: AbsurdePhoton
 *
 * v1 2018/07/08
 *
 * Reads MPO stereo image files
 *
 */

#ifndef MPO_H
#define MPO_H

#include "opencv2/opencv.hpp"

bool MPO2Pixmap(QString source, std::vector<QPixmap>& vector);
bool MPO2Mat(QString source, std::vector<cv::Mat>& vector);

#endif // MPO_H
