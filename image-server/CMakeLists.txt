#
#cmake file for buiding rcar package
#cmake compability
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

#allow more human readalbe "if then else" construct
SET( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# cmake project name, default language is cxx
PROJECT(rcar CXX)

#project version
SET ($(PROJECT_NAME)_MAJOR_VERSION 0)
SET ($(PROJECT_NAME)_MINOR_VERSION 1)
SET ($(PROJECT_NAME)_PATCH_LEVEL 0)
SET (TARGET_NAME rcar-image-server)

#project option
IF (NOT CMAKE_BUILD_TYPE)
    SET (CMAKE_BUILD_TYPE Debug)
ENDIF()

#include directoris
INCLUDE_DIRECTORIES(
    "${PROJECT_SOURCE_DIR}/."
    "${PROJECT_SOURCE_DIR}/.."
    "${PROJECT_SOURCE_DIR}/../include/"
    "${PROJECT_SOURCE_DIR}/../3rdparty/include"
    )

# c++ definitions
ADD_DEFINITIONS("-Wall -g")
ADD_DEFINITIONS("-std=c++11")
ADD_DEFINITIONS("-Wno-unused-local-typedefs")
ADD_DEFINITIONS("-Wno-unused-variable")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS("-DDEBUG")
ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin/)
#project src
AUX_SOURCE_DIRECTORY(. SRC_LIST)
AUX_SOURCE_DIRECTORY(./jpeg JPEG_SRC_LIST)

# add executable files
LINK_DIRECTORIES("/usr/local/lib/"
    "${PROJECT_SOURCE_DIR}/../lib"
    "${PROJECT_SOURCE_DIR}/../3rdparty/lib"
    )
ADD_EXECUTABLE(${TARGET_NAME} 
    ${SRC_LIST} 
    ${JPEG_SRC_LIST} 
    )

TARGET_LINK_LIBRARIES(${TARGET_NAME} 
    dl
    event 
    json-linux-0.6.0
    mongoclient
    boost_regex
    boost_thread
    boost_filesystem
    boost_system
    ssl
    crypto
    curl
    epeg.a
    jpeg
    exif
    rcar.a
    zookeeper_mt.a
    pthread 
    )

# set binary director
SET (EXECUTABLE_BINARY_DIR Build)
SET (EXECUTABLE_OUTPUT_PATH
    ${PROJECT_SOURCE_DIR}/../
    CACHE PATH
    "build and bin directory"
    )

MESSAGE(STATUS "Generating Makefile for linux...")
SET (CMAKE_SYSTEM_NAME Linux) 
# set compiler, the g++ will be used both in linux or windows
SET (CMAKE_C_COMPILER gcc)
SET (CMAKE_CXX_COMPILER g++)
#set default iinstall prefix project root directory
IF (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    SET (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")
ENDIF()

#display status message for important variables
MESSAGE(STATUS "-------------------------------------------------")
MESSAGE(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
MESSAGE(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "EXECUTABLE_OUTPUT_PATH = ${EXECUTABLE_OUTPUT_PATH}")
MESSAGE(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
MESSAGE(STATUS "PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
MESSAGE(STATUS "------------------------------------------------")
MESSAGE(STATUS)
