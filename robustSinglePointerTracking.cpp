#include<iostream>
#include <algorithm>
#include <cv.h>
#include <highgui.h>

using namespace cv;

Mat getMeanMatrix(std::vector<Mat> buffer,int ch){
    
    //put each channel ch of all frames into vector temp
    std::vector<Mat> temp;
    std::vector<Mat> channels;
    for(int i=0; i<buffer.size();i++){
        split(buffer[i],channels);
        temp.push_back(channels[ch]);
    }

    //sum over all frames in temp (thus over one channel)
    Mat mean = temp[0] / temp.size();
    for(int i=1; i<temp.size();i++){
       mean += temp[i] / temp.size();
    }

    return(mean);

}

bool truePositive(Mat diff, int sensitivity, int ncomp){

        //sort matrix first by row then by column
        Mat sorted;
        sort(diff,sorted,CV_SORT_EVERY_ROW + CV_SORT_DESCENDING);
        sort(sorted,sorted,CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);
        uchar* tilde = sorted.col(1).data; //1st col then has top ncols values

        //mean of n second 'best' pixels
        int SENS = sensitivity;
        int BEST = ncomp;
        int secondbest = 0;
        for(int i=1; i<BEST; i++){
            secondbest += tilde[i];
        }
        secondbest /= BEST;

        //only find match if it is better then average over following best matches
        //only match if better than random
        return(tilde[0] - secondbest > SENS);
}

Point robustSinglePointTracking(Mat frame, std::vector<Mat> buffer, int sens, int ncomp, int ch=2, bool refresh = true){
    
        Mat avg = getMeanMatrix(buffer,ch);

        if(refresh){
            buffer.erase(buffer.begin());
            buffer.push_back(frame.clone());
        }

        //substract current frame from average (substract noise/background from signal)
        std::vector<Mat> channels;
        split(frame,channels);

        Mat sub;
        subtract(channels[ch],avg,sub);

        //find brightest point
        Point pt;
        minMaxLoc(sub,NULL,NULL,NULL,&pt);
        
        if(truePositive(sub,sens,ncomp)){
            return(pt);
        }else{
            return(Point(-1,-1));
        }

}

std::vector<Rect> rasterizeImage(Mat img, Size r){
    
    std::vector<Rect> raster;
//    for(int i = 0; i < img.rows - r.height; i += r.height){
//       for(int j=0; j < img.cols - r.width; j += r.width){
//            raster.push_back(Rect(Point(j,i),r));
//        }
//    }

    for(int i = 0; i < img.rows; i += r.height){
        Size sz = Size(r.width,r.height);
        if(i + r.height >= img.rows){
            sz.height = img.rows - i;
        }
        for(int j=0; j < img.cols; j += r.width){
            if(j + r.width >= img.cols){
                sz.width = img.cols - j;
                raster.push_back(Rect(Point(j,i),sz));//Size(img.cols - j - 1,r.height)));
            }else{
                raster.push_back(Rect(Point(j,i),sz));
            }
        }
    }

    return(raster);

}
//raster avg image and frame into non-overlapping Rects and perform singlePointTracking in each
std::vector<Point> multiPointTracking(Mat frame, std::vector<Mat> buffer, int width, int height, int sens, int ncomp, int ch=2, bool refresh = true){
    
    std::vector<Rect> raster = rasterizeImage(frame, Size(width,height));

    std::vector<Point> results;
    for(std::vector<Rect>::iterator it = raster.begin(); it < raster.end(); it++){
        std::vector<Mat> buffer_rasterize;
        for(std::vector<Mat>::iterator it2 = buffer.begin(); it2 < buffer.end(); it2++){
            buffer_rasterize.push_back(Mat(*it2,*it));
        }

        Mat frameROI = Mat(frame,*it);
        Point pt = robustSinglePointTracking(frameROI,buffer_rasterize,sens,ncomp,ch,refresh);
        Point rel; Size sz;
        frameROI.locateROI(sz,rel); 

        if(pt.x != -1 && pt.y != -1){
            results.push_back(pt+rel);
        }
        
    }

    return(results);
}
//std::vector<Point> multiPointTracking(Mat frame, Mat buffer, int ch=2, bool refresh=true){
//
//        Mat avg = getMeanMatrix(buffer,ch);
//
//        if(refresh){
//            buffer.erase(buffer.begin());
//            buffer.push_back(frame.clone());
//        }
//
//        //substract current frame from average (substract noise/background from signal)
//        std::vector<Mat> channels;
//        split(frame,channels);
//
//        Mat sub;
//        subtract(channels[ch],avg,sub);
//
//        //sort matrix first by row then by column
////        Mat sorted;
////        sortIdx(diff,sorted,CV_SORT_EVERY_ROW + CV_SORT_DESCENDING);
////        sortIdx(sorted,sorted,CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);
////        uchar* tilde = sorted.data; //1st col then has top ncols values
//
//        std::vector<Rect> rois; //vector with regions of interest
//        int count_fails = 0;
//        int MAXFAILS = 8;
//        int WIDTH = 5;
//        int HEIGHT = 5;
//        Size sz = Size(WIDTH,HEIGHT);
//        while(count_fails < MAXFAILS){
//            //find brightest point
//            Point pt;
//            minMaxLoc(sub,NULL,NULL,NULL,&pt);
//
//            bool newROI = true;
//            for(std::vector<Rect>::iterator it = rois.begin(); it < rois.end(); it++){
//                if(*it.contains(pt)){
//                    newROI = false;
//                    break;
//                }
//            }
//            if(newROI){
//                rois.push_back(Rect(pt,sz));
//            }else{
//                cout_fails++;
//            }
//
//            //TODO
//        }
//}

