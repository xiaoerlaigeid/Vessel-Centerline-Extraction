cmake_minimum_required(VERSION 2.8)
 
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
 
PROJECT(CenterlineExtraction)
 
find_package(VMTK REQUIRED)
#include(${VMTK_USE_FILE})
include_directories(${VMTK_INCLUDE_DIRS})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  # Instruct CMake to run moc automatically when needed.
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Widgets REQUIRED QUIET)
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif()
 
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
 
file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CPP_FILES *.cpp)
file(GLOB CXX_FILES *.cxx)
 
set(ENV_PATH "PATH=%PATH%;")
set(ENV_PATH "${ENV_PATH}${_qt5_install_prefix}/../../bin;")

if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  qt5_wrap_ui(UISrcs ${UI_FILES} )
  # CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
  add_executable(CenterlineExtraction MACOSX_BUNDLE

    ${CXX_FILES} ${CPP_FILES} ${UISrcs} ${QT_WRAP})
  qt5_use_modules(CenterlineExtraction Core Gui)
  target_link_libraries(CenterlineExtraction ${VTK_LIBRARIES})
  set(ENV_PATH "${ENV_PATH}${VTK_INSTALL_PREFIX}/bin;")
else()
  QT4_WRAP_UI(UISrcs ${UI_FILES})
  QT4_WRAP_CPP(MOCSrcs ${QT_WRAP})
  add_executable(CenterlineExtraction MACOSX_BUNDLE ${CXX_FILES} ${CPP_FILES} ${UISrcs} ${MOCSrcs})
  set(ENV_PATH "${ENV_PATH}${VTK_INSTALL_PREFIX}/bin;")
  if(VTK_LIBRARIES)
    if(${VTK_VERSION} VERSION_LESS "6")
      target_link_libraries(CenterlineExtraction ${VTK_LIBRARIES} QVTK)
    else()
      target_link_libraries(CenterlineExtraction ${VTK_LIBRARIES} ${QT_LIBRARIES})
    endif()
  else()
    target_link_libraries(CenterlineExtraction vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
  endif()
endif()

if(VMTK_LIBRARIES)
	target_link_libraries(CenterlineExtraction ${VMTK_LIBRARIES})
	set(ENV_PATH "${ENV_PATH}${VMTK_INSTALL_PREFIX}/bin;")
endif()

##########################################################################
#设置VS的调试环境
if(WIN32)
  message("evn path: ${ENV_PATH}")
  set_target_properties(CenterlineExtraction
		PROPERTIES VS_DEBUGGER_ENVIRONMENT "${ENV_PATH}"
	)
endif()
