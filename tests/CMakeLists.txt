##### Probably don't want to edit below this line ##### 

SET( QT_USE_QTTEST TRUE )

# Use it
INCLUDE( ${QT_USE_FILE} )

INCLUDE(AddFileDependencies)

# Include the library include directories, and the current build directory (moc)
INCLUDE_DIRECTORIES(
  ../src
  ../src/utils
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET( UNIT_TESTS
  testcommon
)

# Build the tests
FOREACH(test ${UNIT_TESTS})
  MESSAGE(STATUS "Building ${test}")
  QT4_WRAP_CPP(MOC_SOURCE ${test}.cpp)
  ADD_EXECUTABLE(
    ${test}
    ${test}.cpp
  )

  ADD_FILE_DEPENDENCIES(${test}.cpp ${MOC_SOURCE})
  TARGET_LINK_LIBRARIES(
          ${test}
          ${QT_LIBRARIES}
          ${KDE4_KDEUI_LIBS}
          ${KDE4_KIO_LIBS}
          ${KDE4_SOLID_LIBS}
          kaveau_utils
  )
  ADD_TEST( ${test} ${test} )
ENDFOREACH()
