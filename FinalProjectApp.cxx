/*=========================================================================

  University of Pittsburgh Bioengineering 1351/2351
  Final project example code

  Copyright (c) 2011 by Damion Shelton

  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <time.h>
#include "FinalProjectApp.h"
#include <string.h>
#include <itkArray.h>
#include <random>

FinalProjectApp
::FinalProjectApp()
{
  std::cout << "In FinalProjectApp constructor" << std::endl;

  // Initialize OpenCV things to null
  m_CameraImageOpenCV = 0;
  m_OpenCVCapture = 0;

  // We know the image size in advance. A better implementation would be
  // to set this only after connecting to the camera and checking the
  // actual image size.
  m_ImageWidth = 640;
  m_ImageHeight = 480;
  m_NumPixels = m_ImageWidth * m_ImageHeight;

  // Buffers to hold raw image data
  m_CameraFrameRGBBuffer = new unsigned char[m_NumPixels*3];
  m_TempRGBABuffer = new unsigned char[m_NumPixels*4];

  // Not yet connected to a camera
  m_ConnectedToCamera = false;

  // Filter parameter
  m_Threshold = 40;
  m_EyePairBigEnabled = true;
  updateAttentionBar(m_Threshold);

  m_FilterEnabled = false;

  //Start the counter
  m_attentionCounter = 0;

  // Load whichever Haar cascades we'll need so we don't have to read from file for every frame
  m_HaarLeftEye = LoadHaarCascade("haarcascade_mcs_lefteye.xml");
  m_HaarRightEye = LoadHaarCascade("haarcascade_mcs_righteye.xml");
  m_HaarEyePairSmall = LoadHaarCascade("haarcascade_mcs_eyepair_small.xml");
  m_HaarEyePairBig = LoadHaarCascade("haarcascade_mcs_eyepair_big.xml");
  m_HaarFrontalFace = LoadHaarCascade("haarcascade_frontalface_default.xml");
  m_HaarMouth = LoadHaarCascade("haarcascade_mcs_mouth.xml");
  m_HaarNose = LoadHaarCascade("haarcascade_mcs_nose.xml");

  // Initialize a log file with hard coded headers
  m_logFile = fopen("Log File.csv","w");
  fprintf(m_logFile, "%s,%s,%s,%s,%s\n", "Time", "Trial", "Feature", "Detect", "Epoch");

  // Initialize the frame index for the arrays
  m_frame = 0;

  /** Initialize log file dynamic arrays */
  int TotalFrames = 10000; 
  m_TimeStamp = new double [TotalFrames];
  m_Trial = new int [TotalFrames];
  m_Feature = new int [TotalFrames];
  m_Detect = new int [TotalFrames];
  m_Epoch = new int [TotalFrames];
  m_CurrentEpoch = 0;
  m_CurrentFeature = 1; //This is overrided with -1 if tracking is not enabled
  m_QTime.start();
  m_CurrentTrial = 0;
  m_SuccessfulTrials = 0;
  m_FailedTrials = 0;

  //initialize random number seed to randomly advance the Epoch
  srand( time(NULL) );
}


FinalProjectApp
::~FinalProjectApp()
{
  std::cout << "In FinalProjectApp destructor" << std::endl;

  if(m_ConnectedToCamera)
  {
    std::cout << "In FinalProjectApp destructor: disconnecting camera" << std::endl;
    this->DisconnectCamera();
  }

  // Automatically save a log file upon exiting the program
  SaveLog();

  delete[] m_CameraFrameRGBBuffer;
  delete[] m_TempRGBABuffer;

  delete[] m_TimeStamp;
  delete[] m_Trial;
  delete[] m_Feature;
  delete[] m_Detect;
  delete[] m_Epoch;

  // Append feature definitions and Epoch numbers to the log file
  fprintf(m_logFile, "\n%s,\t%s,\t%s,\t%s,\t%s,\t%s", "bigEyePair = 1", "smallEyePair = 2", "frontalFace = 3", "leftRightEye = 4", "mouth = 5", "nose = 6");
  fprintf(m_logFile, "\n%s,\t%s,\t%s","Epoch 0 = Intertrial", "Epoch 1 = Button Press", "Epoch 2 = Reach");
  fclose(m_logFile);
}


