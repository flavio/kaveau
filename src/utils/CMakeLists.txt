INCLUDE_DIRECTORIES(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}
                    ${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

set(kaveau_utils_SRC
    common.cpp
    settings.cpp
    processlistener.cpp
    backupthread.cpp
    backupmanager.cpp
    backupremoverthread.cpp
    backupdevice.cpp
)

KDE4_ADD_LIBRARY(kaveau_utils STATIC ${kaveau_utils_SRC})
