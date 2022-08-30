#
# Deployment - Automatically Detect and Copy Dependencies to Deploy folder when building release (for windows)
#
IF(CMAKE_BUILD_TYPE MATCHES Release)
    # Windows deployment
    if (WIN32)
        # Copy executable to deploy folder
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe" "${CMAKE_BINARY_DIR}/../Deploy/Windows/${PROJECT_NAME}.exe" COMMENT "Copying to output directory")

        # Copy additional windows libs
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/libs/Windows/libeay32.dll" "${CMAKE_BINARY_DIR}/../Deploy/Windows/libeay32.dll" COMMENT "Copying required DLLs to output directory")
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/libs/Windows/ssleay32.dll" "${CMAKE_BINARY_DIR}/../Deploy/Windows/ssleay32.dll")

        # Copy required DLLs to deploy folder
        get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
        set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${QT_WINDEPLOYQT_EXECUTABLE} "${CMAKE_BINARY_DIR}/../Deploy/Windows/${PROJECT_NAME}.exe")
    endif(WIN32)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)
