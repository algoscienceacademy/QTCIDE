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

# Qt deployment and bundling
if(WIN32)
    # Find windeployqt with better path detection
    find_program(WINDEPLOYQT_EXECUTABLE 
        NAMES windeployqt windeployqt.exe
        HINTS 
            ${Qt6_DIR}/../../../bin
            ${Qt6_DIR}/../../..
            ${QT_QMAKE_EXECUTABLE}/..
            ${Qt6Core_DIR}/../../../bin
        PATHS
            "C:/Qt/6.*/msvc*/bin"
            "C:/Qt/6.*/mingw*/bin"
            "C:/Qt/*/bin"
            "C:/msys64/mingw64/bin"
        DOC "Path to windeployqt executable"
    )
    
    # Find common DLL paths for manual copying
    set(DLL_SEARCH_PATHS
        "C:/msys64/mingw64/bin"
        "C:/Qt/Tools/mingw*/bin"
        "${Qt6_DIR}/../../../bin"
        "${Qt6Core_DIR}/../../../bin"
        "C:/Qt/6.*/mingw*/bin"
    )
    
    # Function to find and copy DLL
    function(find_and_copy_dll dll_name target_dir)
        find_file(${dll_name}_PATH
            NAMES ${dll_name}
            PATHS ${DLL_SEARCH_PATHS}
            NO_DEFAULT_PATH
        )
        
        if(${dll_name}_PATH)
            message(STATUS "Found ${dll_name}: ${${dll_name}_PATH}")
            list(APPEND FOUND_DLLS ${${dll_name}_PATH})
            set(FOUND_DLLS ${FOUND_DLLS} PARENT_SCOPE)
        else()
            message(WARNING "Could not find ${dll_name} in search paths")
        endif()
    endfunction()
    
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        
        # Find required DLLs
        set(FOUND_DLLS)
        find_and_copy_dll("libfreetype-6.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libharfbuzz-0.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libpng16-16.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libmd4c.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libbz2-1.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libzstd.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libbrotlicommon.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libbrotlidec.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libglib-2.0-0.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libgraphite2.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libintl-8.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libiconv-2.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("libpcre2-16-0.dll" "${CMAKE_BINARY_DIR}/deploy")
        find_and_copy_dll("zlib1.dll" "${CMAKE_BINARY_DIR}/deploy")
        
        # Add custom target for deployment
        add_custom_target(deploy
            COMMAND ${CMAKE_COMMAND} -E echo "Cleaning deployment directory..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/deploy"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/deploy"
            COMMAND ${CMAKE_COMMAND} -E echo "Copying executable..."
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:QTCIDE>" "${CMAKE_BINARY_DIR}/deploy/"
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --verbose 2 --force --compiler-runtime --dir "${CMAKE_BINARY_DIR}/deploy" "${CMAKE_BINARY_DIR}/deploy/QTCIDE.exe"
            COMMAND ${CMAKE_COMMAND} -E echo "Copying additional DLLs..."
            COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "Deployment completed successfully!"
            DEPENDS QTCIDE copy_dlls_script
            COMMENT "Deploying Qt libraries and creating standalone package"
            VERBATIM
        )
        
        # Create a script to copy additional DLLs
        add_custom_target(copy_dlls_script
            COMMAND ${CMAKE_COMMAND} -E echo "# DLL Copy Script" > "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "set(DLL_SEARCH_PATHS \"${DLL_SEARCH_PATHS}\")" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "set(REQUIRED_DLLS" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libfreetype-6.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libharfbuzz-0.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libpng16-16.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libmd4c.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libbz2-1.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libzstd.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libbrotlicommon.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libbrotlidec.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libglib-2.0-0.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libgraphite2.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libintl-8.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libiconv-2.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    libpcre2-16-0.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    zlib1.dll" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo ")" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "foreach(dll IN LISTS REQUIRED_DLLS)" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    find_file(\${dll}_PATH" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        NAMES \${dll}" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        PATHS \${DLL_SEARCH_PATHS}" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        NO_DEFAULT_PATH" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    )" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    if(\${dll}_PATH)" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        message(STATUS \"Copying \${dll}: \${\${dll}_PATH}\")" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        file(COPY \${\${dll}_PATH} DESTINATION \"${CMAKE_BINARY_DIR}/deploy\")" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    else()" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "        message(WARNING \"Could not find \${dll}\")" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "    endif()" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMAND ${CMAKE_COMMAND} -E echo "endforeach()" >> "${CMAKE_BINARY_DIR}/copy_dlls.cmake"
            COMMENT "Creating DLL copy script"
            VERBATIM
        )
        
        # Add custom target for creating portable executable
        add_custom_target(package-portable
            COMMAND ${CMAKE_COMMAND} -E echo "Creating portable package..."
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/QTCIDE-Portable"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/deploy" "${CMAKE_BINARY_DIR}/QTCIDE-Portable"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_BINARY_DIR}/QTCIDE-Portable/README.md" || ${CMAKE_COMMAND} -E echo "README.md not found, skipping..."
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/LICENSE.txt" "${CMAKE_BINARY_DIR}/QTCIDE-Portable/LICENSE.txt" || ${CMAKE_COMMAND} -E echo "LICENSE.txt not found, skipping..."
            COMMAND ${CMAKE_COMMAND} -E echo "Portable package created at: ${CMAKE_BINARY_DIR}/QTCIDE-Portable"
            DEPENDS deploy
            COMMENT "Creating portable package"
            VERBATIM
        )
        
        # Add custom target for creating ZIP archive
        add_custom_target(package-zip
            COMMAND ${CMAKE_COMMAND} -E echo "Creating ZIP archive..."
            COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_BINARY_DIR}/QTCIDE-${PROJECT_VERSION}-Portable.zip" --format=zip -- .
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/QTCIDE-Portable"
            DEPENDS package-portable
            COMMENT "Creating ZIP archive of portable package"
            VERBATIM
        )
        
    else()
        message(WARNING "windeployqt not found. Deployment targets will not be available.")
        message(STATUS "Searched paths:")
        foreach(path IN LISTS DLL_SEARCH_PATHS)
            message(STATUS "  - ${path}")
        endforeach()
        message(STATUS "Please ensure Qt development tools are installed and in PATH")
        
        # Create dummy targets
        add_custom_target(deploy
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: windeployqt not found. Cannot deploy Qt libraries."
            COMMAND ${CMAKE_COMMAND} -E echo "Please install Qt development tools or add windeployqt to PATH"
            COMMENT "Deployment not available - windeployqt not found"
        )
        
        add_custom_target(package-portable
            COMMAND ${CMAKE_COMMAND} -E echo "ERROR: Cannot create portable package without windeployqt"
            COMMENT "Portable packaging not available"
        )
    endif()
    
    # Manual deployment fallback with DLL copying
    add_custom_target(deploy-manual
        COMMAND ${CMAKE_COMMAND} -E echo "Manual deployment - copying executable and searching for DLLs..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/deploy-manual"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/deploy-manual"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:QTCIDE>" "${CMAKE_BINARY_DIR}/deploy-manual/"
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "Manual deployment completed at: ${CMAKE_BINARY_DIR}/deploy-manual"
        COMMAND ${CMAKE_COMMAND} -E echo "Note: Some Qt DLLs may still be missing. Test the executable."
        DEPENDS QTCIDE copy_dlls_manual_script
        COMMENT "Manual deployment with DLL search"
        VERBATIM
    )
    
    # Create manual DLL copy script
    add_custom_target(copy_dlls_manual_script
        COMMAND ${CMAKE_COMMAND} -E echo "# Manual DLL Copy Script" > "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "set(DLL_SEARCH_PATHS \"${DLL_SEARCH_PATHS}\")" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "set(REQUIRED_DLLS libfreetype-6.dll libharfbuzz-0.dll libpng16-16.dll libmd4c.dll)" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "foreach(dll IN LISTS REQUIRED_DLLS)" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "    find_file(\${dll}_PATH NAMES \${dll} PATHS \${DLL_SEARCH_PATHS} NO_DEFAULT_PATH)" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "    if(\${dll}_PATH)" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "        message(STATUS \"Copying \${dll}\")" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "        file(COPY \${\${dll}_PATH} DESTINATION \"${CMAKE_BINARY_DIR}/deploy-manual\")" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "    endif()" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "endforeach()" >> "${CMAKE_BINARY_DIR}/copy_dlls_manual.cmake"
        COMMENT "Creating manual DLL copy script"
        VERBATIM
    )
endif()

# CPack configuration for MSI installer
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "QTCIDE")
set(CPACK_PACKAGE_VENDOR "QTCIDE Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QTCIDE - Professional Qt IDE")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "QTCIDE")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