void
FinalProjectApp
::SetupApp()
{
  if( this->SetupCamera() )
    this->SetupITKPipeline();
}


bool
FinalProjectApp
::SetupCamera()
{
  // Try to get any open camera
  m_OpenCVCapture = cvCaptureFromCAM(CV_CAP_ANY);

  // Proceed if we found a camera
  if(m_OpenCVCapture != 0)
  {
    // Set the width and height of the camera image
    cvSetCaptureProperty(m_OpenCVCapture, CV_CAP_PROP_FRAME_HEIGHT, m_ImageHeight); 
    cvSetCaptureProperty(m_OpenCVCapture, CV_CAP_PROP_FRAME_WIDTH, m_ImageWidth);

    // Succesfully opened the camera
    m_ConnectedToCamera = true;
    return true;
  }
  else // Oops, no camera
    return false;
}

void
FinalProjectApp
::DisconnectCamera()
{
  // Free the video capture object
  cvReleaseCapture(&m_OpenCVCapture); 
}

void
FinalProjectApp
::RealtimeUpdate()
{
  // If we're talking to the camera, we can do the cool stuff
  if(m_ConnectedToCamera)
  {
    // Snap an image from the webcam
    m_CameraImageOpenCV = cvQueryFrame(m_OpenCVCapture);

    // Did the capture fail?
    if(m_CameraImageOpenCV == NULL)
      return;

	/*  RGB extraction is not necessary for our purposes.  Keeping code just in case.
    // Extract RGB data from captured image
    unsigned char * openCVBuffer = (unsigned char*)(m_CameraImageOpenCV->imageData);

    // Store the RGB data in our local buffer
    for(int b = 0; b < m_NumPixels * 3; b++)
    {
      m_CameraFrameRGBBuffer[b] = openCVBuffer[b];
    }
	

    // Update the ITK image
    this->CopyImageToITK();
	*/
    if(m_FilterEnabled)
    {
		  //Determine which radiobutton is selected and track appropriately
		  if(m_EyePairBigEnabled)
		  {
			  CvRect m_EyePairBig = TrackFeature(m_CameraImageOpenCV, m_HaarEyePairBig);
		  }
		  else if(m_EyePairSmallEnabled)
		  {
			  CvRect m_EyePairSmall = TrackFeature(m_CameraImageOpenCV, m_HaarEyePairSmall);
		  }
		  else if(m_FrontalFaceEnabled)
		  {
			  CvRect m_FrontalFace = TrackFeature(m_CameraImageOpenCV, m_HaarFrontalFace);
		  }
		  else if(m_LeftRightEyeEnabled)
		  {
			  CvRect m_LeftEye = TrackFeature(m_CameraImageOpenCV, m_HaarLeftEye);
			  CvRect m_RightEye = TrackFeature(m_CameraImageOpenCV, m_HaarRightEye);
		  }
		  else if(m_MouthEnabled)
		  {
			  CvRect m_Mouth = TrackFeature(m_CameraImageOpenCV, m_HaarMouth);
		  }
		  else if(m_NoseEnabled)
		  {
			  CvRect m_Nose = TrackFeature(m_CameraImageOpenCV, m_HaarNose);
		  }
		

		  QImage processedImage = *IplImage2QImage(m_CameraImageOpenCV);
		  emit SendImage( processedImage );
		  emit updateAttentionBar( m_attentionCounter );
		  m_Feature[m_frame] = m_CurrentFeature;
    }

    else
    {
		
      // Signify with -1 that no tracking is being conducted
      m_Detect[m_frame] = -1;
	    m_Feature[m_frame] = -1;
	    
      // Log for the attention bar
      if(m_attentionCounter >0) {
				m_attentionCounter--;
			}
		  
      QImage processedImage = *IplImage2QImage(m_CameraImageOpenCV);
		  // Send a copy of the image out via signals/slots
		  emit SendImage( processedImage );
		  emit updateAttentionBar( m_attentionCounter );
    }

    // Within capture image but outside filter if statement
    m_Trial[m_frame] = m_CurrentTrial;
    m_Epoch[m_frame] = m_CurrentEpoch;
	
	  m_TimeStamp[m_frame] = ((double)m_QTime.elapsed())/1000;
    
    // Create a frame index, make sure we don't overwrite the 
    if(m_frame > 9998) SaveLog(); //m_frame will be reset to zero inside SaveLog()
	  
    // Proceed to the next frame index
    else m_frame++;
  }
 
  /** Randomly advance to the next Epoch.  This will be replaced by signals from 
  the external controller program, but for now we want to make sure it's working **/
  if(rand() % 10 == 1)AdvanceTrialEpoch( (m_CurrentEpoch+1)%3 ); 
  // 10% chance to advance, set nextEpoch to next value, wrap from 2->0

}


