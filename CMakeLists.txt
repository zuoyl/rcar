CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if(64_BIT_PLATFORM) 
     message(STATUS "using 64bits")  
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")  
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")  
else()  
endif()  

PROJECT(rcar CXX)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(apl-server)
#ADD_SUBDIRECTORY(msg-server)
#ADD_SUBDIRECTORY(image-server)

