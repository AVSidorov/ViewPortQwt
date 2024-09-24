cmake_minimum_required(VERSION 3.25)

set(VCPKG_OVERLAY_PORTS ${CMAKE_CURRENT_LIST_DIR}/overlays)
message("VCPKG_OVERLAY_PORTS ${VCPKG_OVERLAY_PORTS}")

project(ViewPortQwt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/ViewPortQwt.cpp
        src/QwtRasterImage.cpp
)

find_package(Qt5
        COMPONENTS
        Widgets
        REQUIRED
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt::Widgets
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("use debug lib")
    set(subfolder debug)
else()
    set(subfolder "")
endif()

find_package(Qwt REQUIRED)
find_package(OpenCV REQUIRED)

message("Qwt_DIR ${Qwt_DIR}")
message("Qwt_INCLUDE_DIR ${Qwt_INCLUDE_DIR}")
message("OpenCV_DIR ${OpenCV_DIR}")
message( "OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS}" )
message( "OpenCV_LIBS ${OpenCV_LIBS}" )


target_link_libraries(${PROJECT_NAME} PRIVATE Qwt::qwt ${OpenCV_LIBS})

