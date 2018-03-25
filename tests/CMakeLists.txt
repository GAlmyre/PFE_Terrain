# Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Widgets OpenGL REQUIRED)


find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  )
add_definitions (-DBOOST_TEST_DYN_LINK)


add_executable (VariableOptionTest VariableOptionTest.cpp ../include/VariableOption.h)
target_link_libraries (VariableOptionTest
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  Qt5::Widgets Qt5::Core Qt5::OpenGL GL -L/usr/X11R6/lib -lm -lpthread -lX11
  )
add_test (NAME VariableOptionTest COMMAND VariableOptionTest)
