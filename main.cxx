/*=========================================================================

  University of Pittsburgh Bioengineering 1351/2351
  Final project example code

  Copyright (c) 2011 by Damion Shelton

  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <qapplication.h>
#include "FinalProjectWindow.h"

// main is very short... most of the action occurs in the window
// and application classes and main only serves to launch the window.
// This is very typical of more advanced application design
int main( int argc, char** argv )
{
  std::cout << "Creating QApplication" << std::endl;
  QApplication app( argc, argv );
  
  std::cout << "Creating FinalProjectWindow" << std::endl;
  FinalProjectWindow* mainWindow = new FinalProjectWindow;
  mainWindow->show();
  mainWindow->repaint();
  
  std::cout << "Starting app event loop" << std::endl;


  return app.exec();
}