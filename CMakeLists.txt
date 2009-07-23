project(kaveau)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

add_subdirectory( src )
