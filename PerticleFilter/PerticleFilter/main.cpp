//
//  main.cpp
//  frameDeff
//
//  Created by Ryunosuke Kirikihira on 2014/09/24.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <random>
#include <ctype.h>
#include <math.h>
#include <opencv2/legacy/legacy.hpp>

using namespace cv;
using namespace std;

/**
 *  尤度の計算
 */
float likelihood (IplImage * img, int x, int y){
    float b, g, r;
    float dist = 0.0, sigma = 50.0;
    b = img->imageData[img->widthStep * y + x * 3]; //B
    g=img->imageData[img->widthStep*y+x*3+1]; //G
    r=img->imageData[img->widthStep*y+x*3+2]; //R
    dist = sqrt (b * b + g * g + (255.0 - r) * (255.0 - r));
    return 1.0 / (sqrt (2.0 * CV_PI) * sigma) * expf (-dist * dist / (2.0 * sigma * sigma));
}


/**
 *  Main
 */
int main (int argc, char **argv){
    double w = 0.0, h = 0.0;
    Mat small, dst_img;
    int n_stat = 4;
    int n_particle = 100;
    CvConDensation *cond = 0;
    CvMat *lowerBound = 0;
    CvMat *upperBound = 0;
    
    cv::VideoCapture capture = VideoCapture("/Users/Kiri/Desktop/test.mp4");
    
    Mat frame;
    capture >> frame;
    w = frame.cols;
    h = frame.rows;
    cond = cvCreateConDensation (n_stat, 0, n_particle);

    cond->DynamMatr[1] = 0.0;
    cond->DynamMatr[2] = 1.0;
    cond->DynamMatr[3] = 0.0;
    cond->DynamMatr[4] = 0.0;
    cond->DynamMatr[5] = 1.0;
    cond->DynamMatr[6] = 0.0;
    cond->DynamMatr[7] = 1.0;
    cond->DynamMatr[8] = 0.0;
    cond->DynamMatr[9] = 0.0;
    cond->DynamMatr[10] = 1.0;
    cond->DynamMatr[11] = 0.0;
    cond->DynamMatr[12] = 0.0;
    cond->DynamMatr[13] = 0.0;
    cond->DynamMatr[14] = 0.0;
    cond->DynamMatr[15] = 1.0;
    
    cvmSet (lowerBound, 0, 0, 0.0);
    cvmSet (lowerBound, 1, 0, 0.0);
    cvmSet (lowerBound, 2, 0, -10.0);
    cvmSet (lowerBound, 3, 0, -10.0);
    cvmSet (upperBound, 0, 0, w);
    cvmSet (upperBound, 1, 0, h);
    cvmSet (upperBound, 2, 0, 10.0);
    cvmSet (upperBound, 3, 0, 10.0);
    
    while (1) {
        capture >> frame;
        // 各パーティクルについて尤度を計算
        for (int i = 0; i < n_particle; i++) {
            int xx, yy;
            xx = (int) (cond->flSamples[i][0]);
            yy = (int) (cond->flSamples[i][1]);
            if (xx < 0 || xx >= w || yy < 0 || yy >= h) {
                cond->flConfidence[i] = 0.0; }
            else {
                IplImage* image;
                image = cvCreateImage(cvSize(frame.cols,frame.rows),8,3);
                IplImage ipltemp=frame;
                cvCopy(&ipltemp,image);
                cond->flConfidence[i] = likelihood (image, xx, yy);
                circle (frame, cvPoint (xx, yy), 2, CV_RGB (0, 0, 255), -1);
            }
            
            small = frame;
            resize(small, dst_img, cv::Size(), 0.5, 0.5);
            imshow("Condensation", frame);
            imshow("hoge", dst_img);
            
            cvConDensUpdateByTime (cond);
        }
    }
    return 0;
}
