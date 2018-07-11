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

using namespace std;
using namespace cv;

bool MPO2Pixmap(QString source, vector<QPixmap>& vector);
bool MPO2Mat(QString source, vector<cv::Mat>& vector);

#endif // MPO_H
