#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

bool verifySizes(RotatedRect rectCandidatePlate);

int FindPlates(Mat &imageSource, vector<Rect> &rectsNotRotated) {
    int iKey;
    Mat imageGray;
    Mat imageSobel;                     // Vertical lines only
    Mat imageThreshold;
    Mat imageMorph;                     // 'Closed' so edges merge
    Mat imageRectangles;                // Candidate rectangles
    vector< vector< Point> > contours;

    Mat element = getStructuringElement(MORPH_RECT, Size(25,3));
#ifdef AVS_QT
    namedWindow("Source");
    namedWindow("Gray");
    namedWindow("Sobel");
    namedWindow("Threshold");
    namedWindow("Morph");
    namedWindow("Rectangles");

    imshow("Source", imageSource);

    // On Windows/Linux, we need to convert from colour to gray.  On Android, it's already gray.
    cvtColor(imageSource, imageGray, CV_BGR2GRAY);
    blur(imageGray, imageGray, Size(5,5));
#else
    // On Android, already gray for some reason, so skip the convert and do the blur straight from source.
    blur(imageSource, imageGray, Size(5,5));
#endif // AVS_QT
    // By the time we get here, it's the same on both platforms.




    //imshow("Gray", imageGray);

    Sobel(imageGray, imageSobel, CV_8U, 1, 0, 3, 1, 0);
#ifdef AVS_QT
    imshow("Sobel", imageSobel);
#endif // AVS_QT

    threshold(imageSobel, imageThreshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);

#ifdef AVS_QT
    imshow("Threshold", imageThreshold);
#endif // AVS_QT

    morphologyEx(imageThreshold, imageMorph, CV_MOP_CLOSE, element);

#ifdef AVS_QT
    imshow("Morph", imageMorph);
#endif // AVS_QT

    findContours(imageMorph, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    vector<vector<Point> >::iterator itc=contours.begin();
    vector<RotatedRect> rects;
    //vector<Rect> rectsNotRotated;

    while (itc!=contours.end()) {
        // Create bounding rect
        RotatedRect rectBounding = minAreaRect(Mat(* itc));

        if (!verifySizes(rectBounding)) {
            itc = contours.erase(itc);
            }
        else {
            itc++;
            rects.push_back(rectBounding);
            rectsNotRotated.push_back(rectBounding.boundingRect());
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