void
FinalProjectApp
::SetupITKPipeline()
{
  // Image size and spacing parameters
  unsigned long sourceImageSize[]  = { m_ImageWidth, m_ImageHeight};
  double sourceImageSpacing[] = { 1.0,1.0 };
  double sourceImageOrigin[] = { 0,0 };

  // Creates the sourceImage (but doesn't set the size or allocate memory)
  m_Image = ImageType::New();
  m_Image->SetOrigin(sourceImageOrigin);
  m_Image->SetSpacing(sourceImageSpacing);

  // Create a size object native to the sourceImage type
  ImageType::SizeType sourceImageSizeObject;

  // Set the size object to the array defined earlier
  sourceImageSizeObject.SetSize( sourceImageSize );

  // Create a region object native to the sourceImage type
  ImageType::RegionType largestPossibleRegion;

  // Resize the region
  largestPossibleRegion.SetSize( sourceImageSizeObject );

  // Set the largest legal region size (i.e. the size of the whole sourceImage) to what we just defined
  m_Image->SetRegions( largestPossibleRegion );

  // Now allocate memory for the sourceImage
  m_Image->Allocate();

  //---------Next, set up the filter
  // This is really to setup the successful trial threshold
  m_ThresholdFilter = ThresholdType::New();
  m_ThresholdFilter->SetInput( m_Image );
  m_ThresholdFilter->SetOutsideValue( 255 );
  m_ThresholdFilter->SetInsideValue( 0 );
  m_ThresholdFilter->SetLowerThreshold( m_Threshold );
  m_ThresholdFilter->SetUpperThreshold( 255 ); 
}


void
FinalProjectApp
::CopyImageToITK()
{
  // Create the iterator
  itk::ImageRegionIterator<ImageType> it = 
    itk::ImageRegionIterator<ImageType>(m_Image, m_Image->GetLargestPossibleRegion() );

  // Move iterator to the start of the ITK image
  it.GoToBegin();

  unsigned int linearByteIndex = 0;
  double r, g, b;

  for(int p = 0; p < m_NumPixels; p++)
  {
    r = (double)(m_CameraFrameRGBBuffer[linearByteIndex]);
    linearByteIndex++;
    g = (double)(m_CameraFrameRGBBuffer[linearByteIndex]);
    linearByteIndex++;
    b = (double)(m_CameraFrameRGBBuffer[linearByteIndex]);
    linearByteIndex++;
    
    /*Convert from RGB to grayscale
      If you're wondering why the scale factors are the way they are, the answer
      is that it's just one possible weighting. "Correct" perceptual values
      depend on the interaction of camera and human eye response and 
      will vary from individual to individual and camera to camera.

      As an additional note, the OpenCV image is actually storing color data,
      and the FLTK display image (see app.h/app.cxx) is capable of displaying
      color data. I'm discarding it when copying to the ITK image because
      grayscale data is easier to work with. If you want to do a project
      involving color data, keep in mind that it's already here. You just
      have to use it - nothing will change in the capture code. */
    unsigned char grayscale = (unsigned char)(0.3*r + 0.59*g + 0.11*b);
    it.Set(grayscale);

    // Move to next ITK pixel
    ++it;
  }
}


 QImage
 FinalProjectApp
