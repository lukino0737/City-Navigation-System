cmake_minimum_required(VERSION 3.16)
project(City-Navigation-System LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Quick
)
qt_standard_project_setup()

include_directories(src)

set(PROJECT_SOURCES
    src/main.cpp
    src/api/NavigationAPI.h
    src/core/DataModel/Graph.h
    src/core/DataModel/Graph.cpp
    src/core/modules/DataGenerator.h
    src/core/modules/DataGenerator.cpp
    src/core/modules/PathFinder.h
    src/core/modules/PathFinder.cpp
    src/core/modules/SpatialSearch.h
    src/core/modules/SpatialSearch.cpp
    src/core/modules/TrafficSimulator.h
    src/core/modules/TrafficSimulator.cpp
    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
    src/gui/MapView.h
    src/gui/MapView.cpp
)

if(QT_VERSION VERSION_LESS 5.15)
    qt5_add_resources(PROJECT_SOURCES qml.qrc)
elseif(QT_VERSION VERSION_LESS 6.2)
    qt_add_resources(PROJECT_SOURCES qml.qrc)
endif()

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

if(QT_VERSION VERSION_GREATER_EQUAL 6.2)
    qt_add_qml_module(${PROJECT_NAME}
        URI city-navigation-system
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml/"
        QML_FILES
            main.qml
    )
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Quick
)


if(Qt6_FOUND)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()
