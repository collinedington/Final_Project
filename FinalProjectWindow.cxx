/*=========================================================================

  University of Pittsburgh Bioengineering 1351/2351
  Final project example code

  Copyright (c) 2011 by Damion Shelton

  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "FinalProjectWindow.h"

#include <QtGui>
#include <QPixmap>

FinalProjectWindow
::FinalProjectWindow(QWidget* parent)
{
  std::cout << "In FinalProjectWindow constructor" << std::endl;

  // Initial call to configure the Qt window
  setupUi(this);
 
  // Setup graphics scene and view used to display the camera image
  graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  m_GraphicsScene = new QGraphicsScene(this);
  QPixmap tempPixmap(640, 480);
  tempPixmap.fill(Qt::blue);
  m_PixmapItem = new QGraphicsPixmapItem(tempPixmap, 0);
  m_GraphicsScene->addItem(m_PixmapItem);
  
  graphicsView->setScene(m_GraphicsScene);
  graphicsView->scale(1.0, 1.0);

  // Create the app
  m_App = new FinalProjectApp;
  m_App->SetupApp();

  // Connect signals/slots within the GUI
  connect(thresholdSlider, SIGNAL( valueChanged(int) ), thresholdSpinBox, SLOT( setValue(int) ) );
  connect(thresholdSpinBox, SIGNAL( valueChanged(int) ), thresholdSlider, SLOT( setValue(int) ) );
  //my addition - sets the maximum of the attention bar to the threshold slider value
  connect(thresholdSpinBox, SIGNAL( valueChanged(int) ), attentionBar, SLOT( setMaximum(int) ) );

  //My additions
  connect(radioButtonEyePairBig, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonEyePairBig(bool)) );
  connect(radioButtonEyePairSmall, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonEyePairSmall(bool)) );
  connect(radioButtonFrontalFace, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonFrontalFace(bool)) );
  connect(radioButtonLeftRightEye, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonLeftRightEye(bool)) );
  connect(radioButtonMouth, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonMouth(bool)) );
  connect(radioButtonNose, SIGNAL( toggled(bool) ), m_App, SLOT( SetRadioButtonNose(bool)) );
  
  connect(m_App, SIGNAL( updateAttentionBar(int)), attentionBar , SLOT( setValue(int) ));

  // Connect signals/slots to the app
  connect(m_App, SIGNAL( SendImage(QImage) ), this, SLOT( OnReceiveImage(QImage) ));
  connect(applyThresholdCheckBox, SIGNAL( toggled(bool) ), m_App, SLOT( SetApplyFilter(bool) ));
  connect(thresholdSpinBox, SIGNAL( valueChanged(int) ), m_App, SLOT( SetThreshold(int) ));
  connect(saveButton, SIGNAL( clicked() ), m_App, SLOT( SaveLog() ));

  // Start the event timer
  m_TimerID = startTimer(33);
}


FinalProjectWindow
::~FinalProjectWindow()
{
  std::cout << "In FinalProjectWindow destructor" << std::endl;
}


void
FinalProjectWindow
::closeEvent(QCloseEvent *event)
{
  std::cout << "In FinalProjectWindow::closeEvent" << std::endl;

  // Delete the app, which also stops the camera capture process
  delete m_App;

  // Since we've closed the window, we also want to quit the app
  // Note that qApp is a global variable that refers to the overall
  // Qt application structure, not the final project app class
  qApp->exit();
}


void
FinalProjectWindow
::timerEvent(QTimerEvent* event)
{
  // If the timer event applied to this window, process it
  // Otherwise pass the event to the base class handler
  if (event->timerId() == m_TimerID)
  {
    this->RealtimeUpdate();
  }
  else
  {
    QWidget::timerEvent(event);
  }
}

void
FinalProjectWindow
::RealtimeUpdate()
{
  // Update the app
  m_App->RealtimeUpdate();
}


void
FinalProjectWindow
::OnReceiveImage(QImage image)
{
  // Display the image
  m_PixmapItem->setPixmap(QPixmap::fromImage(image));
}
