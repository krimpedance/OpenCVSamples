//
//  main.cpp
//  frameDeff
//
//  Created by Ryunosuke Kirikihira on 2014/09/24.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//

#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define THRESHOLD 40

using namespace cv;
using namespace std;

int main(int argc, const char * argv[]) {
    
    VideoCapture capture = VideoCapture("/Users/Kiri/Desktop/framediff.mov");

    Mat previousFrame;
    capture >> previousFrame;
    
    int frameCount = 0; // 現在までのフレーム数
    int detectCount = 0; // 物の動きを連続で検出した数
    while (waitKey(1) == -1)
    {
        Mat frame,drawFrame;
        int x, y;
        uchar p1[3], p2[3];
        
        // 画像取得
        capture >> frame;
        
        if (frame.empty())
            break;
        
        drawFrame = Mat::ones(previousFrame.rows, previousFrame.cols, CV_8UC1);
     
        int diffCount = 0; // 差分のある画像数
        for (y = previousFrame.rows/2; y < previousFrame.rows; y++) {
            for (x = 0 ; x < previousFrame.cols/2; x++) {
                for (int i=0; i<previousFrame.channels(); i++) {
                    p1[i] = previousFrame.data[previousFrame.step*y + previousFrame.elemSize()*x + i];  //1枚目の画素値
                    p2[i] = frame.data[frame.step*y + frame.elemSize()*x + i];  //2枚目の画素値
                }
                
                if ( abs(p1[0]-p2[0]) > THRESHOLD || abs(p1[1]-p2[1]) > THRESHOLD || abs(p1[2]-p2[2]) > THRESHOLD ) {   /*もし違う色なら黒にする*/
                    for (int i=0; i<previousFrame.channels(); i++) {
                        drawFrame.data[drawFrame.step*y + drawFrame.elemSize()*x + i] = p2[i];
                        diffCount++;
                    }
                }else{ /*もし同じ色なら白にする*/
                    for (int i=0; i<previousFrame.channels(); i++) {
                        drawFrame.data[drawFrame.step*y + drawFrame.elemSize()*x + i] = 255;
                    }
                }
            }
        }
        
        imshow("output", drawFrame);
        imshow("input", frame);
        
        // 差分のある画素数が一定以上ならものが動いたと判断
        if (diffCount > 10000) {
            detectCount++;
        } else {
            detectCount = 0;
        }
        
        // 連続して10フレーム判断されていたら侵入者と断定
        if (detectCount >= 10) {
            printf("侵入者!!\n");
        }
        
        
        // 差分を行うフレームを更新
        frameCount ++;
        if(frameCount == 10) {
            previousFrame = frame;
            frameCount = 0;
        }
    }
    
    return 0;
}
