/*
 * File  : mpo.cpp
 * Author: AbsurdePhoton
 *
 * v1 2018/07/08
 *
 * Reads MPO stereo image files
 *
 */

#include <QPixmap>
#include <QFile>

#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

bool MPO2Pixmap(QString source, vector<QPixmap>& vector) {
    int count = 0;
    int index, fileLength;
    int secondImageStart = 0;

    QFile imageFile(source); // open MPO file as datastream
    if (imageFile.exists()) {
        imageFile.open(QIODevice::ReadOnly); // read-only mode
        QDataStream dataInfilestream(&imageFile);
        fileLength = imageFile.size();

        // read data from stream
        QVector<uchar> imageData(fileLength); // data vector
        while(!dataInfilestream.atEnd()) {
            dataInfilestream >> imageData[count++];
        }
        imageFile.close();

        // look for marker of second image at (+/- 10%) half  of the full MPO file
        for (index = (int)(fileLength * 0.40); index < (int)(fileLength * 0.60); index++) {
            if ((imageData[index] == 0xFF) && (imageData[index+1] == 0xD8) \
              && (imageData[index+2] == 0xFF) && (imageData[index+3] == 0xE1)) {
                secondImageStart = index;
                break;
            };
        }
        if (secondImageStart == 0) { // no separator found
            return false;
        }

        // QPixmap variables to store the images
        QPixmap *leftView = new QPixmap;
        QPixmap *rightView = new QPixmap;
        // copy the 2 jpegs in buffers
        leftView->loadFromData(&imageData[0], (secondImageStart - 1));
        rightView->loadFromData(&imageData[secondImageStart], (fileLength - secondImageStart));
        // decode jpegs in pointer array
        std::vector<QPixmap> stereoImages(2);
        stereoImages[0] = *leftView;
        stereoImages[1] = *rightView;
        // return pointer array
        vector = stereoImages;
        return true;
    }
    else return false;
}

bool MPO2Mat(QString source, vector<cv::Mat>& vector)
{
    int count = 0;
    int index, fileLength;
    int secondImageStart = 0;

    QFile imageFile(source); // open MPO file as datastream
    // open if exist
    if (imageFile.exists()) {
        imageFile.open(QIODevice::ReadOnly); // read-only mode
        QDataStream dataInfilestream(&imageFile);
        // get file length
        fileLength = imageFile.size();

        // read data from stream
        QVector<uchar> imageData(fileLength);  // data vector
        while(!dataInfilestream.atEnd()) {
            dataInfilestream >> imageData[count++];
        }
        imageFile.close();

        // look for marker of second image at (+/- 10%) half  of the full MPO file
        for (index = (int)(fileLength * 0.40); index < (int)(fileLength * 0.60); index++) {
            if ((imageData[index] == 0xFF) && (imageData[index+1] == 0xD8) \
              && (imageData[index+2] == 0xFF) && (imageData[index+3] == 0xE1)) {
                secondImageStart = index;
                break;
            }
        }
        if (secondImageStart == 0) { // no separator found
            return false;
        }

        // return buffers : 2 jpegs
        std::vector<cv::Mat> ret(2);
        // copy the 2 jpegs in buffers
        QVector<uchar> leftImageData = imageData.mid(0,secondImageStart - 1);
        QVector<uchar> rightImageData = imageData.mid(secondImageStart,fileLength - secondImageStart);
        // decode jpegs in Mat containers
        ret[0] = imdecode(leftImageData.toStdVector(), IMREAD_COLOR);
        ret[1] = imdecode(rightImageData.toStdVector(), IMREAD_COLOR);
        // return pointer array
        vector = ret;
        // success
        return true;
    }
    else return false;
}
