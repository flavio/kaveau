include_directories( ../utils
  ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}
  ${CMAKE_SOURCE_DIR}/utils ${CMAKE_BINARY_DIR}/utils
  ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

# KaveauDaemon
# -----------------------------

set(kaveaudaemon_SRCS
  processlistener.cpp
  backupmanager.cpp
  backupdevice.cpp
  kaveaudaemon.cpp
  main.cpp)

# Server configuration
#kde4_add_kcfg_files(kaveaudaemon_SRCS nepomukserversettings.kcfgc)

# qt4_add_dbus_adaptor(kaveaudaemon_SRCS
#   ../interfaces/org.kde.NepomukServer.xml
#   nepomukserver.h
#   Nepomuk::Server)
# qt4_add_dbus_adaptor(kaveaudaemon_SRCS
#   ../interfaces/org.kde.nepomuk.ServiceManager.xml
#   servicemanager.h
#   Nepomuk::ServiceManager)
# 
# qt4_add_dbus_interface(kaveaudaemon_SRCS
#   ../interfaces/org.kde.nepomuk.ServiceControl.xml
#   servicecontrolinterface)

kde4_add_kdeinit_executable(kaveaudaemon ${kaveaudaemon_SRCS})

target_link_libraries(kdeinit_kaveaudaemon
  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} kaveau_utils
  )

target_link_libraries(kaveaudaemon
  kdeinit_kaveaudaemon
)

install(
  FILES kaveaudaemon.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
  FILES kaveaudaemon.desktop
  DESTINATION ${AUTOSTART_INSTALL_DIR})

install(
  TARGETS kdeinit_kaveaudaemon
  DESTINATION ${LIB_INSTALL_DIR})

install(
  TARGETS kaveaudaemon
  ${INSTALL_TARGETS_DEFAULT_ARGS})
# -----------------------------
