//
//  main.cpp
//  researchDev
//
//  Created by Ryunosuke Kirikihira on 2014/09/16.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//


#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iomanip>
#include <fstream>

#define MOUTH_CASCADE "/Users/Kiri/Documents/xcode/OpenCV/haarcascades/haarcascade_mcs_mouth.xml"
#define FACE_CASCADE "/Users/Kiri/Documents/xcode/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"
#define SCALE 1

#define MOVIE_PATH "/Users/Kiri/Downloads/researchDev/m2/m2.mp4"
#define SPEACH_FILE_PATH "/Users/Kiri/Documents/xcode/OpenCV/speachActivity.csv"
#define OUTPUT_PATH "/Users/Kiri/Desktop/detectSpeaker_mouthAndFrameDiff.avi"

using namespace cv;
using namespace std;

void presentTime(String str);
vector<vector<int>> getPixel(Mat& frame);
Rect detectWithCascade( Mat& frame, CascadeClassifier& cascade, double scale);
int diffForFrame(Mat& previousFrame, Mat& frame, int threshold);
void exportFile(vector<vector<int>> frameDiffs, vector<vector<int>> mouthDiffs);



/**
 *  メイン
 */

int main(int argc, const char * argv[]) {
    /**
     *  読み込み
     */

    //映像読み込み
    VideoCapture capture = VideoCapture(MOVIE_PATH);
    
    if(!capture.isOpened()) {
        return 0;
    }
    
    //カスケード読み込み
    CascadeClassifier faceCascade, mouthCascade;
    faceCascade.load(FACE_CASCADE);
    mouthCascade.load(MOUTH_CASCADE);

    
    /**
     *  各種変数
     */

    ///総フレーム数
    int allFrameNum = capture.get(CV_CAP_PROP_FRAME_COUNT);
    
    ///処理完了度
    int progress = 0;
    
    ///現在のフレーム
    int frameNum = 0;
    
    ///4分割したときの画像サイズ
    float width, height;

    ///差分値の配列
//    vector<vector<int>> mouthDiffs, frameDiffs;
    
    ///フレーム毎の画素値の配列
//    vector<vector<vector<int>>> pixels;
    
    ///カメラ毎の画素値の配列
//    vector<vector<vec>>
    
    
    /**
     *  フレーム間差分用のフレーム設定
     */
    
    ///差分用フレーム
    Mat previousFrame;
    capture >> previousFrame;
    
    //ついでに分割時の画像サイズの値代入
    width = previousFrame.cols/2;
    height = previousFrame.rows/2;
    
    ///前のフレームの口の位置
    Rect previousMouthRect[4];
    
    //初期化
    for (int n=0; n<4; n++) {
        //一つのカメラ画像
        Mat divideFrame = previousFrame(Rect(n%2*width,n/2*height,width,height));

        Mat mouthDetectFrame = divideFrame(Rect(width/3, height/5, width/4, height*2/4));
        previousMouthRect[n] = detectWithCascade(mouthDetectFrame, mouthCascade, 0.1);
    }
    
    
    /**
     *  処理系
     */

    //開始時刻を出力
    presentTime("start");
    

    
    ofstream frameDiffFile("/Users/Kiri/Downloads/researchDev/m2/originalData/csv/m2_frameDiff.csv");
    ofstream mouthDiffFile("/Users/Kiri/Downloads/researchDev/m2/originalData/csv/m2_mouthDiff.csv");
    


    /**
     *  各種変数
     */
    
    /**
     *  フレーム用の変数
     *
     * @param frame                             検出用フレーム画像
     * @param drawFrame                     描画用フレーム画像
     * @param detectFrameForMouth      口を検出するためのフレーム画像
     *
     */
    Mat frame, drawFrame, mouthDetectFrame;
    
    /// 分割画像のフレーム間差分値の配列 {左上,右上,左下,右下}
    vector<int> frameDiff(4);
    /// 口のフレーム間差分値の配列 {左上,右上,左下,右下}
    vector<int> mouthDiff(4);

    //処理開始
    while (waitKey(1) == -1)
    {
        /**
         *  フレームの設定
         */
        // 画像取得
        capture >> frame;
        frameNum++;
        // 描画用画像にクローン
        drawFrame = frame.clone();
        // 映像が終わったらループを抜ける
        if (frame.empty()) break;
        
        
        /**
          *　発話区間内であれば顔と口を検出，描画
          */
        for (int i=0; i<4; i++) {
            ///一つのカメラ画像
            Mat divideFrame = frame(Rect(i%2*width,i/2*height,width,height));
            
            ///一つのカメラ画像のフレーム間差分
            Mat dividePreviousFrame = previousFrame(Rect(i%2*width,i/2*height,width,height));
            frameDiff[i] = diffForFrame(dividePreviousFrame, divideFrame, 5);

            ///口を検出する範囲
            Rect mouthDetectRect =  Rect((i%2*width)+(width/3), (i/2*height)+(height/5), width/3, height*1/2);
            
            mouthDetectFrame = frame(mouthDetectRect);
            
            //口検出
            Rect mouthRect = detectWithCascade(mouthDetectFrame, mouthCascade, 0.1);
            
            //口が検出されなかったら前のフレームの口検出位置を使用
            if (mouthRect == Rect(0,0,0,0))
                mouthRect = previousMouthRect[i];
            
            //口範囲のフレーム間差分
            if (mouthRect != Rect(0,0,0,0)) {
                //口範囲の更新
                previousMouthRect[i] = mouthRect;
                
                //口範囲のフレーム画像
                Mat mouthFrame = mouthDetectFrame(mouthRect);
                
                //前フレームの口範囲画像
                Mat previousMouthFrame = previousFrame(mouthRect);
                
                //フレーム間差分
                mouthDiff[i] = diffForFrame(previousMouthFrame, mouthFrame, 50);
            } else {
                mouthDiff[i] = 0;
            }
            
            
            //debag:顔と口の範囲を描画
//            rectangle(drawFrame, Rect(mouthDetectRect.x, mouthDetectRect.y,mouthDetectRect.width,mouthDetectRect.height), CV_RGB(0, 0, 255), 2, 4);
//            rectangle(drawFrame, Rect(mouthDetectRect.x+mouthRect.x, mouthDetectRect.y+mouthRect.y, mouthRect.width, mouthRect.height), CV_RGB(255, 0, 0), 2, 4);
        }

        /**
         *  ファイル書き込み
         */
//        frameDiffFile << frameDiff[0] << ", " << frameDiff[1] << ", " << frameDiff[2] << ", " << frameDiff[3] << endl;
//        mouthDiffFile << mouthDiff[0] << ", " << mouthDiff[1] << ", " << mouthDiff[2] << ", " << mouthDiff[3] << endl;

        
        /**
         *  後処理
         */
        // 差分用フレームを入れ替え
        previousFrame = frame.clone();
        
        //進行度
        if ((int)(((float)frameNum/allFrameNum)*100) > progress) {
            cout << "progress --> " << setprecision(2) << (int)(((float)frameNum/allFrameNum)*100) << "%" << endl;
            progress = (int)(((float)frameNum/allFrameNum)*100);
        }
        
        //debag:画像出力
//        imshow("output", drawFrame);
    }
    
    
    /**
     *  プログラム実行終了処理
     */
    
    //ウィンドウの解放
//    destroyWindow("output");
    
    //終了時刻を出力
    presentTime("end");
    
    return 0;
}


