cmake_minimum_required(VERSION 2.6) 
 
set(PROJECT mtfind) 
 
project(${PROJECT}) 

include_directories(include)#;${Boost_INCLUDE_DIRS})
 
set(HEADERS 
	include/search_core.h)
 
set(SOURCES 
    	src/search_core.cpp
	src/main.cpp) 

set(CMAKE_CXX_FLAGS "-std=c++11")

#pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#boost
#set(Boost_ADDITIONAL_VERSIONS "1.65" "1.65.0")
#find_package(Boost 1.65 COMPONENTS filesystem unit_test_framework REQUIRED)
#message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
#message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
#message(status "** Boost Libraries: ${Boost_LIBRARIES}")
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
#LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_executable (${PROJECT} ${HEADERS} ${SOURCES})

enable_testing()

add_test(NAME ares_test COMMAND ${PROJECT} input.txt "?ad")

target_link_libraries (${PROJECT} ${CMAKE_THREAD_LIBS_INIT})# ${Boost_LIBRARIES})

install(TARGETS ${PROJECT} DESTINATION bin)

# vim:ts=4:sts=4:sw=4:et:
