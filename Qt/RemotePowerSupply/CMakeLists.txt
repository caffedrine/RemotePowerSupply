cmake_minimum_required(VERSION 3.5)

project(RemotePowerSupply LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Widgets
    Core
    SerialPort
    Network
    REQUIRED)

######################################Include everuthing src##################################
file(GLOB_RECURSE Proj_SOURCES "src/*.c" "src/*.cpp")
file(GLOB_RECURSE Proj_HEADERS "src/*.h" "src/*.hpp")
file(GLOB_RECURSE Proj_UIS     "src/*.ui")
file(GLOB_RECURSE Proj_RES     "res/*.qrc")

foreach (_headerFile ${Proj_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND Proj_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES Proj_INCLUDE_DIRS)

include_directories(${Proj_INCLUDE_DIRS})
link_directories(${Proj_INCLUDE_DIRS})
##############################################################################################

IF (WIN32)
    # Icon How-To:
    # https://forum.qt.io/topic/122430/setting-up-the-application-information-in-cmake/4

    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/res/win.rc")
    add_executable(${PROJECT_NAME} WIN32 ${Proj_UIS} ${Proj_SOURCES} ${Proj_RES} ${APP_ICON_RESOURCE_WINDOWS})
ELSE()
    add_executable(${PROJECT_NAME}${Proj_UIS} ${Proj_SOURCES} ${Proj_RES})
ENDIF()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets)

# Deploy app
include(deploy.cmake)
