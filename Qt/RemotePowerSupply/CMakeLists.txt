cmake_minimum_required(VERSION 3.5)

project(RemotePowerSupply LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network Core SerialPort REQUIRED)

# Include thirdparty libraries
include(thirdparty/thirdparty_dependencies.cmake)

######################################Include everuthing src##################################
file(GLOB_RECURSE Proj_SOURCES "src/*.c" "src/*.cpp" "ui/*.cpp")
file(GLOB_RECURSE Proj_HEADERS "src/*.h" "src/*.hpp" "ui/*.h")
file(GLOB_RECURSE Proj_UIS     "ui/*.ui")
file(GLOB_RECURSE Proj_RES     "res/*.qrc")

foreach (_headerFile ${Proj_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Proj_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES Proj_INCLUDE_DIRS)

include_directories(${Proj_INCLUDE_DIRS})
link_directories(${Proj_INCLUDE_DIRS})
##############################################################################################

# Sum up all sources into a single variable
set(ALL_SOURCES ${Proj_UIS} ${Proj_SOURCES} ${Proj_RES} ${THIRD_PARTY_SRC})

IF (WIN32)
    # Icon How-To:
    # https://forum.qt.io/topic/122430/setting-up-the-application-information-in-cmake/4

    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/res/win.rc")
    add_executable(${PROJECT_NAME} WIN32 ${ALL_SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
ELSE()
    add_executable(${PROJECT_NAME}${Proj_UIS} ${ALL_SOURCES})
ENDIF()

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${THIRD_PARTY_LIBS}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets)

# Deploy app
include(deploy.cmake)
