#include <jni.h>
#include <string>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;

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
                                                                                jboolean isFirst) {

    Mat& mGray2 = *(Mat *)frameGray;
    Mat& mRgba2 = *(Mat *)frameRgba;

    /* Detect keypoints */

    vector<KeyPoint> mKeypoints;
    orb_features.detect(mGray2, mKeypoints);

    /* Compute Descriptors */
    Ptr<DescriptorExtractor>  orb_descriptors = DescriptorExtractor::create("ORB");

    orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);

    if (!isFirst) {

        /*
        for (vector<KeyPoint>::const_iterator it = mKeypoints.begin(); it != mKeypoints.end(); it++ ) {

            Point2f pt = it->pt;
            circle(mRgba2, pt, it->size*0.2, cvScalar(255, 255, 0, 0), 2);
        }*/

        BFMatcher matcher(NORM_HAMMING, true);
        vector<DMatch> matches;

        matcher.match(currentDescriptors, prevFrameDescriptors, matches);

        vector<Point2f> f1Matches;
        vector<Point2f> f2Matches;
        // filter to get good matches
        for (int i = 0; i < matches.size(); i++) {
            if (matches[i].distance < 100) {
                f1Matches.push_back(mKeypoints[matches[i].queryIdx].pt);        //current
                f2Matches.push_back(prevKeypoints[matches[i].trainIdx].pt);     //previous
            }
        }

        /* Find homography estimate */
        Mat H = findHomography(/* src */ f1Matches, /* dst */ f2Matches, RANSAC);

        Mat stabilizedM;
        warpPerspective(mGray2, /*mRgba2*/ stabilizedM, H, mRgba2.size());

      //  mRgba2 = stabilizedM;
        mGray2 = stabilizedM;
        orb_features.detect(mGray2, mKeypoints);
        orb_descriptors->compute(mGray2, mKeypoints, currentDescriptors);
    }

    prevFrameDescriptors = currentDescriptors.clone();
    prevKeypoints = mKeypoints;
}