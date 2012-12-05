#ifndef MAINWINDOWFILEMANAGER_H
#define MAINWINDOWFILEMANAGER_H

#include "ui_mainwindowfilemanager.h"
#include <QtGui/QMainWindow>
#include <QtGui/QFileSystemModel>
#include "addressbar.h"
#include "viewcontent.h"
#include "icons.h"

class MainWindowFileManager : public QMainWindow, public Ui::MainWindowFileManager
{
    Q_OBJECT

public:
    MainWindowFileManager(QWidget *parent = 0);
    ~MainWindowFileManager();

private slots:
    void onClickedDirectoryAddressBar(QFileInfo info);
    void onChangeDirCurrentView(QString dir);
    void onPreviousDir(bool activate);
    void onNextDir(bool activate);
    void on_actionDetailsView_triggered();
    void on_actionAddTab_triggered();
    void on_actionGoNextDir_triggered();
    void on_actionGoPreviousDir_triggered();
    void on_actionGoHome_triggered();
    void on_actionCloseTab_triggered();
    void on_actionUpDir_triggered();
    void on_actionTreeView_triggered();
    void on_m_tabWidget_tabCloseRequested(int index);
    void on_actionTerminal_triggered();
    void on_actionIconView_triggered();
    void on_actionListView_triggered();
    void on_m_tabWidget_currentChanged(int index);

    void on_actionViewHide_triggered();

private:
    void setPropertiesTerminal();
    void setViewHideFile();
    void setTheme();
    void createAddressBar();

    ThumbnailIconProvider *m_iconProvider;
    Icons *m_icons;
    AddressBar *m_addresBar;
};

#endif // MAINWINDOWFILEMANAGER_H
