//
//  main.cpp
//  contrast
//
//  Created by Ryunosuke Kirikihira on 2014/10/01.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int main() {
    // 画像をグレイスケールで読み込む
    string fileName = "/Users/Kiri/Pictures/その他/2012_09_08_05_50_46.jpg";
    Mat original = imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);
    
    // 出力画像
    Mat result;
    
    // ルックアップテーブル作成
    float a = 50.0; // 入力パラメータ
    uchar lut[256];
    for (int i = 0; i < 256; i++)
        lut[i] = 255.0 / (1+exp(-a*(i-128)/255));
    
    // 輝度値の置き換え処理
    // Matを1行として扱う（高速化のため）
    Mat p = original.reshape(0, 1).clone();
    for (int i = 0; i < p.cols; i++) {
        p.at<uchar>(0, i) = lut[p.at<uchar>(0, i)];
    }
    // 元の形にもどす
    result = p.reshape(0, original.rows);
    
    // この置き換え処理をする関数が用意されている
    // LUT(original, Mat(Size(256, 1), CV_8U, lut), result);
    
    // 元画像の表示
    imshow("original", original);
    // 結果画像の表示
    imshow("result", result);
    waitKey(0);
    
    return 0;
}