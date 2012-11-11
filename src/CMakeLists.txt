set(EXE_NAME qtfilemanager)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)

set(QTFILEMANAGER_SOURCES
    main.cpp
    mainwindow.cpp
    customactions.cpp
    icondlg.cpp
    progressdlg.cpp
    propertiesdialog.cpp
    propertiesdlg.cpp
    tabbar.cpp
    properties.cpp
    mymodel.cpp
    mymodelitem.cpp
)

set(QTFILEMANAGER_MOC_SOURCES
    mainwindow.h
    bookmarkmodel.h
    customactions.h
    icondlg.h
    progressdlg.h
    propertiesdialog.h
    propertiesdlg.h
    tabbar.h
    mymodel.h
)

set(QTFILEMANAGER_UI_SOURCES
    forms/propertiesdialog.ui
)

set(QTFILEMANAGER_RCC_SRC
    ../resources.qrc
)

QT4_WRAP_UI(QTFILEMANAGER_UI ${QTFILEMANAGER_UI_SOURCES} )
QT4_WRAP_CPP(QTFILEMANAGER_MOC ${QTFILEMANAGER_MOC_SOURCES} )
QT4_ADD_RESOURCES( QTFILEMANAGER_RCC ${QTFILEMANAGER_RCC_SRC} )

include(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QTERMWIDGET_INCLUDE_DIR}
)
add_executable(${EXE_NAME}
    ${QTFILEMANAGER_SOURCES}
    ${QTFILEMANAGER_UI}
    ${QTFILEMANAGER_MOC}
    ${QTFILEMANAGER_RCC}
)

target_link_libraries(${EXE_NAME}
    magic
    ${QTERMWIDGET_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)
