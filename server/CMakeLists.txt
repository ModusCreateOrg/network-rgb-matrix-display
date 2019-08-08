cmake_minimum_required(VERSION 3.13)
#add_compile_definitions(BOOST_ASIO_ENABLE_OLD_SERVICES=true)

project(matrix-server)

set(CMAKE_CXX_STANDARD 14)

# external libraries
INCLUDE(FindPkgConfig)

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIR}
)

#add_compile_definitions(REMOVE_DEPRECATED_TRANSFORMERS true);

#Flip the image viertically
add_compile_definitions(__MATRIX_STRIP_BOTTOM_UP__ true)

set(MATRIX_SRC_DIR ${CMAKE_SOURCE_DIR}/rpi-rgb-led-matrix/)

# Currently using everything. Compilation is a little slower, we could work to remove items if we wish.
file(
  GLOB LED_MATRIX_SRC
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${MATRIX_SRC_DIR}/lib/*.cc
  ${MATRIX_SRC_DIR}/lib/*.c
  ${MATRIX_SRC_DIR}/lib/*.h
)

INCLUDE_DIRECTORIES(${MATRIX_SRC_DIR}/include ${MATRIX_SRC_DIR}/lib)

add_executable(${PROJECT_NAME} ${LED_MATRIX_SRC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})



