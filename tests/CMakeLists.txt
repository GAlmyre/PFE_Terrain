# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Widgets OpenGL Test REQUIRED)

add_executable (VariableOptionTest VariableOptionTest.cpp ../include/VariableOption.h)
target_link_libraries (VariableOptionTest
  Qt5::Widgets Qt5::Core Qt5::OpenGL Qt5::Test GL -L/usr/X11R6/lib -lm -lpthread -lX11
  )
add_test (NAME VariableOptionTest COMMAND VariableOptionTest)

add_executable (DirectionalLightTest DirectionalLightTest.cpp ../include/DirectionalLight.h ../src/DirectionalLight.cpp)
target_link_libraries (DirectionalLightTest
  Qt5::Widgets Qt5::Core Qt5::OpenGL Qt5::Test GL -L/usr/X11R6/lib -lm -lpthread -lX11
  )
add_test (NAME DirectionalLightTest COMMAND DirectionalLightTest)
