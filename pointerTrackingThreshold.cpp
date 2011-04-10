//This program tracks a laser pointer by looking for the brightest 
//pixel in the image.
//Over several frames it will din the direction of the movement as 
//a vector
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

void identifyMovement(int* startPos,int* endPos);
void controlMocp(int* startPos,int*endPos);
bool findPixelAboveThreshold(IplImage* img, int thresh, int* pixelPos);

int main(void){

    int startPoint[2];

    //set up capture device
    CvCapture* capture = cvCaptureFromCAM(0); // /dev/video0
    if(!capture){
        printf("could not open capture device\n");
    }

    //setup display window
    cvNamedWindow("PointerTracking",CV_WINDOW_AUTOSIZE);

        //setup trackbar
    int sliderValue = 130;
    cvCreateTrackbar("Threshold","PointerTracking",&sliderValue,250,NULL);

    int backgroundBrightestPixel[2];
    IplImage* tempFrame = NULL;
    bool signal = false;
    while(true){
        
        //get the frame;
        IplImage* currentFrame = cvQueryFrame(capture);
        if(!currentFrame){
            printf("No more images\n");
            break;
        }
        
        //in first iteration allocate image
        //and use it as background image
        if(!tempFrame){
            tempFrame = cvCreateImage(cvSize(currentFrame->width,currentFrame->height),IPL_DEPTH_8U,currentFrame->nChannels);
        }
        
        //copy currentFrame to tempFrame
        cvCopy(currentFrame,tempFrame);

        //apply threshold filter
        cvThreshold(tempFrame,tempFrame,(double)sliderValue,255,CV_THRESH_BINARY);
        int brightestPixel [2];
        //signal
        if(findPixelAboveThreshold(tempFrame,sliderValue,brightestPixel)){
            cvCircle(tempFrame,cvPoint(brightestPixel[0]+3,brightestPixel[1]),2,CV_RGB(250,0,0),-1);
            if(!signal){
               startPoint[0]  = brightestPixel[0];
               startPoint[1] = brightestPixel[1];

               signal = true;
            }


        }else{
            if(signal){
               identifyMovement(startPoint,brightestPixel);
               controlMocp(startPoint,brightestPixel);

               signal = false;
            }
        }

        cvShowImage("PointerTracking",tempFrame);
        //if key is pressed exit;
        if(cvWaitKey(10) > 0){
            break;
        }

    }
    
    cvDestroyWindow("PointerTracking");
    cvReleaseCapture(&capture);
    if(tempFrame){
        cvReleaseImage(&tempFrame);
    }

    return 0;
}



void identifyMovement(int* startPos,int*endPos){
   //printf("MOV: %d,%d -> %d,%d\n",startPos[0],startPos[1],endPos[0],endPos[1]); 
    int THRESHOLD = 20;

    int x1 = startPos[0];
    int x2 = endPos[0];
    int y1 = startPos[1];
    int y2 = endPos[1];
    
    int xdir = x2 - x1;
    int ydir = y2 - y1;
    
    bool noMovement = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    if(abs(xdir) < THRESHOLD && abs(ydir) < THRESHOLD){
        noMovement = true;
    }

    if(abs(xdir) > THRESHOLD && xdir > 0){
        right = true;
    }else if(abs(xdir) > THRESHOLD && xdir < 0){
        left = true;
    }

    if(abs(ydir) > THRESHOLD && ydir > 0){
        down = true;
    }else if(abs(ydir) > THRESHOLD && ydir < 0){
        up = true;
    }    
    if(noMovement){
//        printf("NO_MOV\n");
    }else if(left && up){
        printf("LEFTDIAG_UP\n");
    }else if(left && down){
        printf("LEFTDIAG_DOWN\n");
    }else if(right && up){
        printf("RIGHTDIAG_UP\n");
    }else if(right && down){
        printf("RIGHTDIAG_DOWN\n");
    }else if(left){
        printf("LEFT\n");
    }else if(right){
        printf("RIGHT\n");
    }else if(up){
        printf("UP\n");
    }else if(down){
        printf("DOWN\n");
    }else{
        printf("ERROR\n");
    }

}

//look for Pixel with value above threshold thresh
bool findPixelAboveThreshold(IplImage* img, int thresh, int* pixelPos){
    bool found = false;

    uchar* data = (uchar*)img->imageData;
    int step = img->widthStep;
    int channels = img->nChannels;
    for(int i=0; i<img->height;i++){

        for(int j=0; j<img->width; j++){
            
            for(int k=0;k<channels;k++){
                int intensity = data[i*step+j*channels+k];
                if(intensity > thresh){
                    pixelPos[1] = i;
                    pixelPos[0] = j;
                    found = true;
                }
            }   
        }
    }

    return found;
}

void controlMocp(int* startPos,int*endPos){
    int THRESHOLD = 20;

    int x1 = startPos[0];
    int x2 = endPos[0];
    int y1 = startPos[1];
    int y2 = endPos[1];
    
    int xdir = x2 - x1;
    int ydir = y2 - y1;
    
    bool noMovement = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    if(abs(xdir) < THRESHOLD && abs(ydir) < THRESHOLD){
        noMovement = true;
    }

    if(abs(xdir) > THRESHOLD && xdir > 0){
        right = true;
    }else if(abs(xdir) > THRESHOLD && xdir < 0){
        left = true;
    }

    if(abs(ydir) > THRESHOLD && ydir > 0){
        down = true;
    }else if(abs(ydir) > THRESHOLD && ydir < 0){
        up = true;
    }    
    if(noMovement){
//        printf("NO_MOV\n");
    }else if(left && up){
//        printf("LEFTDIAG_UP\n");
    }else if(left && down){
//        printf("LEFTDIAG_DOWN\n");
    }else if(right && up){
        printf("Toggle Play/Pause\n");
        system("mocp -G");
    }else if(right && down){
        printf("RIGHTDIAG_DOWN\n");
    }else if(left){
        printf("previous\n");
        system("mocp -r");
    }else if(right){
        printf("next\n");
        system("mocp -f");
    }else if(up){
        printf("Vol up\n");
        system("mocp -v +10");
    }else if(down){
        printf("Vol down\n");
        system("mocp -v -10");
    }else{
        printf("ERROR\n");
    }

}
