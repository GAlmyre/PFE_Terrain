cmake_minimum_required(VERSION 3.5)

# project name
project(TerrainTintin)

set(CMAKE_CXX_STANDARD 11)

# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)

#Variable "source_files" to store all the files in src directory
file(
  GLOB_RECURSE
  source_files
  src/*
  include/*
)

include_directories(ext/)
include_directories(assets/)
include_directories(include/)
include_directories(ext/eigen3/)
include_directories(ext/CImg/)

# Executable + source_files
add_executable(
  TerrainTintin
  ${source_files}
        include/utils.h include/Scene.h include/TerrainScene.h include/TessTestScene.h include/TessTerrainScene.h include/Sphere.h)

target_link_libraries(TerrainTintin Qt5::Widgets Qt5::Core Qt5::OpenGL GL -L/usr/X11R6/lib -lm -lpthread -lX11)
