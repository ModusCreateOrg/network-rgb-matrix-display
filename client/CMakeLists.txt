cmake_minimum_required(VERSION 3.13)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake.files")

project(network-display-client)

set(CMAKE_CXX_STANDARD 14)

# external libraries
INCLUDE(FindPkgConfig)

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIR}
)


# Currently using everything. Compilation is a little slower, we could work to remove items if we wish.
file(
  GLOB SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)



# Direct linux controls (non-SDL2!)
if (DEFINED ENV{DEBUG_USE_SDL_WINDOW})
    ADD_COMPILE_DEFINITIONS(__USE_SDL2_VIDEO__=true)
    # CMake Module path (additional cmake files via find_package)

    FIND_PACKAGE(SDL2 REQUIRED)
#    FIND_PACKAGE(SDL2_image REQUIRED)

    INCLUDE_DIRECTORIES(
        ${SDL2_INCLUDE_DIR}
        ${SDL2_IMAGE_INCLUDE_DIR}
    )

    SET(ADDITIONAL_LIBRARIES -L/usr/local/lib ${SDL2_LIBRARIES})

else()
    SET(ADDITIONAL_LIBRARIES "")
endif()





add_executable(${PROJECT_NAME} examples/simple.cpp ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${ADDITIONAL_LIBRARIES})



