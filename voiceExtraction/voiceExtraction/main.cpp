//
//  main.cpp
//  voiceExtraction
//
//  Created by Ryunosuke Kirikihira on 2015/01/19.
//  Copyright (c) 2015年 Ryunosuke Kirikihira. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <fstream>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// WAVを扱うための諸定義

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define WAVE_FORMAT_PCM 1

#include <pshpack1.h>   // 1バイト単位でパック設定push
typedef struct waveformat_tag {
    WORD    wFormatTag;
    WORD    nChannels;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
} WAVEFORMAT;
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;
#include <poppack.h>    // パック設定pop

//////////////////////////////////////////////////////////////////////////////


typedef struct tagWAVE {
    PCMWAVEFORMAT format;
    DWORD cb;
    BYTE *pb;
} WAVE;

void FreeWave(WAVE* pWave) {
    free(pWave->pb);
    free(pWave);
}

WAVE* LoadWave(const char *filename) {
    PCMWAVEFORMAT format;
    WAVE *pWave;
    char ab[60];
    FILE *fin;
    int count, offset;
    BYTE *pb;
    DWORD cb;
    
    fin = fopen(filename, "rb");
    if (fin == NULL)
    {
        printf("エラー: ファイル \"%s\" が開けません。\n", filename);
        return 0;
    }
    
    count = fread(ab, 1, 60, fin);
    if (count < 44)
    {
        fclose(fin);
        printf("エラー: ファイルが壊れているか、読み込みに失敗しました。\n");
        return 0;
    }
    
    if (memcmp(ab, "RIFF", 4) != 0 ||
        memcmp(ab + 8, "WAVE", 4) != 0 ||
        memcmp(ab + 12, "fmt ", 4) != 0)
    {
        fclose(fin);
        printf("エラー: 有効なWAVファイルではありません。\n");
        return 0;
    }
    
    count = *(DWORD*)(ab + 16);
    if (count != 16 && count != 18)
    {
        fclose(fin);
        printf("エラー: 有効なWAVファイルではありません。\n");
        return 0;
    }
    
    memcpy(&format, ab + 20, sizeof(PCMWAVEFORMAT));
    
    if (format.wf.wFormatTag != WAVE_FORMAT_PCM)
    {
        fclose(fin);
        printf("エラー: PCM形式ではありません。\n");
        return 0;
    }
    
    offset = 0;
    if (count == 16)
    {
        if (memcmp(ab + 36, "fact", 4) == 0)
            offset = 56;    // factチャンクがある
        else if (memcmp(ab + 36, "data", 4) == 0)
            offset = 44;    // factチャンクがない
    }
    else if (count == 18)
    {
        if (memcmp(ab + 38, "fact", 4) == 0)
            offset = 58;    // factチャンクがある
        else if (memcmp(ab + 38, "data", 4) == 0)
            offset = 46;    // factチャンクがない
    }
    
    if (offset == 0)
    {
        fclose(fin);
        printf("エラー: 有効なWAVファイルではありません。\n");
        return 0;
    }
    
    cb = *(DWORD*)(ab + offset - 4);
    if (cb == 0)
    {
        printf("警告: WAV波形データがありません。\n");
    }
    
    pb = (BYTE*)malloc(cb ? cb : 1);
    if (pb == NULL)
    {
        fclose(fin);
        printf("エラー: メモリが足りません。\n");
        return 0;
    }
    
    fseek(fin, offset, SEEK_SET);
    if (fread(pb, 1, cb, fin) != cb)
    {
        fclose(fin);
        free(pb);
        printf("エラー: ファイルが壊れているか、読み込みに失敗しました。\n");
        return 0;
    }
    
    fclose(fin);
    
    pWave = (WAVE*)malloc(sizeof(WAVE));
    if (pWave == NULL)
    {
        free(pb);
        printf("エラー: メモリが足りません。\n");
        return 0;
    }
    
    pWave->format = format;
    pWave->cb = cb;
    pWave->pb = pb;
    
    return pWave;
}



int main(int argc, char **argv)
{
    WAVE* pWave;
    BYTE *pb;
    DWORD i, cSamples;
    
//    if (argc != 2)
//    {
//        printf("使い方1: \"%s\" WAVファイル名\n", argv[1]);
//        printf("使い方2: \"%s\" WAVファイル名 > 出力ファイル.txt\n", argv[1]);
//        return 1;
//    }
    
    pWave = LoadWave("/Users/Kiri/Downloads/researchDev/m1/m1.wav");
    if (pWave != NULL)
    {
        std::ofstream outputFile("/Users/Kiri/Desktop/debag.csv");
        
        PCMWAVEFORMAT *format = &(pWave->format);
        outputFile << "チャンネル数," << format->wf.nChannels << endl;
        outputFile << "オーディオ サンプル レート," << format->wf.nSamplesPerSec << endl;
        outputFile << "ビットレート," << format->wf.nAvgBytesPerSec << endl;
        outputFile << "オーディオサンプルサイズ," << format->wBitsPerSample << endl;
        outputFile << "波形データサイズ," << pWave->cb << endl;
        outputFile << "波形データ" << endl;
        
        cSamples = pWave->cb / (format->wBitsPerSample / 8) / format->wf.nChannels;
        pb = pWave->pb;
        switch(format->wBitsPerSample)
        {
            case 8: // 1サンプル8ビット
            case 855:
                if (format->wf.nChannels == 1)
                {
                    // チャンネル数1
                    for(i = 0; i < cSamples; i++)
                    {
                        printf("%d\n", (CHAR)*pb);
                        pb++;
                    }
                }
                else if (format->wf.nChannels == 2)
                {
                    /**
                     *  ここ入る
                     */
                    // チャンネル数2
                    for(i = 0; i < cSamples; i++)
                    {
                        if (i%37 == 0) {
                            outputFile << (int)(CHAR)pb[0] << "," << (int)(CHAR)pb[1] << endl;
                        }
                        pb += 2;
                    }
                }
                else
                {
                    printf("エラー: そのようなやつは知らない。\n");
                }
                break;
                
            case 16:    // 1サンプル16ビット
                if (format->wf.nChannels == 1)
                {
                    // チャンネル数1
                    for(i = 0; i < cSamples; i++)
                    {
                        printf("%d,%d\n", *(SHORT*)pb, *(SHORT*)pb);
                        pb += 2;
                    }
                }
                else if (format->wf.nChannels == 2)
                {
                    // チャンネル数2
                    for(i = 0; i < cSamples; i++)
                    {
                        printf("%d,%d\n", *(SHORT*)&pb[0], *(SHORT*)&pb[2]);
                        pb += 4;
                    }
                }
                else
                {
                    printf("エラー: そのようなやつは知らない。\n");
                }
                break;
                
            default:
                printf("エラー: 君のようなやつは知らない。\n");
        }
        FreeWave(pWave);
    }
    
    
    return 0;
}