# Windows-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "WIX;NSIS;ZIP")
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
    set(CPACK_WIX_PRODUCT_GUID "87654321-4321-4321-4321-210987654321")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "QTCIDE")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/installer/banner.bmp")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/installer/dialog.bmp")
    
    # NSIS specific settings
    set(CPACK_NSIS_DISPLAY_NAME "QTCIDE - Professional Qt IDE")
    set(CPACK_NSIS_PACKAGE_NAME "QTCIDE")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "QTCIDE.exe")
    set(CPACK_NSIS_HELP_LINK "https://github.com/algoscienceacademy/QTCIDE")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/algoscienceacademy/QTCIDE")
    set(CPACK_NSIS_CONTACT "support@qtcide.com")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\QTCIDE.lnk' '$INSTDIR\\\\QTCIDE.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\QTCIDE.lnk'"
    )
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "QTCIDE")
    set(CPACK_DMG_FORMAT "UDBZ")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "QTCIDE Team")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libqt6core6, libqt6gui6, libqt6widgets6")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
endif()

include(CPack)

# Custom target to build everything and create packages
add_custom_target(bundle-all
    DEPENDS QTCIDE
    COMMENT "Building and bundling complete application"
)

if(WIN32)
    if(WINDEPLOYQT_EXECUTABLE)
        add_dependencies(bundle-all deploy package-portable package-zip)
    else()
        add_dependencies(bundle-all deploy-manual)
    endif()
    
    # Remove automatic post-build deployment (causes issues)
    # Users can manually run: cmake --build build --target deploy
