set(EXE_NAME qtfilemanager)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)

set(QTFILEMANAGER_SOURCES
    actions.cpp
    bookmarks.cpp
    customactions.cpp
    icondlg.cpp
    mymodel.cpp
    mymodelitem.cpp
    progressdlg.cpp
    properties.cpp
    propertiesdialog.cpp
    propertiesdlg.cpp
    tabbar.cpp
    mainwindow.cpp
    main.cpp
)

set(QTFILEMANAGER_MOC_SOURCES
    bookmarkmodel.h
    customactions.h
    icondlg.h
    mainwindow.h
    mymodel.h
    progressdlg.h
    propertiesdialog.h
    propertiesdlg.h
    tabbar.h
)

set(QTFILEMANAGER_UI_SOURCES
    forms/propertiesdialog.ui
)

QT4_WRAP_UI(QTFILEMANAGER__UI ${QTFILEMANAGER_UI_SOURCES} )
QT4_WRAP_CPP(QTFILEMANAGER_MOC ${QTFILEMANAGER_MOC_SOURCES} )

include(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
)
add_executable(${EXE_NAME}
    ${QTFILEMANAGER_SOURCES}
    ${QTFILEMANAGER_UI}
    ${QTFILEMANAGER_MOC}
)

target_link_libraries(${EXE_NAME}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)
