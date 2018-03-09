package com.example.juani.videostabilizationandroid;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import android.util.Log;


public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private static final int        VIEW_CAMERA_RGBA = 0;
    private static final int        VIEW_CAMERA_ORB = 1;

    private static final int        DIALOG_SHOW_FIRST = 0;
    private static final int        DIALOG_SHOW_ORB = 1;

    private int                     mCameraViewMode;

    private Mat                     mRgba;
    private Mat                     mGray;

    private MenuItem                mItemStabilizer;

    private CameraBridgeViewBase    mOpenCvCameraView;

    private boolean                 firstFrame = true;


    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i("LOG:", "Loader Callback Success");

                    System.loadLibrary("native-lib");
                    mOpenCvCameraView.enableView();
                } break;
                default:
                    super.onManagerConnected(status);
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        //Set up Camera
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.surface_view);
        mOpenCvCameraView.setMaxFrameSize(640, 480);
        mOpenCvCameraView.setCvCameraViewListener(this);

        mCameraViewMode = VIEW_CAMERA_RGBA;

        showToasts(DIALOG_SHOW_FIRST);
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mItemStabilizer = menu.add("Camera Stabilization");
        return true;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
    }

    public boolean onOptionsItemSelected(MenuItem item) {

        if (item == mItemStabilizer) {
            mCameraViewMode = VIEW_CAMERA_ORB;
            showToasts(DIALOG_SHOW_ORB);
        }

        return true;
    }

    public void onCameraViewStarted(int width, int height) {
        mRgba = new Mat(height, width, CvType.CV_8UC4);
        mGray = new Mat(height, width, CvType.CV_8UC1);
    }

    public void onCameraViewStopped() {
        mRgba.release();
        mGray.release();
    }

    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame currentFrame) {
        final int viewMode = mCameraViewMode;
        mRgba = currentFrame.rgba();
        mGray = currentFrame.gray();
        switch(viewMode) {
            case VIEW_CAMERA_RGBA:
                break;
            case VIEW_CAMERA_ORB: {
                    AlignFrame(mRgba.getNativeObjAddr(), mGray.getNativeObjAddr(), firstFrame);
                    firstFrame = false;
                } break;
            default:
                break;
        }

        return mGray;//mRgba;
    }


    private void showToasts(int id) {
        switch(id) {
            case DIALOG_SHOW_FIRST:
                Toast.makeText(this, "Click on the menu for openCV options.", Toast.LENGTH_LONG).show();
                break;
            case DIALOG_SHOW_ORB:
                Toast.makeText(this, "Displaying ORB keypoints.", Toast.LENGTH_LONG).show();
                break;
            default:
                break;
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void AlignFrame(long mRgba, long mGray, boolean isFirst);
}
