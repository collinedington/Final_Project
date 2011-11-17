CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(FinalProject)

# Some Windows specific Qt stuff
IF (WIN32)
  SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32)

# Find Qt
FIND_PACKAGE(Qt)
IF(QT_FOUND)
  INCLUDE(${QT_USE_FILE})
  INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
ELSE(QT_FOUND)
  MESSAGE(FATAL_ERROR "QT not found. Please set QT_DIR.") 
ENDIF(QT_FOUND)

# Find ITK 
INCLUDE (${CMAKE_ROOT}/Modules/FindITK.cmake) 
IF (USE_ITK_FILE)   
  INCLUDE(${USE_ITK_FILE}) 
ENDIF(USE_ITK_FILE)

# Find OpenCV
FIND_PACKAGE( OpenCV REQUIRED )

# Set additional include directories
SET(FinalProject_include_dirs
  ${FinalProject_SOURCE_DIR}
  ${QT_INCLUDE_DIR}
  ${FinalProject_SOURCE_DIR}/Library/Base
  ${FinalProject_SOURCE_DIR}/Library/Qt)

# Initial setting for the linker directories
SET(FinalProject_link_dirs
  ${FinalProject_SOURCE_DIR}
)

# Specify libraries to link against
SET(FinalProject_libraries

  # Qt libraries
  ${QT_LIBRARIES}

  # ITK libraries  
  ITKBasicFilters
  ITKCommon
  ITKIO
  ITKNumerics

  # OpenCV libraries
  ${OpenCV_LIBS}
)  

SET(FinalProject_files
  FinalProjectApp.cxx
  FinalProjectWindow.cxx
  main.cxx)

# Set headers that require MOC
SET(FinalProject_MOCHeaders
    FinalProjectApp.h
    FinalProjectWindow.h)


# Set UI files that need to be converted to classes
SET(UIS
    FinalProject.ui)

# Do Qt specific stuff
QT4_WRAP_UI(UIHeaders ${UIS})
QT4_WRAP_CPP(MOCSrcs ${FinalProject_MOCHeaders} )

# Make sure to include the wrapped UI output header
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# Qt compiler definitions
ADD_DEFINITIONS(-DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT -DQT_DLL)


SET_SOURCE_FILES_PROPERTIES(${FinalProject_files} PROPERTIES OBJECT_DEPENDS "${UIHeaders}")

# Add the UI definitions and headers to the project
SET (FinalProject_files ${FinalProject_files} ${UIS} ${FinalProject_MOCHeaders})

# Final specifications of directories where headers and libraries live
INCLUDE_DIRECTORIES(${FinalProject_include_dirs})
LINK_DIRECTORIES(${FinalProject_link_dirs})

# Add the final project executable as a Windows executable (rather than command line)
#ADD_EXECUTABLE(FinalProject WIN32 ${FinalProject_files} ${UISrcs} ${MOCSrcs})

# Add the final project executable as a command line executable
ADD_EXECUTABLE(FinalProject ${FinalProject_files} ${UISrcs} ${MOCSrcs})

# Add the final project executable as a command line executable
#ADD_EXECUTABLE(FinalProject ${FinalProject_files} ${UISrcs} ${MOCSrcs})

# Final specification for linker
TARGET_LINK_LIBRARIES(FinalProject ${FinalProject_libraries})
