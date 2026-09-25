cmake_minimum_required(VERSION 3.16)
project(topqml
    VERSION 0.1.0
    DESCRIPTION "Qt6/QML module: system monitoring collectors (btop-derived) with QML bindings"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Qml)
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(TOPQML_SOURCES
    src/collect/tools_qt.h
    src/collect/tools_qt.cpp
    src/collect/cpu_mem.h
    src/collect/cpu_mem.cpp
    src/collect/psi.h
    src/collect/psi.cpp
    src/collect/proc.h
    src/collect/proc.cpp
    src/collect/disk_net.h
    src/collect/disk_net.cpp
    src/collect/gpu_sensors.h
    src/collect/gpu_sensors.cpp
    src/collect/gpu_procs.h
    src/collect/gpu_procs.cpp
    src/backend/snapshots.h
    src/backend/TopConfig.h
    src/backend/TopConfig.cpp
    src/backend/TopEngine.h
    src/backend/TopEngine.cpp
    src/backend/CollectorWorker.h
    src/backend/CollectorWorker.cpp
    src/backend/Monitors.h
    src/backend/Monitors.cpp
    src/backend/DiskNetMonitors.h
    src/backend/DiskNetMonitors.cpp
    src/backend/GpuSensorsMonitors.h
    src/backend/GpuSensorsMonitors.cpp
    src/backend/ProcessModel.h
    src/backend/ProcessModel.cpp
    src/backend/ProcFilterProxyModel.h
    src/backend/ProcFilterProxyModel.cpp
)

qt_add_qml_module(topqml
    URI Top
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Top
    SOURCES
        ${TOPQML_SOURCES}
)

target_include_directories(topqml
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/backend>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/collect>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(topqml PUBLIC
    Qt6::Core Qt6::Qml
)

install(TARGETS topqml
    EXPORT topqmlTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#? C++ consumption: public headers (Monitors, ProcessModel, ProcFilterProxyModel,
#? TopConfig, snapshots, …) land flat in include/topqml/
install(DIRECTORY src/backend/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/topqml
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Top/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt6/qml/Top
)

install(EXPORT topqmlTargets
    FILE topqmlTargets.cmake
    NAMESPACE topqml::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topqml
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/topqmlConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/topqmlConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topqml
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/topqmlConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/topqmlConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/topqmlConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/topqml
)

include(CTest)
if(BUILD_TESTING)
    find_program(QML_RUNNER NAMES qml6 qml HINTS /usr/lib/qt6/bin)
    if(QML_RUNNER)
        add_test(NAME smoke COMMAND ${QML_RUNNER} ${CMAKE_CURRENT_SOURCE_DIR}/test/smoke.qml)
        set_tests_properties(smoke PROPERTIES
            ENVIRONMENT "QML_IMPORT_PATH=${CMAKE_CURRENT_BINARY_DIR};QT_QPA_PLATFORM=offscreen;QT_FORCE_STDERR_LOGGING=1"
            TIMEOUT 30
        )
    endif()
endif()
