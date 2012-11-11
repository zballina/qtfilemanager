#-------------------------------------------------
#
# Project created by QtCreator 2012-10-08T22:20:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtFileManager
TEMPLATE = app

DEPENDPATH += . \
    src
INCLUDEPATH += . \
    src
OBJECTS_DIR = build
MOC_DIR = build


SOURCES += \
    src/tabbar.cpp \
    src/propertiesdlg.cpp \
    src/progressdlg.cpp \
    src/mymodelitem.cpp \
    src/mymodel.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/icondlg.cpp \
    src/customactions.cpp \
    src/bookmarks.cpp \
    src/actions.cpp \
    src/properties.cpp \
    src/propertiesdialog.cpp

HEADERS  += \
    src/tabbar.h \
    src/propertiesdlg.h \
    src/progressdlg.h \
    src/mymodelitem.h \
    src/mymodel.h \
    src/mainwindow.h \
    src/icondlg.h \
    src/customactions.h \
    src/bookmarkmodel.h \
    src/properties.h \
    src/config.h \
    src/propertiesdialog.h

CONFIG += release warn_off thread
RESOURCES += resources.qrc
QT+= network
LIBS += -lmagic -lqtermwidget

target.path = /usr/bin
desktop.files += qtfilemanager.desktop
desktop.path += /usr/share/applications
icon.files += images/qtfilemanager.png
icon.path += /usr/share/pixmaps

docs.path += /usr/share/doc/qtfilemanager
docs.files += README CHANGELOG COPYING

trans.path += /usr/share/qtfilemanager
trans.files += translations/qtfm_da.qm \
	       translations/qtfm_de.qm \
	       translations/qtfm_es.qm \
	       translations/qtfm_fr.qm \
               translations/qtfm_it.qm \
	       translations/qtfm_pl.qm \
               translations/qtfm_ru.qm \
               translations/qtfm_sr.qm \
               translations/qtfm_zh.qm \
               translations/qtfm_zh_TW.qm

INSTALLS += target desktop icon docs trans

FORMS += \
    src/propertiesdialog.ui
