cmake_minimum_required(VERSION 3.15)

project(itkImageCompare)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(ITK REQUIRED)
find_package(TCLAP REQUIRED)

include(${ITK_USE_FILE})

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.cxx)
target_include_directories(${PROJECT_NAME} PRIVATE ${TCLAP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ITK_LIBRARIES})
