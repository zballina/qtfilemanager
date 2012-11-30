#ifndef MAINWINDOWFILEMANAGER_H
#define MAINWINDOWFILEMANAGER_H

#include "ui_mainwindowfilemanager.h"
#include <QtGui/QMainWindow>
#include <QtGui/QFileSystemModel>
#include "viewcontent.h"
#include "icons.h"

class MainWindowFileManager : public QMainWindow, public Ui::MainWindowFileManager
{
    Q_OBJECT

public:
    MainWindowFileManager(QWidget *parent = 0);
    ~MainWindowFileManager();

private slots:
    void onChangeDirCurrentView(QString dir);
    void onDirPrevious(bool activate);
    void onDirFollow(bool activate);
    void on_actionDetailsView_triggered();
    void on_actionAddTab_triggered();
    void on_actionGoHome_triggered();
    void on_actionPreviosDir_triggered();
    void on_actionCloseTab_triggered();
    void on_actionUpDir_triggered();
    void on_actionTreeView_triggered();
    void on_m_tabWidget_tabCloseRequested(int index);
    void on_actionTerminal_triggered();
    void on_actionIconView_triggered();
    void on_actionListView_triggered();

private:
    void setPropertiesTerminal();

    ThumbnailIconProvider *m_iconProvider;
    Icons *m_icons;
};

#endif // MAINWINDOWFILEMANAGER_H
