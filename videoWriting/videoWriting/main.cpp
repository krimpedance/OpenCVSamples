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
#define THRESHOLD 0

#define MOVIE_PATH "/Users/Kiri/Documents/xcode/OpenCV/meeting.m4v"
#define SPEACH_FILE_PATH "/Users/Kiri/Documents/xcode/OpenCV/speachActivity.csv"
#define OUTPUT_PATH "/Users/Kiri/Desktop/detectSpeaker_mouthAndFrameDiff.avi"

using namespace cv;
using namespace std;

vector<vector<float>> openSpeakerFile();
vector<vector<int>> openSpeachActivityFile();


/**
 *  メイン
 */

int main(int argc, const char * argv[]) {
    vector<vector<float>> speakers = openSpeakerFile();
    vector<vector<int>> speachActivities = openSpeachActivityFile();
    
    ofstream file("/Users/Kiri/Desktop/result.txt");
    
    for (int i=0; i<speachActivities.size(); i++) {
        float speaker[5] = {0,0,0,0,0};
        float max[2] = {0,0};
        
        for (int j=speachActivities[i][0]-1; j<speachActivities[i][1]-1 ; j++) {
            speaker[(int)speakers[j][0]] ++;
        }
        
        for (int k=0; k<4; k++) {
            if (max[1] < speaker[k]) {
                max[0] = k+1;
                max[1] = speaker[k];
            }
        }
        
        file << speachActivities[i][0] << " ~ " << speachActivities[i][1] << " ----------> " << max[0] << endl;
    }

}

vector<vector<float>> openSpeakerFile() {
    ifstream file("/Users/Kiri/Desktop/speakers.csv");
    vector<vector<float>> values;
    
    if(!file){
        cerr << "CSV file is not found.." << endl;
        exit(0);
    }
    
    float speaker;
    string sin;
    while (!file.eof()) {
        vector<float> inner;
        
        file >> speaker >> sin;

        sin.erase(0, 1);
        
        
        inner.push_back(speaker);
        inner.push_back(stof(sin));
        values.push_back(inner);
        
    }
    
    return values;
}

vector<vector<int>> openSpeachActivityFile() {
    ifstream file(SPEACH_FILE_PATH);
    vector<vector<int>> values;
    
    if(!file){
        cerr << "CSV file is not found.." << endl;
        exit(0);
    }
    
    int start;
    int end;
    char conma;
    while (!file.eof()) {
        vector<int> inner;
        
        file >> start >> conma >> end;
        
        inner.push_back(start);
        inner.push_back(end);
        
        values.push_back(inner);
    }
    
    return values;
}

