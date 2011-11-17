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
  m_LowerThreshold = 40;

  m_FilterEnabled = false;
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

  delete[] m_CameraFrameRGBBuffer;
  delete[] m_TempRGBABuffer;
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

    // Extract RGB data from captured image
    unsigned char * openCVBuffer = (unsigned char*)(m_CameraImageOpenCV->imageData);

    // Store the RGB data in our local buffer
    for(int b = 0; b < m_NumPixels * 3; b++)
    {
      m_CameraFrameRGBBuffer[b] = openCVBuffer[b];
    }

    // Update the ITK image
    this->CopyImageToITK();

    if(m_FilterEnabled)
    {
      // Update the filter. Since we're modifying the source image outside
      // of the normal pipeline mechanism, we need to let the filter know
      // that the input was modified
      m_ThresholdFilter->Modified();
      m_ThresholdFilter->Update();

      QImage processedImage = MonoBufferToQImage( m_ThresholdFilter->GetOutput()->GetPixelContainer()->GetBufferPointer() );
      emit SendImage( processedImage.copy() );
    }
    else
    {
      QImage cameraImage = RGBBufferToQImage(m_CameraFrameRGBBuffer);
 
      // Send a copy of the image out via signals/slots
      emit SendImage( cameraImage.copy() );
    }
  }
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

  m_ThresholdFilter = ThresholdType::New();
  m_ThresholdFilter->SetInput( m_Image );
  m_ThresholdFilter->SetOutsideValue( 255 );
  m_ThresholdFilter->SetInsideValue( 0 );
  m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
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
    m_TempRGBABuffer[rgbaCounter] = 255;
    rgbaCounter++;
  }

  // Convert to Qt format
  QImage result(m_TempRGBABuffer, m_ImageWidth, m_ImageHeight, QImage::Format_RGB32);
  return result;
}


void
FinalProjectApp
::SetApplyFilter(bool useFilter)
{
  m_FilterEnabled = useFilter;
}


 void
FinalProjectApp
::SetThreshold(int threshold)
 {
   m_LowerThreshold = threshold;
   m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
 }
