#include <jni.h>
#include <string>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

using namespace std;
using namespace cv;

OrbFeatureDetector  orb_features (200 /* # of features */, 1.2f /* scale factor */, 8 /* # levels */);

vector<KeyPoint> prevKeypoints;
Mat prevFrameDescriptors;

Mat currentDescriptors;
Mat stabilizedM;

Mat prevH = Mat(3,3, CV_64F, 0.0);

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_example_juani_videostabilizationandroid_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void

JNICALL Java_com_example_juani_videostabilizationandroid_MainActivity_AlignFrame(JNIEnv*, jobject, jlong frameRgba, jlong frameGray,
                                                                                jboolean isFirst, jint num_frames) {

    Mat& mGray2 = *(Mat *)frameGray;
    Mat& mRgba2 = *(Mat *)frameRgba;

    /* Detect keypoints */

    vector<KeyPoint> mKeypoints;
    orb_features.detect(mGray2, mKeypoints);

    /* Compute Descriptors */
    Ptr<DescriptorExtractor>  orb_descriptors = DescriptorExtractor::create("ORB");

    orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);

    if (!isFirst) {

        BFMatcher matcher(NORM_HAMMING, true);
        vector<DMatch> matches;

        matcher.match(currentDescriptors, prevFrameDescriptors, matches);

        vector<Point2f> f1Matches;
        vector<Point2f> f2Matches;

        // filter to get good matches
        for (int i = 0; i < matches.size(); i++) {
            if (matches[i].distance < 80) {
                f1Matches.push_back(mKeypoints[matches[i].queryIdx].pt);        //current
                f2Matches.push_back(prevKeypoints[matches[i].trainIdx].pt);     //previous
            }
        }

        /* Find homography estimate */
 //           Mat H = estimateRigidTransform(f1Matches, f2Matches, false);
  //        H = prevH * H;
    //        warpAffine(mRgba2, stabilizedM, H, mRgba2.size());

        Mat H = findHomography(/* src */ f1Matches, /* dst */ f2Matches, RANSAC);
        H = prevH * H;
        warpPerspective(mRgba2, stabilizedM, H, mRgba2.size());

        mRgba2 = stabilizedM;
        prevH = H.clone();
       // if (num_frames % 10 == 0) {
       //     cvtColor(stabilizedM, mGray2, cv::COLOR_RGB2GRAY);
        //    orb_features.detect(mGray2, mKeypoints);
         //   orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);
       // }
    }
    else {
   // if (num_frames % 10 == 0) {
        prevH = Mat(3,3, CV_64F, 0.0);
        prevH.at<double>(0,0) = 1;
        prevH.at<double>(1,1) = 1;
        prevH.at<double>(2,2) = 1;
    }
//    if (num_frames % 10 == 0) {
     prevFrameDescriptors = currentDescriptors.clone();
     prevKeypoints = mKeypoints;
 //   }
}