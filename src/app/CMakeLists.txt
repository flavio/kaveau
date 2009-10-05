include_directories( ../utils
                    ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}
                    ${CMAKE_SOURCE_DIR}/utils ${CMAKE_BINARY_DIR}/utils
                    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(kaveau_app_SRC
    main.cpp
    mainwindow.cpp
    excludeditemsdialog.cpp
    addbackupwizard.cpp
    addbackupwizardpage1.cpp
    addbackupwizardpage2.cpp
    addbackupwizardpage3.cpp
    devicewidget.cpp
    logdialog.cpp
    changediskdialog.cpp
)

kde4_add_ui_files(
  kaveau_app_SRC
  mainwidgetbase.ui
  excludeditemsview.ui
  addbackupwizardpage1view.ui
  addbackupwizardpage2view.ui
  addbackupwizardpage3view.ui
  logview.ui
  devicewidgetview.ui
  changediskdialog.ui
)

kde4_add_executable(kaveau ${kaveau_app_SRC})

target_link_libraries(kaveau  kaveau_utils ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})


install(TARGETS kaveau DESTINATION ${BIN_INSTALL_DIR})
install(FILES kaveauui.rc
        DESTINATION  ${DATA_INSTALL_DIR}/kaveau)

install(FILES kaveau.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kaveau/ )
