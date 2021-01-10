cmake_minimum_required(VERSION 3.17)
#SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake.files")
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake.files)

project(network-display-client)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-implicit-exception-spec-mismatch")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-implicit-exception-spec-mismatch")

# external libraries
INCLUDE(FindPkgConfig)

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIR}
)


set(INI_LIB_DIR ${CMAKE_SOURCE_DIR}/../lib/ini)

file(
  GLOB SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${INI_LIB_DIR}/*.c
)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${INI_LIB_DIR}/)

SET(DEBUG_USE_SDL_WINDOW=true)
#if (DEFINED ENV{DEBUG_USE_SDL_WINDOW})
if (1)
#    ADD_COMPILE_DEFINITIONS(__USE_SDL2_VIDEO__=true)
    # CMake Module path (additional cmake files via find_package)

    FIND_PACKAGE(SDL2 REQUIRED)
    INCLUDE_DIRECTORIES(
        ${SDL2_INCLUDE_DIR}
    )

    SET(ADDITIONAL_LIBRARIES  ${SDL2_LIBRARY} )

else()
    SET(ADDITIONAL_LIBRARIES "")
endif()

set(EXECUTABLE_NAME "draw-bitmap")

#pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
#    libavcodec
#    libavdevice
#    libavfilter
#    libavformat
#    libavresample
#    libavutil
#    libpostproc
#    libswresample
#    libswscale
#)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} PkgConfig::LIBAV)
MESSAGE("SDL2_LIBRARIES " ${ADDITIONAL_LIBRARIES})
#SET(ADDITIONAL_LIBRARIES -L/usr/local/lib ${SDL2_LIBRARIES})

add_executable(${PROJECT_NAME} examples/${EXECUTABLE_NAME}.cpp ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${SDL2_LIBRARY})


add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/examples/${EXECUTABLE_NAME}.ini
    ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}.ini
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/examples/image.bmp
    ${CMAKE_CURRENT_BINARY_DIR}/image.bmp
)



