//
//  main.cpp
//  ffmpeg_sample
//
//  Created by Ryunosuke Kirikihira on 2014/10/02.
//  Copyright (c) 2014年 Ryunosuke Kirikihira. All rights reserved.
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


int main( int argc, char* argv[] ) {
    av_register_all();
    return 0;
}