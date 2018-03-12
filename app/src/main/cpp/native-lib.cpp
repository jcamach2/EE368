#include <jni.h>
#include <string>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <android/log.h>
#include <iostream>

using namespace std;
using namespace cv;

//OrbFeatureDetector  orb_features (250 /* # of features */, 1.2f /* scale factor */, 8 /* # levels */);

OrbFeatureDetector  orb_features (250 /* # of features */, 1.2f /* scale factor */, 8 /* # levels */);


vector<KeyPoint> prevKeypoints;
Mat prevFrameDescriptors;

Mat currentDescriptors;

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
    double e1 = (double) getTickCount();
    vector<KeyPoint> mKeypoints;
    orb_features.detect(mGray2, mKeypoints);
    double e2 = (double) getTickCount();
    double t = (e2 - e1)/getTickFrequency();
    __android_log_print(ANDROID_LOG_INFO, "test", "Detect keypoints timing = %f", t);

    /* Compute Descriptors */
    double f1 = (double) getTickCount();
    Ptr<DescriptorExtractor>  orb_descriptors = DescriptorExtractor::create("ORB");

    orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);

    double f2 = (double) getTickCount();
    double t2 = (f2 - f1)/getTickFrequency();
    __android_log_print(ANDROID_LOG_INFO, "test", "Compute descriptors timing = %f", t2);

    if (!isFirst) {

        /*
        for (vector<KeyPoint>::const_iterator it = mKeypoints.begin(); it != mKeypoints.end(); it++ ) {

            Point2f pt = it->pt;
            circle(mRgba2, pt, it->size*0.2, cvScalar(255, 255, 0, 0), 2);
        }*/
        double g1 = (double) getTickCount();
        BFMatcher matcher(NORM_HAMMING, true);
        vector<DMatch> matches;

        matcher.match(currentDescriptors, prevFrameDescriptors, matches);

        vector<Point2f> f1Matches;
        vector<Point2f> f2Matches;
        // filter to get good matches
        for (int i = 0; i < matches.size(); i++) {
            if (matches[i].distance < 75) {
                f1Matches.push_back(mKeypoints[matches[i].queryIdx].pt);        //current
                f2Matches.push_back(prevKeypoints[matches[i].trainIdx].pt);     //previous
            }
        }
        double g2 = (double) getTickCount();
        double t3 = (g2 - g1)/getTickFrequency();
        __android_log_print(ANDROID_LOG_INFO, "test", "Match features timing = %f", t3);

        /* Find homography estimate */

        double h1 = (double) getTickCount();

        Mat stabilizedM;
//        Mat H = findHomography(/* src */ f1Matches, /* dst */ f2Matches, RANSAC);
//        warpPerspective(mGray2, /*mRgba2*/ stabilizedM, H, mRgba2.size());
        Mat affine = estimateRigidTransform(f1Matches, f2Matches, false);
        warpAffine(mGray2, stabilizedM, affine, mRgba2.size());
      //  mRgba2 = stabilizedM;
        mGray2 = stabilizedM;


            double h2 = (double) getTickCount();
            double t4 = (h2 - h1) / getTickFrequency();
            __android_log_print(ANDROID_LOG_INFO, "test", "Create transform timing = %f", t4);
        if (num_frames % 10 == 0) { // only save the descriptor data every 10 frames
            double i1 = (double) getTickCount();

            orb_features.detect(mGray2, mKeypoints);

            double i2 = (double) getTickCount();
            double t5 = (i2 - i1) / getTickFrequency();
            __android_log_print(ANDROID_LOG_INFO, "test", "Detect orb features timing = %f", t5);
            double j1 = (double) getTickCount();

            orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);

            double j2 = (double) getTickCount();
            double t6 = (j2 - j1) / getTickFrequency();
            __android_log_print(ANDROID_LOG_INFO, "test", "Compute orb descriptors timing = %f",
                                t6);

        }

    }
    if (num_frames % 10 == 0) {
        prevFrameDescriptors = currentDescriptors.clone();
        prevKeypoints = mKeypoints;
    }
}