endif()

# Install rules
install(TARGETS QTCIDE
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

# Install Qt libraries on Windows
if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    install(CODE "
        execute_process(
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --verbose 2 --dir \${CMAKE_INSTALL_PREFIX}/bin \${CMAKE_INSTALL_PREFIX}/bin/QTCIDE.exe
        )
    ")
endif()

# Install additional files
install(FILES "${CMAKE_SOURCE_DIR}/README.md" DESTINATION . OPTIONAL)
install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.txt" DESTINATION . OPTIONAL)

# Create desktop entry on Linux
if(UNIX AND NOT APPLE)
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/qtcide.desktop.in"
        "${CMAKE_BINARY_DIR}/qtcide.desktop"
        @ONLY
    )
    install(FILES "${CMAKE_BINARY_DIR}/qtcide.desktop" DESTINATION share/applications)
    install(FILES "${CMAKE_SOURCE_DIR}/resources/icon.png" DESTINATION share/pixmaps RENAME qtcide.png)
endif()

# Print build information
message(STATUS "")
message(STATUS "=== QTCIDE Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Qt version: ${Qt6_VERSION}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(WIN32)
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "WinDeployQt: ${WINDEPLOYQT_EXECUTABLE}")
        message(STATUS "Packaging: WIX, NSIS, ZIP, Portable")
    else()
        message(STATUS "WinDeployQt: NOT FOUND")
        message(STATUS "Packaging: Manual deployment only")
    endif()
elseif(APPLE)
    message(STATUS "Packaging: DragNDrop, TGZ")
elseif(UNIX)
    message(STATUS "Packaging: DEB, RPM, TGZ")
endif()
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  cmake --build build                    - Build the application")
if(WIN32)
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "  cmake --build build --target deploy           - Deploy Qt libraries")
        message(STATUS "  cmake --build build --target package-portable - Create portable package")
        message(STATUS "  cmake --build build --target package-zip      - Create ZIP archive")
    else()
        message(STATUS "  cmake --build build --target deploy-manual    - Manual deployment")
    endif()
endif()
message(STATUS "  cmake --build build --target bundle-all       - Build and bundle everything")
message(STATUS "  cmake --build build --target package          - Create installer packages")
message(STATUS "")

if(WIN32 AND NOT WINDEPLOYQT_EXECUTABLE)
    message(STATUS "⚠️  IMPORTANT: windeployqt not found!")
    message(STATUS "   To enable full deployment, ensure Qt development tools are installed")
    message(STATUS "   and windeployqt.exe is in your PATH or Qt bin directory")
    message(STATUS "")
endif()
