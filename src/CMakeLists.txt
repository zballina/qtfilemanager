set(EXE_NAME qtfilemanager)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)

include(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QTERMWIDGET_INCLUDE_DIR}
)

set(QTFILEMANAGER_HEADERS
)

set(QTFILEMANAGER_SOURCES
    main.cpp
    mainwindowfilemanager.cpp
    icondlg.cpp
    propertiesdialog.cpp
    properties.cpp
    utils.cpp
    icons.cpp
    viewcontent.cpp
    thumbnailiconprovider.cpp
    addressbar.cpp
    addresspart.cpp
)

set(QTFILEMANAGER_MOC_SOURCES
    mainwindowfilemanager.h
    icondlg.h
    propertiesdialog.h
    viewcontent.h
    addressbar.h
    addresspart.h
)

set(QTFILEMANAGER_UI_SOURCES
    forms/propertiesdialog.ui
    forms/mainwindowfilemanager.ui
    forms/viewcontent.ui
    forms/addressbar.ui
    forms/addresspart.ui
)

set(QTFILEMANAGER_RCC_SRC
    resources/resources.qrc
)

QT4_WRAP_UI(QTFILEMANAGER_UI ${QTFILEMANAGER_UI_SOURCES} )
QT4_WRAP_CPP(QTFILEMANAGER_MOC ${QTFILEMANAGER_MOC_SOURCES} )
QT4_ADD_RESOURCES( QTFILEMANAGER_RCC ${QTFILEMANAGER_RCC_SRC} )

# Translations **********************************
include(RazorTranslate)
razor_translate_ts(QM_FILES
    SOURCES
        ${QTFILEMANAGER_SOURCES}
        ${QTFILEMANAGER_SOURCES}
        ${QTFILEMANAGER_UI_SOURCES}
)

razor_translate_desktop(DESKTOP_FILES
    SOURCES
        qtfilemanager.desktop.in
)
#************************************************

add_executable(${EXE_NAME}
    ${QTFILEMANAGER_SOURCES}
    ${QTFILEMANAGER_UI}
    ${QTFILEMANAGER_MOC}
    ${QTFILEMANAGER_RCC}
    ${QM_FILES}
    ${DESKTOP_FILES}
)

target_link_libraries(${EXE_NAME}
    magic
    qtxdg
    ${QTERMWIDGET_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin)
