/*=========================================================================

  University of Pittsburgh Bioengineering 1351/2351
  Final project example code

  Copyright (c) 2011 by Damion Shelton

  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _appBase_h
#define _appBase_h

#include <iostream>

#include <QImage>

#include <cv.h>
#include <highgui.h>
#include <QTime>

#include "itkImage.h"
#include "itkBinaryThresholdImageFilter.h"

class FinalProjectApp : public QObject
{
Q_OBJECT

public:

  // Image typedef
  typedef itk::Image< unsigned char, 2 > ImageType;
  typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> ThresholdType;

  /** Constructor */
  FinalProjectApp();

  /** Destructor */
  virtual ~FinalProjectApp();

  /** Setup the camera connection */
  void SetupApp();

public slots:

  /** Function to update the application in response to an external timer loop*/
  void RealtimeUpdate();

  /** Change from color to threshold image or vice versa */
  void SetApplyFilter(bool useFilter);

  /** Set the lower value for the threshold filter */
  void SetThreshold(int threshold);

  /** New Buttons and stuff that we added to the GUI*/
  void SetRadioButtonEyePairBig(bool bigEyePair);
  void SetRadioButtonEyePairSmall(bool smallEyePair); 
  void SetRadioButtonFrontalFace(bool frontalFace);
  void SetRadioButtonLeftRightEye(bool leftRightEye);
  void SetRadioButtonMouth(bool mouth);
  void SetRadioButtonNose(bool nose);

  /** Write the frame variables to the log file */
  void SaveLog();

  /** A slot that the external program can call to advance the trial stage.
  0 = Intertrial	1 = Button Press	2 = Reach				**/
  void AdvanceTrialStage(int nextStage);



signals:

  /** Send a QImage to a receiver */
  void SendImage(QImage image);

  /** update the attention bar */
  void updateAttentionBar(int attentionProgress);

  /** update the trial counter lcds **/
  void updateSuccessfulTrialsLCD(int successTrials);
  void updateFailedTrialsLCD(int failTrials);

protected:

  /** Setup the connection to the webcam */
  bool SetupCamera();

  /** Disconnect from the webcam */
  void DisconnectCamera();

  /** Configure the ITK pipeline to filter the acquired image data */
  void SetupITKPipeline();

  /** Copy the acquired image data to the ITK image */
  void CopyImageToITK();

  /** Convert an unsigned char buffer containing RGB data to a QImage */
  QImage RGBBufferToQImage(unsigned char* buffer);

  /** Convert an unsigned char buffer containing monochrome data to a QImage */
  QImage MonoBufferToQImage(unsigned char* buffer);

  /** Width of the image (# columns) in pixels */
  unsigned int m_ImageWidth;

  /** Height of the image (# rows) in pixels */
  unsigned int m_ImageHeight;

  /** The number of pixels in the image */
  unsigned int m_NumPixels;

  /** OpenCV webcam capture structure */
  CvCapture* m_OpenCVCapture;

  /** Flag to indicate camera connection; true if connected */
  bool m_ConnectedToCamera;

  /** The image in OpenCV format */
  IplImage* m_CameraImageOpenCV;

  /** Buffer containing RGB data from the camera */
  unsigned char* m_CameraFrameRGBBuffer;

  /** Temporary buffer used to store RGBA data for forming a QImage */
  unsigned char* m_TempRGBABuffer;

  /** The captured image in ITK format */
  ImageType::Pointer m_Image;

  /** ITK image threshold filter */
  ThresholdType::Pointer m_ThresholdFilter;

  /** Lower threshold value in filter */
  int m_Threshold;

  /** Is the filter enabled? */
  bool m_FilterEnabled;

  /** Radio Button Values */
  bool m_EyePairBigEnabled;
  bool m_EyePairSmallEnabled;
  bool m_FrontalFaceEnabled;
  bool m_LeftRightEyeEnabled;
  bool m_MouthEnabled;
  bool m_NoseEnabled;

  /** Haar Cascades for finding different Features */
  CvHaarClassifierCascade* m_HaarLeftEye;
  CvHaarClassifierCascade* m_HaarRightEye;
  CvHaarClassifierCascade* m_HaarEyePairSmall;
  CvHaarClassifierCascade* m_HaarEyePairBig;
  CvHaarClassifierCascade* m_HaarFrontalFace;
  CvHaarClassifierCascade* m_HaarMouth;
  CvHaarClassifierCascade* m_HaarNose;

  /** Find faces or eyes in an image using code from http://www.shervinemami.co.cc/faceRecognition.html */
  CvRect detectEyesInImage(IplImage *inputImg, CvHaarClassifierCascade* cascade);

  /** Convert IplImage to QtImage and vice-versa from http://umanga.wordpress.com/2010/04/19/how-to-covert-qt-qimage-into-opencv-iplimage-and-wise-versa/ */
  QImage* IplImage2QImage(IplImage *iplImg);
  IplImage* QImage2IplImage(QImage *qimg);

  /** Wrapper to reduce the amount of code we need to add into RealtimeUpdate for tracking. 
  Send in an image and haar template, and get a rectangle back (and drawn on the image)*/
  CvRect TrackFeature(IplImage* inputImg, CvHaarClassifierCascade* m_Cascade);
  CvHaarClassifierCascade* LoadHaarCascade(char* m_CascadeFilename);

  /**  Check to make sure two eye regions are not the same one.  From http://opencv-users.1802565.n2.nabble.com/cvRect-overlap-td3836140.html */
  CvRect intersect(CvRect r1, CvRect r2);

  /** Counter to see how much we've been successfully tracking */
  int m_attentionCounter;

  /** Initialize a frame index for the arrays */
  int m_frame;

  /** Initialize a log file */
  FILE *m_logFile;

  /** Initialize log file variables */
  int *m_Detect;
  double *m_TimeStamp;
  int *m_Trial;
  int *m_Feature;
  int *m_Stage;
  /*Switched to this method so instead of checking what stage/feature we're using each time,
  we simply reference the current state.  It saves a few if statements, and makes it easier to
  reference these variables in the future, instead of having to look into the array (if we make them dynamic)
  or having to run an if statement */
  int m_CurrentStage;
  int m_CurrentFeature;
  int m_CurrentTrial;
  QTime m_QTime;
  int m_SuccessfulTrials, m_FailedTrials;
  
};

#endif
