#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "AVS-ANPR.h"

//#include "FindPlates.h"

using namespace std;
using namespace cv;

bool verifySizes(RotatedRect rectCandidatePlate);
double LineLength(cv::Point *pt1, cv::Point *pt2);

int FindPlates(Mat &imageSource, vector<Rect> &rectsNotRotated, vector<RotatedRect> &rectsRotated, Mat &imageDebug, int iDebugMode) {
    int iKey;
    Mat imageGray;
    Mat imageSobel;                     // Vertical lines only
    Mat imageThreshold;
    Mat imageMorph;                     // 'Closed' so edges merge
    Mat imageRectangles;                // Candidate rectangles
    vector< vector< Point> > contours;
    std::vector<cv::Point> approx;
    RotatedRect rectRotated;
    Rect rect;
    RotatedRect rectBounding;

    Mat element = getStructuringElement(MORPH_RECT, Size(25,3));
#ifdef AVS_QT
    namedWindow("Source");
    namedWindow("Gray");
    namedWindow("Sobel");
    namedWindow("Threshold");
    namedWindow("Morph");
    namedWindow("Rectangles");

    //imshow("Source", imageSource);

    // On Windows/Linux, we need to convert from colour to gray.  On Android, it's already gray.
    cvtColor(imageSource, imageGray, CV_BGR2GRAY);
    blur(imageGray, imageGray, Size(5,5));
#else

    cvtColor(imageSource, imageGray, CV_BGR2GRAY);
    blur(imageGray, imageGray, Size(5,5));
    //blur(imageSource, imageGray, Size(5,5));
#endif // AVS_QT
    // By the time we get here, it's the same on both platforms.




    //imshow("Gray", imageGray);

    Sobel(imageGray, imageSobel, CV_8U, 1, 0, 3, 1, 0);
    //Sobel(imageGray, imageSobel, , 1, 0, 3, 1, 0);

    if (iDebugMode == AVS_DEBUG_MODE_DISPLAY_SOBEL) {
    	// Need to convert it to colour before we can copy it
    	cvtColor(imageSobel, imageDebug, CV_GRAY2BGRA);  // Converts and copies in one go
        }

#ifdef AVS_QT
    imshow("Sobel", imageSobel);
#endif // AVS_QT

    threshold(imageSobel, imageThreshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);

#ifdef AVS_QT
    imshow("Threshold", imageThreshold);
#endif // AVS_QT

    morphologyEx(imageThreshold, imageMorph, CV_MOP_CLOSE, element);

    if (iDebugMode == AVS_DEBUG_MODE_DISPLAY_MORPH) {
    	// Need to convert it to colour before we can copy it
    	cvtColor(imageMorph, imageDebug, CV_GRAY2BGRA);  // Converts and copies in one go
        }

#ifdef AVS_QT
    imshow("Morph", imageMorph);
#endif // AVS_QT

    findContours(imageMorph, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    vector<RotatedRect> rects;
    for (size_t i = 0; i < contours.size(); i++) {

        rectBounding = minAreaRect((InputArray) contours[i]);
        rect = boundingRect((InputArray) contours[i]);


        if (!verifySizes(rectBounding)) {
            }
        else {
            //rectangle(imageSource, rect, CV_RGB(255,0,0));
            //rectangle()
            rects.push_back(rectBounding);
            rectsNotRotated.push_back(rectBounding.boundingRect());
            rectsRotated.push_back(rectBounding);
            }

        } // End 'check each contour for aspect ratio and size'


#ifdef AVS_QT
    iKey = cvWaitKey(1);
    if (iKey == 27) {
        return(-1);                 // -1 == ESCAPE
        }
#endif // AVS_QT
    return 0;
    }


bool verifySizes(RotatedRect rectCandidatePlate) {
    // Part of plate with digits on is 43x8cm, so aspect ratio == 5.
    float fAspectRatioWanted = 5.0;
    float fErrorAllowed = 0.2;
    float fAspectRatio;
    int iRectWidth;

    // First, the width and height of a RotatedRect can be confusing.  Get the bounding rect
    // of the rotated rect, and make sure we're only using ones which are wider than long,
    // number plates as they would normally appear.

    Rect rectBounding = rectCandidatePlate.boundingRect();

    //iRectWidth = rectBounding.width;

    if ((float) rectBounding.width < (float) rectBounding.height) {
        return(false);
        }

    if (rectBounding.width < 30 || rectBounding.height < 30) {
        // Too small for us to use, and saves a lot of 'point-sized' regions being
        // processed later, causing problems for perspective warping, etc.
        return(false);
        }

    fAspectRatio = (float) rectCandidatePlate.size.width / (float) rectCandidatePlate.size.height;
    if (fAspectRatio < 1.0) {
        fAspectRatio = 1/fAspectRatio;
        }

    if (fAspectRatio < (fAspectRatioWanted - fAspectRatioWanted*fErrorAllowed) ||
       fAspectRatio > (fAspectRatioWanted + fAspectRatioWanted*fErrorAllowed)) {
        return(false);
        }
    else {
        return(true);
        }

    } // End 'verifySizes()'

void FindLargestPlate(vector<RotatedRect> &rectsRotated, RotatedRect &rotatedrectLargestPlate) {
    if (!rectsRotated.size()) {
        // no rects detected
        return;
        }

    int max_width = 0;
    int max_height = 0;
    int max_rect_idx = 0;

    for (size_t i = 0; i < rectsRotated.size(); i++) {

        Point2f points[4];

        RotatedRect rotatedrectTemp = rectsRotated[i];
        rotatedrectTemp.points(points); // Get the points
        //rectsRotated[i].points(points); // Get the points
        // boundingRect needs it as a vector (or Mat*)
        vector<Point> vectorPoints;
        vectorPoints.push_back(points[0]);
        vectorPoints.push_back(points[1]);
        vectorPoints.push_back(points[2]);
        vectorPoints.push_back(points[3]);

        cv::Rect rectangle = boundingRect(vectorPoints);

        // Store the index position of the biggest rectangle found
        if ((rectangle.width >= max_width) && (rectangle.height >= max_height)) {
            max_width = rectangle.width;
            max_height = rectangle.height;
            max_rect_idx = i;
            }
        } // End 'for each rectangle'

    rotatedrectLargestPlate = rectsRotated[max_rect_idx];

    }

//#ifdef OUT_FOR_NOW

Mat DerotateAndCorrectPerspective(Mat &imageSource, RotatedRect &rectRotated) {
    Mat imageDerotatedAndCorrected;
    Mat imageCropped;
    Point2f pt[4];
    int iLeftmostPointIndex, iRightmostPointIndex;
    int iSecondLeftmostPointIndex, iSecondRightmostPointIndex;
    int iLeftmostPointX, iSecondLeftmostPointX;
    int iRightmostPointX, iSecondRightmostPointX;
    Point ptTLH, ptTRH, ptBLH, ptBRH;
    double fLineLength1, fLineLength2;
    double fDestWidth, fDestHeight;

    rectRotated.points(pt);             // Populate the points

    iLeftmostPointX = 99999;
    iRightmostPointX = -1;
    iSecondLeftmostPointX = 99999;
    iSecondRightmostPointX = -1;

    // We now need to know which is the TLH, TRH, BLH and BRH points.  This might be messy and there's probably a better way,
    // but this does work and gives us full control over it.

    // First, find the left-most and right-most.

    for (int i=0; i<4; i++) {           // For each of the 4 points
        if (pt[i].x < iLeftmostPointX) {
            iLeftmostPointIndex = i;
            iLeftmostPointX = pt[i].x-1;   // by doing -1 (and +1 below) we make sure all 4 points are different.
            }
        if (pt[i].x > iRightmostPointX) {
            iRightmostPointIndex = i;
            iRightmostPointX = pt[i].x+1;
            }
        } // End 'for each corner point'

    // Among the other two points, find which is the leftmost and rightmost i.e. the second leftmost and rightmost.

    for (int i=0; i<4; i++) {  // For each of the 4 points
        if (i == iLeftmostPointIndex || i == iRightmostPointIndex) {
            // We only want to look at the other two points
            continue;
            }
        if (pt[i].x < iSecondLeftmostPointX) {
            iSecondLeftmostPointIndex = i;
            iSecondLeftmostPointX = pt[i].x-1;  // by doing -1 (and +1 below) we make sure all 4 points are different.
            }
        if (pt[i].x > iSecondRightmostPointX) {
            iSecondRightmostPointIndex = i;
            iSecondRightmostPointX = pt[i].x+1;
            }
        }

    // We now know which are the left and rightmost points, and the second left//rightmost, and we've
    // been careful to make sure all 4 points have been used, i.e. one can't be second left and rightmost
    // while the other goes unused.

    // Now sort the points so we know which is which.

    if (pt[iLeftmostPointIndex].y < pt[iSecondLeftmostPointIndex].y) {
        ptTLH = pt[iLeftmostPointIndex];
        ptBLH = pt[iSecondLeftmostPointIndex];
        }
    else {
        ptTLH = pt[iSecondLeftmostPointIndex];
        ptBLH = pt[iLeftmostPointIndex];
        }

    if (pt[iRightmostPointIndex].y < pt[iSecondRightmostPointIndex].y) {
        ptTRH = pt[iRightmostPointIndex];
        ptBRH = pt[iSecondRightmostPointIndex];
        }
    else {
        ptTRH = pt[iSecondRightmostPointIndex];
        ptBRH = pt[iRightmostPointIndex];
        }
#ifdef OLD_AVS_DEBUG

    circle(imageDebug, ptTLH, 5, CV_RGB(255,0,0));
    circle(imageDebug, ptTRH, 5, CV_RGB(0,255,0));
    circle(imageDebug, ptBLH, 5, CV_RGB(255,255,255));
    circle(imageDebug, ptBRH, 5, CV_RGB(0,0,255));
    imshow("Debug", imageDebug);
    waitKey(1);
#endif // OLD_AVS_DEBUG

    //
    // Now we're ready to start the perspective warping.
    //

    // For the destination height, we'll use the length of the longest of the vertical lines
    // in the source image - likewise for the width.

    fLineLength1 = LineLength(&ptTLH, &ptBLH);
    fLineLength2 = LineLength(&ptTRH, &ptBRH);
    if (fLineLength1 > fLineLength2) {
        fDestHeight = fLineLength1;
        }
    else {
        fDestHeight = fLineLength2;
        }

    fLineLength1 = LineLength(&ptTLH, &ptTRH);
    fLineLength2 = LineLength(&ptBLH, &ptBRH);
    if (fLineLength1 > fLineLength2) {
        fDestWidth = fLineLength1;
        }
    else {
        fDestWidth = fLineLength2;
        }

    imageDerotatedAndCorrected.create( imageSource.size(), imageSource.depth());

    Point2f src[4], dst[4];
    Mat matTransform;

    src[0].x = ptTLH.x;
    src[0].y = ptTLH.y;
    src[1].x = ptTRH.x;
    src[1].y = ptTRH.y;
    src[2].x = ptBLH.x;
    src[2].y = ptBLH.y;
    src[3].x = ptBRH.x;
    src[3].y = ptBRH.y;

    dst[0].x = src[0].x; // TLH corner can be in the same place.
    dst[0].y = src[0].y;
    dst[1].x = dst[0].x + fDestWidth;
    dst[1].y = dst[0].y;
    dst[2].x = dst[0].x;
    dst[2].y = dst[0].y + fDestHeight;
    dst[3].x = dst[0].x + fDestWidth;
    dst[3].y = dst[0].y + fDestHeight;

    // Calculate perspective warp matrix
    matTransform = getPerspectiveTransform(src, dst);

    // Perspective Warp

    warpPerspective(imageSource, imageDerotatedAndCorrected, matTransform, imageDerotatedAndCorrected.size(), INTER_LINEAR);

    // Crop the new image to the correct size

    imageCropped.create(imageSource.size(), imageSource.depth());
    imageCropped.create(Size((int) fDestWidth, (int) fDestHeight), imageSource.depth());

    printf("dst[0].x = %d, dst[0].y = %d, fDestWidth = %d, fDestHeight = %d\n", (int) dst[0].x, (int) dst[0].y, (int) fDestWidth, (int) fDestHeight);

    Rect rectROI((int) dst[0].x, (int) dst[0].y, (int) fDestWidth, (int) fDestHeight);

    if (rectROI.x < 0) {
        rectROI.x = 0;
        }
    if (rectROI.y < 0) {
        rectROI.y = 0;
        }
    if (rectROI.x + (rectROI.width-1) >= imageDerotatedAndCorrected.size().width) {
        rectROI.width = (imageDerotatedAndCorrected.size().width - (rectROI.x)) - 1;
        }
    if (rectROI.y + (rectROI.height-1) >= imageDerotatedAndCorrected.size().height) {
        rectROI.height = (imageDerotatedAndCorrected.size().height - (rectROI.y)) - 1;
        }
    // C++ making a f**king wonderful meal of this as usual.
    printf("Rect: x=%d,y=%d, width=%d, height=%d\n", rectROI.x, rectROI.y, rectROI.width, rectROI.height);
    printf("Image: width=%d, height=%d\n", imageDerotatedAndCorrected.size().width, imageDerotatedAndCorrected.size().height);
    //Rect()
    imageCropped = imageDerotatedAndCorrected(rectROI);  // Very efficient - just points to ROI in the big image with no copying

    //return imageDerotatedAndCorrected;
    return imageCropped;
    }

double LineLength(cv::Point *pt1, cv::Point *pt2) {
    return (   sqrt((double) pow((float)(pt1->x - pt2->x), 2) + pow((float)(pt1->y - pt2->y), 2))  );
    }
//#endif // OUT_FOR_NOW
