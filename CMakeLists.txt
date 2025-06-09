cmake_minimum_required(VERSION 3.16)
project(QTCIDE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt6_standard_project_setup()

set(SOURCES
    main.cpp
    MainWindow.cpp
    WelcomeScreen.cpp
    Terminal.cpp
    CodeEditor.cpp
    ProjectManager.cpp
    NewProjectDialog.cpp
    SettingsDialog.cpp
)

set(HEADERS
    MainWindow.h
    WelcomeScreen.h
    Terminal.h
    CodeEditor.h
    ProjectManager.h
    NewProjectDialog.h
    SettingsDialog.h
)

# Add MOC files for Q_OBJECT classes
qt6_add_executable(QTCIDE ${SOURCES})

target_link_libraries(QTCIDE 
    PRIVATE 
        Qt6::Core 
        Qt6::Widgets
)

# Enable automatic MOC, UIC, and RCC
set_target_properties(QTCIDE PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Platform-specific settings
if(WIN32)
    set_target_properties(QTCIDE PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(QTCIDE PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# Install rules
install(TARGETS QTCIDE
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