::RGBBufferToQImage(unsigned char* buffer)
{
  unsigned int rgbCounter = 0;
  unsigned int rgbaCounter = 0;

  for(int b = 0; b < m_NumPixels; b++)
  {
    // Red
    m_TempRGBABuffer[rgbaCounter] = buffer[rgbCounter];
    rgbCounter++;
    rgbaCounter++;

    // Green
    m_TempRGBABuffer[rgbaCounter] = buffer[rgbCounter];
    rgbCounter++;
    rgbaCounter++;

    // Blue
    m_TempRGBABuffer[rgbaCounter] = buffer[rgbCounter];
    rgbCounter++;
    rgbaCounter++;

    // Alpha
    m_TempRGBABuffer[rgbaCounter] = 255;
    rgbaCounter++;
  }

  // Convert to Qt format
  QImage result(m_TempRGBABuffer, m_ImageWidth, m_ImageHeight, QImage::Format_RGB32);
  return result;
}

QImage
FinalProjectApp
::MonoBufferToQImage(unsigned char* buffer)
{
  unsigned int rgbaCounter = 0;

  for(int b = 0; b < m_NumPixels; b++)
  {
    // Red
    m_TempRGBABuffer[rgbaCounter] = buffer[b];
    rgbaCounter++;

    // Green
    m_TempRGBABuffer[rgbaCounter] = buffer[b];
    rgbaCounter++;

    // Blue
    m_TempRGBABuffer[rgbaCounter] = buffer[b];
    rgbaCounter++;

    // Alpha
    m_TempRGBABuffer[rgbaCounter] = 128;
    rgbaCounter++;
  }

  // Convert to Qt format
  QImage result(m_TempRGBABuffer, m_ImageWidth, m_ImageHeight, QImage::Format_RGB32);
  return result;
}

// The radio button functions to choose the tracked feature
void
FinalProjectApp
::SetApplyFilter(bool useFilter)
{
  m_FilterEnabled = useFilter;
}

void
FinalProjectApp
::SetRadioButtonEyePairBig(bool bigEyePair){
	m_EyePairBigEnabled = bigEyePair;
	m_CurrentFeature = 1;
}

void 
FinalProjectApp
::SetRadioButtonEyePairSmall(bool smallEyePair){
	m_EyePairSmallEnabled = smallEyePair;
	 m_CurrentFeature = 2;
  }

void 
FinalProjectApp
::SetRadioButtonFrontalFace(bool frontalFace){
	m_FrontalFaceEnabled = frontalFace;
	 m_CurrentFeature = 3;
}

void 
FinalProjectApp
::SetRadioButtonLeftRightEye(bool leftRightEye){
	m_LeftRightEyeEnabled = leftRightEye;
	 m_CurrentFeature = 4;
}

void 
FinalProjectApp
::SetRadioButtonMouth(bool mouth){
	m_MouthEnabled = mouth;
	 m_CurrentFeature = 5;
}

void 
FinalProjectApp
::SetRadioButtonNose(bool nose){
	m_NoseEnabled = nose;
	 m_CurrentFeature = 6;
}

