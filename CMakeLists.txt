cmake_minimum_required(VERSION 3.10)
project(SystemInfo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files for the main application
set(SOURCES
    src/main.cpp
    src/main_window.cpp
    src/cpu_info.cpp
    src/cpu_info_widget.cpp
    src/disk_window.cpp
    src/network_window.cpp
    src/service_window.cpp
    src/theme_manager.cpp
    src/usb_window.cpp
    src/disk_info.cpp
    src/network_info.cpp
    src/service_info.cpp
    src/memory_window.cpp
)

# Define header files with Q_OBJECT macro
set(MOC_HEADERS
    include/main_window.h
    include/cpu_info_widget.h
    include/disk_window.h
    include/network_window.h
    include/service_window.h
    include/theme_manager.h
    include/usb_window.h
    include/memory_window.h
)

# Other headers without Q_OBJECT
set(HEADERS
    include/cpu_info.h
    include/disk_info.h
    include/network_info.h
    include/service_info.h
)

# Add resources
set(RESOURCES
    resources.qrc
)

# Try to find Qt5
option(USE_QT_GUI "Build with Qt GUI" ON)
if(USE_QT_GUI)
    # Setup Qt
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    # Find Qt5 Widgets (required)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)

    # Find Qt5 Charts (optional)
    find_package(Qt5 COMPONENTS Charts QUIET)

    if(Qt5_FOUND)
        message(STATUS "Qt5 found, building GUI version")
        add_definitions(-DUSE_QT_GUI)

        # Manually run MOC on header files with Q_OBJECT
        qt5_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

        # Create the main executable
        add_executable(QTSystemStats ${SOURCES} ${MOC_SOURCES} ${RESOURCES})

        # Set include directories for the main executable
        target_include_directories(QTSystemStats PRIVATE include)

        # Link Qt libraries
        target_link_libraries(QTSystemStats PRIVATE Qt5::Widgets)

        # Add Charts if available
        if(Qt5Charts_FOUND)
            message(STATUS "Qt5Charts found, enabling charts functionality")
            add_definitions(-DUSE_QT_CHARTS)
            target_link_libraries(QTSystemStats PRIVATE Qt5::Charts)
        else()
            message(STATUS "Qt5Charts not found, disabling charts functionality")
        endif()
    else()
        message(STATUS "Qt5 not found, building console version only")
        set(USE_QT_GUI OFF)

        # Create the main executable (console version)
        add_executable(QTSystemStats ${SOURCES})
        target_include_directories(QTSystemStats PRIVATE include)
    endif()
else()
    # Create the main executable (console version)
    add_executable(QTSystemStats ${SOURCES})
    target_include_directories(QTSystemStats PRIVATE include)
endif()