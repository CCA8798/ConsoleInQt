cmake_minimum_required(VERSION 3.10)

project(ConsoleInQt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_PREFIX_PATH "F:/Software/Qt5/Qt5.14.2/5.14.2/mingw73_64")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_library(ConsoleInQt SHARED
  ConsoleInQt_global.h
  Console.cpp
  Console.h
)

target_link_libraries(ConsoleInQt PRIVATE Qt5::Widgets)

target_compile_definitions(ConsoleInQt PRIVATE CONSOLEINQT_LIBRARY)