// Load the settings file for which ever feature was selected to be tracked
CvHaarClassifierCascade* 
FinalProjectApp
::LoadHaarCascade(char* m_CascadeFilename)
{
	CvHaarClassifierCascade* m_Cascade = (CvHaarClassifierCascade*)cvLoad(m_CascadeFilename, 0, 0, 0);
	if( !m_Cascade ) {
		printf("Couldnt load Haar Cascade '%s'\n", m_CascadeFilename);
		exit(1);
	}
	return m_Cascade;
}

// The function to actually track the feature
CvRect
FinalProjectApp
::TrackFeature(IplImage* inputImg, CvHaarClassifierCascade* m_Cascade)
{
	// Perform face detection on the input image, using the given Haar classifier
	CvRect eyeRect = detectEyesInImage(inputImg, m_Cascade);

  // Time stamp when the frame was taken
  m_TimeStamp[m_frame] = m_frame;

	// Make sure a valid face was detected.
	if (eyeRect.width > 0) {
    if(m_attentionCounter < m_Threshold) {
				m_attentionCounter++;
			}
			
    m_Detect[m_frame] = 1;
	}
  
  // No valid face was detected
  else {
    if(m_attentionCounter >0) {
				m_attentionCounter--;
			}

    m_Detect[m_frame] = 0;
  }

	// Trace a red rectangle over the detected area
	cvRectangle(inputImg,cvPoint(eyeRect.x,eyeRect.y), cvPoint(eyeRect.x+eyeRect.width,eyeRect.y+eyeRect.height), CV_RGB(255,0,0), 1, 8, 0);
	return eyeRect;
}

 void
FinalProjectApp
::SetThreshold(int threshold)
 {
   m_Threshold = threshold;
   m_ThresholdFilter->SetLowerThreshold( m_Threshold );
   m_attentionCounter = 0;
 }

 // Create a log file from the data arrays
void 
FinalProjectApp
::SaveLog()
{
  for(int i = 0; i < m_frame; i++) {
      fprintf(m_logFile, "%f,%i,%i,%i,%i\n", m_TimeStamp[i], m_Trial[i], m_Feature[i], m_Detect[i], m_Epoch[i]);
	}
  // Reset the arrays to continue recording data
  m_frame = 0;

  std::cout << "Saved log file\n";
}

/** Create an artificial function to cycle through the epochs, simulating trials.
Replace with signals from the control computer when possible. */
void
FinalProjectApp
::AdvanceTrialEpoch(int nextEpoch)
{
	if(nextEpoch>=0 && nextEpoch<=2){
		m_CurrentEpoch = nextEpoch;
		//Since the Epoch has advanced, lets see if we should update the LCDs
		if(m_CurrentEpoch == 0){
			m_CurrentTrial++;
			//Determine if we call this a success, subject to change
			if(((double)m_attentionCounter) / ((double)m_Threshold) > 0.5) m_SuccessfulTrials++;
			else m_FailedTrials++;
			emit updateSuccessfulTrialsLCD(m_SuccessfulTrials);
			emit updateFailedTrialsLCD(m_FailedTrials);
		}
	}
	else printf("Error, invalid number for next Epoch (%i)",nextEpoch);
}

 //*******************  Copied Code   ****************************
 // Perform face detection on the input image, using the given Haar Cascade.