/**
 *  現在時刻を出力
 */
void presentTime(String str) {
    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    
    char buff[128]="";
    sprintf(buff,"%2d:%2d:%2d",pnow->tm_hour,pnow->tm_min,pnow->tm_sec);

    cout << str << " --> " << buff << endl;
}


/**
 *  検出器による検出
 */
Rect detectWithCascade( Mat& frame, CascadeClassifier& cascade, double scale) {
    double t = 0;
    vector<Rect> objects;
    //    Scalar color = CV_RGB(255,0,0);
    
    //スケーリング
    Mat gray, smallImg( cvRound (frame.rows/scale), cvRound(frame.cols/scale), CV_8UC1 );
    cvtColor( frame, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );
    
    t = (double)cvGetTickCount();
    
    
    //検出
    cascade.detectMultiScale( smallImg, objects,
                             1.1, 2, 0
                             |CV_HAAR_FIND_BIGGEST_OBJECT
                             //|CV_HAAR_DO_ROUGH_SEARCH
                             //|CV_HAAR_SCALE_IMAGE
                             ,
                             Size(30, 30) );
    
    //検出されなかったら0を返す
    if (objects.empty()) {
        return Rect(0,0,0,0);
    }
    
    //最初の検出オブジェクト
    vector<Rect>::const_iterator r = objects.begin();
    
    return Rect(r->x*scale,r->y*scale,r->width*scale,r->height*scale);
}


/**
 *  フレーム間差分
 *
 *  @param previousFrame 前フレーム
 *  @param frame         フレーム
 *
 *  @return 差分値
 */
int diffForFrame(Mat& previousFrame, Mat& frame, int threshold){
    int x, y;
    int p1[3], p2[3];
    int count = 0;
    Mat draw = frame.clone();
    
    for (y = 0 ; y < previousFrame.rows; y++) {
        for (x = 0 ; x < previousFrame.cols; x++) {
            for (int i=0; i<previousFrame.channels(); i++) {
                p1[i] = previousFrame.data[previousFrame.step*y + previousFrame.elemSize()*x + i];  //1枚目の画素値
                p2[i] = frame.data[frame.step*y + frame.elemSize()*x + i];  //2枚目の画素値
            }
            
            if ( abs(p1[0]-p2[0]) > threshold && abs(p1[1]-p2[1]) > threshold && abs(p1[2]-p2[2]) > threshold ) {   /*違う色*/
                count++;
                for (int i=0; i<previousFrame.channels(); i++) {
                    draw.data[draw.step*y + draw.elemSize()*x + i] = 0;
                }
            }else{ /*同じ色*/
                for (int i=0; i<previousFrame.channels(); i++) {
                    draw.data[draw.step*y + draw.elemSize()*x + i] = 255;
                }
            }
        }
    }
    
    
    imshow("debug", draw);
    imshow("original", frame);
    
    
    return count;
}