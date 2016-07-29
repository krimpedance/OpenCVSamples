//
//  main.cpp
//  opticalflow
//
//  Created by Ryunosuke Kirikihira on 2014/08/23.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/superres/optical_flow.hpp>
#include <opencv2/ocl/ocl.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <iomanip>
#include <fstream>
#include <math.h>


using namespace cv;
using namespace cv::superres;
using namespace std;

void visualizeFarnebackFlow(Mat& flow, Mat& visual_flow);

int main() {
	// 動画ファイルの読み込み
	VideoCapture capture = VideoCapture("/Users/Kiri/Desktop/flow.mp4");

    // TV-L1アルゴリズムによるオプティカルフロー計算オブジェクトの生成
    //CPU
	Ptr<DenseOpticalFlowExt> opticalFlow = superres::createOptFlow_Farneback();
    
    Mat previousFrame;
    capture >> previousFrame;

	while (waitKey(1) == -1)
	{
		// 現在のフレームを保存
		Mat frame;
		capture >> frame;

        if (frame.empty()) break;
        
        // オプティカルフローの計算
        Mat flowX, flowY;
        opticalFlow->calc(previousFrame, frame, flowX, flowY);
        
        //  オプティカルフローを極座標に変換（角度は[deg]）
        Mat magnitudes, angles;
        cartToPolar(flowX, flowY, magnitudes, angles, true);
        normalize(magnitudes, magnitudes, 0, 1, NORM_MINMAX);
        
        // 合成ベクトルの角度と大きさ
        float angleSum = 0;
        float magnitudeSum = 0;
        
        //ベクトルの合成
        for (int x=flowX.cols/3; x< flowX.cols*2/3; x++) {
            for (int y=flowX.rows/3; y< flowX.rows*2/3; y++) {
                float angle = angles.at<float>(y,x);
                float magnitude = magnitudes.at<float>(y,x);
                
                if (x == 0 && y == 0) {
                    //最初のベクトルはそのまま収納
                    angleSum = angle;
                    magnitudeSum = magnitude;
                    continue;
                }
                
                //ベクトルをx,yに分割してそれぞれ合成
                float fx,fy;
                fx = (magnitude * cos(angle * M_PI / 180)) + (magnitudeSum * cos(angleSum * M_PI / 180));
                fy = (magnitude * sin(angle * M_PI / 180)) + (magnitudeSum * sin(angleSum * M_PI / 180));

                
                //大きさの合成
                //c = √(a^2 + b^2)
                magnitudeSum = sqrt(pow(fx, 2) + pow(fy, 2));
                
                
                //角度の合成
                //角度によって調整
                if (fx == 0) {
                    if (fy == 0) {
                        angleSum = 0;
                    } else if (fy > 0) {
                        angleSum = 90;
                    } else if (fy < 0) {
                        angleSum = 270;
                    }
                    
                } else if (fx > 0){
                    //Tan-1(|y/x|)
                    angleSum = (atan(abs(fy/fx)) * 180 / M_PI);

                    if (fy == 0) {
                        angleSum = 0;
                    } else if (fy < 0) {
                        angleSum = 360 - angleSum;
                    }
                } else {
                    //Tan-1(|y/x|)
                    angleSum = (atan(abs(fy/fx)) * 180 / M_PI);
                    
                    if (fy == 0) {
                        angleSum = 180;
                    } else if (fy > 0) {
                        angleSum = 180 - angleSum;
                    } else {
                        angleSum += 180;
                    }
                }
            }
        }
 
        if( magnitudeSum > 600) {
            printf("大きさ: %f, 角度: %f\n", magnitudeSum, angleSum);
            if (angleSum > 90 && angleSum < 270) {
                printf("入室!!\n");
            } else {
                printf("退室!!\n");
            }
        }
        
        imshow("aa", frame);
        
		// 表示
//		cv::imshow("input", drawFrame);
        
		// 前のフレームを保存
		previousFrame = frame;
	}
    
    return 0;
}