// Returns a rectangle for the detected region in the given image.
CvRect 
FinalProjectApp
::detectEyesInImage(IplImage *inputImg, CvHaarClassifierCascade* cascade)
{
	// Smallest face size.
	CvSize minFeatureSize = cvSize(10, 10);
	// Only search for 1 face.
	int flags = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
	// How detailed should the search be.
	float search_scale_factor = 1.1f; //default 1.1f
	IplImage *detectImg;
	IplImage *greyImg = 0;
	CvMemStorage* storage;
	CvRect rc;
	double t;
	CvSeq* rects;
	CvSize size;
	int i, ms, nFaces;

	storage = cvCreateMemStorage(0);
	cvClearMemStorage( storage );


	// If the image is color, use a greyscale copy of the image.
	detectImg = (IplImage*)inputImg;
	if (inputImg->nChannels > 1) {
		size = cvSize(inputImg->width, inputImg->height);
		greyImg = cvCreateImage(size, IPL_DEPTH_8U, 1 );
		cvCvtColor( inputImg, greyImg, CV_BGR2GRAY );
		detectImg = greyImg;	// Use the greyscale image.
	}

	// Detect all the faces in the greyscale image.
	t = (double)cvGetTickCount();
	rects = cvHaarDetectObjects( detectImg, cascade, storage,
			search_scale_factor, 3, flags, minFeatureSize);
	t = (double)cvGetTickCount() - t;
	ms = cvRound( t / ((double)cvGetTickFrequency() * 1000.0) );
	nFaces = rects->total;
	//uncomment for debugging
	//printf("Face Detection took %d ms and found %d objects\n", ms, nFaces);

	// Get the first detected face (the biggest).
	if (nFaces > 0)
		rc = *(CvRect*)cvGetSeqElem( rects, 0 );
	else
		rc = cvRect(-1,-1,-1,-1);	// Couldn't find the face.

	if (greyImg)
		cvReleaseImage( &greyImg );
	cvReleaseMemStorage( &storage );
	//Now that we have permanent cascades (to improve performance) we don't want to release them
	//cvReleaseHaarClassifierCascade( &cascade );

	return rc;	// Return the biggest face found, or (-1,-1,-1,-1).
}

QImage*
FinalProjectApp
::IplImage2QImage(IplImage *iplImg)
{
	int h = iplImg->height;
	int w = iplImg->width;
	int channels = iplImg->nChannels;
	QImage *qimg = new QImage(w, h, QImage::Format_ARGB32);
	char *data = iplImg->imageData;

	for (int y = 0; y < h; y++, data += iplImg->widthStep)
	{
		for (int x = 0; x < w; x++)
		{
			char r, g, b, a = 0;
			if (channels == 1)
			{
				r = data[x * channels];
				g = data[x * channels];
				b = data[x * channels];
			}
			else if (channels == 3 || channels == 4)
			{
				r = data[x * channels + 2];
				g = data[x * channels + 1];
				b = data[x * channels];
			}

			if (channels == 4)
			{
				a = data[x * channels + 3];
				qimg->setPixel(x, y, qRgba(r, g, b, a));
			}
			else
			{
				qimg->setPixel(x, y, qRgb(r, g, b));
			}
		}
	}
	return qimg;

}

IplImage* 
FinalProjectApp
::QImage2IplImage(QImage *qimg)
{

	IplImage *imgHeader = cvCreateImageHeader( cvSize(qimg->width(), qimg->height()), IPL_DEPTH_8U, 4);
	imgHeader->imageData = (char*) qimg->bits();

	uchar* newdata = (uchar*) malloc(sizeof(uchar) * qimg->byteCount());
	memcpy(newdata, qimg->bits(), qimg->byteCount());
	imgHeader->imageData = (char*) newdata;
	//cvClo
	return imgHeader;
}

CvRect FinalProjectApp
::intersect(CvRect r1, CvRect r2) 
{ 
    CvRect intersection; 
    
    // find overlapping region 
    intersection.x = (r1.x < r2.x) ? r2.x : r1.x; 
    intersection.y = (r1.y < r2.y) ? r2.y : r1.y; 
    intersection.width = (r1.x + r1.width < r2.x + r2.width) ? 
        r1.x + r1.width : r2.x + r2.width; 
    intersection.width -= intersection.x; 
    intersection.height = (r1.y + r1.height < r2.y + r2.height) ? 
        r1.y + r1.height : r2.y + r2.height; 
    intersection.height -= intersection.y;     
    
    // check for non-overlapping regions 
    if ((intersection.width <= 0) || (intersection.height <= 0)) { 
        intersection = cvRect(0, 0, 0, 0); 
    } 
    
    return intersection; 
} 

