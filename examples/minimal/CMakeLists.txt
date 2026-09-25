cmake_minimum_required(VERSION 3.16)
project(topqml-minimal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(minimal
    main.cpp
)

qt_add_qml_module(minimal
    URI MinimalExample
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(minimal PRIVATE Qt6::Quick)
