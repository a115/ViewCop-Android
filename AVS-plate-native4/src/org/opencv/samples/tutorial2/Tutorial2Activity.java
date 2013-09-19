package org.opencv.samples.tutorial2;

import org.opencv.android.BaseLoaderCallback;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.imgproc.Imgproc;
import org.opencv.highgui.*;
import org.opencv.core.*;

import com.googlecode.tesseract.android.TessBaseAPI;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;


//import com.googlecode.tesseract.android.TessBaseAPI;

public class Tutorial2Activity extends Activity implements CvCameraViewListener2 {
    private static final String    TAG = "OCVSample::Activity";

    private static final int       VIEW_MODE_SOURCE     = 0;
    private static final int       VIEW_MODE_SOBEL      = 1;
    private static final int       VIEW_MODE_MORPH      = 2;
    private static final int       VIEW_MODE_DEBUG      = 3;
    private static final int       VIEW_MODE_PLATE_ONLY = 4;
    
    private int                    mViewMode;
    private Mat                    mRgba;
    private Mat                    mIntermediateMat;
    private Mat                    mGray;
    private Mat                    mPlateOnly;

    private MenuItem               mItemViewSource;
    private MenuItem               mItemViewSobel;
    private MenuItem               mItemViewMorph;
    private MenuItem               mItemViewDebug;
    private MenuItem               mItemViewPlateOnly;

    private CameraBridgeViewBase   mOpenCvCameraView;

    TessBaseAPI m_baseApi;
    
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("mixed_sample");

                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    public Tutorial2Activity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.tutorial2_surface_view);

        // Get Tesseract set up
        m_baseApi = new TessBaseAPI();
        m_baseApi.init("/mnt/sdcard/tesseract", "eng");
        
        
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.tutorial2_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "called onCreateOptionsMenu");
        mItemViewSource = menu.add("Source");
        mItemViewSobel = menu.add("Sobel");
        mItemViewMorph = menu.add("Morph");
        mItemViewDebug = menu.add("Debug");
        mItemViewPlateOnly = menu.add("Plate");
        return true;
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();

        // Unload Tesseract
        m_baseApi.end();

    }

    public void onCameraViewStarted(int width, int height) {
        mRgba = new Mat(height, width, CvType.CV_8UC4);
        mIntermediateMat = new Mat(height, width, CvType.CV_8UC4);
        mGray = new Mat(height, width, CvType.CV_8UC1);
    }

    public void onCameraViewStopped() {
        mRgba.release();
        mGray.release();
        mIntermediateMat.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
    	
        final int viewMode = mViewMode;
        //int iDebugMode;
        int iPlateWidth = -1;
        int iPlateHeight = -1;
        
       // Matrix mtx = new Matrix();
        
        
        mRgba = inputFrame.rgba();
        mPlateOnly = new Mat();
        // input frame has RGBA format
            
        // NOTE:  In the following line, we can only pass viewMode directly
        // if the values defined have been kept in line with the C++ AVS-ANPR.h 
        // #defines - otherwise we'll need a switch statement here to feed 
        // in the correct values.
        ANPR(mRgba.getNativeObjAddr(), mPlateOnly.getNativeObjAddr(), viewMode);

        
        //Bitmap bmp = Bitmap.createBitmap(mRgba.cols(), mRgba.rows(),Bitmap.Config.ARGB_8888);
        //Utils.matToBitmap(mRgba, bmp);
        
        // If we have a candidate plate to look at, get it into the correct format
        // and OCR it.
        if (mPlateOnly.width() > 0 && mPlateOnly.height() > 0) {

            //TessBaseAPI baseApi = new TessBaseAPI();
            //baseApi.init("/mnt/sdcard/tesseract", "eng");
        	
            Bitmap bmp = Bitmap.createBitmap(mPlateOnly.cols(), mPlateOnly.rows(),Bitmap.Config.ARGB_8888);
            Utils.matToBitmap(mPlateOnly, bmp);

            
            //Bitmap bmp = new Bitmap(inputFrame);
            
            m_baseApi.setImage(bmp);
            String recognisedText = m_baseApi.getUTF8Text();
            
            Log.i(TAG, "PLATE WIDTH=" + Integer.toString(mPlateOnly.width()) + ", PLATE HEIGHT=" + Integer.toString(mPlateOnly.height()));
            Log.i(TAG, "OCR RESULT");
            Log.i(TAG, recognisedText);
            
            Core.putText(mRgba, recognisedText, new Point(10,100),  Core.FONT_HERSHEY_PLAIN, 3,  new Scalar(0,255,0,255));
            //baseApi.end();
        	
            }
        
        return mRgba;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "called onOptionsItemSelected; selected item: " + item);

        if (item == mItemViewSource) {
            mViewMode = VIEW_MODE_SOURCE;
        } else if (item == mItemViewSobel) {
            mViewMode = VIEW_MODE_SOBEL;
        } else if (item == mItemViewMorph) {
            mViewMode = VIEW_MODE_MORPH;
        } else if (item == mItemViewDebug) {
            mViewMode = VIEW_MODE_DEBUG;
        } else if (item == mItemViewPlateOnly) {
            mViewMode = VIEW_MODE_PLATE_ONLY;
        }

        return true;
    }

    public native void FindFeatures(long matAddrGr, long matAddrRgba);
    public native void ANPR(long matAddrRgba, long matAddrPlate, int iDebugMode);
}
