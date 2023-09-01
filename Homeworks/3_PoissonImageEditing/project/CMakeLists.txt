cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(PoissonImageEditing VERSION 1.0)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_QtInit(COMPONENTS Core Gui Widgets)
set(OpenCV_DIR "E:\\Download\\opencv_build")
find_package(OpenCV REQUIRED)
include_directories("E:\\Download\\opencv_build\\install\\include")
find_package(Eigen3 REQUIRED)
include_directories("C:\\Program Files (x86)\\Eigen3\\include")

Ubpa_AddSubDirsRec(src)
