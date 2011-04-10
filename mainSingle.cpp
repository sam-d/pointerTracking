#include <./robustSinglePointerTracking.cpp>

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <cv.h>
#include <highgui.h>



int main(int argc, char *argv[]){
    
    if(argc < 2){
        std::cout << "Usage: channel_number" << std::endl;
        return(0);
    }

    int CHANNEL = atoi(argv[1]);
    //set up capturing device
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened()){  // check if we succeeded
        printf("could not open capture device\n");
        return -1;
    }

    //setup display window
    namedWindow("PointerTracking",CV_WINDOW_AUTOSIZE);

    //fill buffer with N frames
    printf("filling buffer...\n");
    int N = 10;
    std::vector<Mat> history;
    Mat frame;
    while(history.size() < N){
        cap >> frame;
        history.push_back(frame.clone());       
    }

    //int framecount = 0;
    while(true){
        //framecount++;

        cap >> frame;

        Point pt = robustSinglePointTracking(frame,history,10,10);

        if(!(pt.x == -1 || pt.y == -1)){
          circle(frame,pt,5,CV_RGB(0,255,0),-1);
        }

        imshow("PointerTracking",frame);

        if(waitKey(10) % 0x100 == 27){
            break;
        }
    }

    return(0);
}
