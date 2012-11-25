/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012 Wittfella
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* Contact e-mail: wittfella@qtfm.org
*
****************************************************************************/

#include <QtGui>
#include <sys/vfs.h>
#include <fcntl.h>

#include "mainwindowfilemanager.h"
#include "mymodel.h"
#include "customactions.h"
#include "actions.cpp"
#include "bookmarks.cpp"
#include "progressdlg.h"
#include "properties.h"

MainWindowFileManager::MainWindowFileManager(QWidget *parent)
    : QMainWindow(parent), Ui::MainWindowFileManager()
{
    setupUi(this);
    Properties::Instance()->loadSettings();
    initializeTerminal();
    isDaemon = false;
    startPath = QDir::currentPath();
    QStringList args = QApplication::arguments();

    if(args.count() > 1)
    {
        if(args.at(1) == "-d")
        {
            isDaemon = true;
            Properties::Instance()->daemon = isDaemon;
        }
        else
        {
            startPath = args.at(1);
        }

        #if QT_VERSION >= 0x040800
        if(QUrl(startPath).isLocalFile())
            startPath = QUrl(args.at(1)).toLocalFile();
        #endif
    }
    else
    {
        isDaemon = Properties::Instance()->daemon;
        startPath = Properties::Instance()->startPath;
    }

    initializeTheme();

    modelList = new myModel(Properties::Instance()->realMimeTypes);

    tabs = new tabBar(modelList->folderIcons);

    horizontalLayout->addWidget(tabs);

    modelTree = new mainTreeFilterProxyModel();
    modelTree->setSourceModel(modelList);
    modelTree->setSortCaseSensitivity(Qt::CaseInsensitive);

    treeViewFolders->setHeaderHidden(true);
    treeViewFolders->setUniformRowHeights(true);
    treeViewFolders->setModel(modelTree);
    treeViewFolders->hideColumn(1);
    treeViewFolders->hideColumn(2);
    treeViewFolders->hideColumn(3);
    treeViewFolders->hideColumn(4);

    modelView = new viewsSortProxyModel();
    modelView->setSourceModel(modelList);
    modelView->setSortCaseSensitivity(Qt::CaseInsensitive);

    listViewFileSystem->setWrapping(true);
    listViewFileSystem->setModel(modelView);
    listSelectionModel = listViewFileSystem->selectionModel();

    treeViewFileSystem->setRootIsDecorated(false);
    treeViewFileSystem->setItemsExpandable(false);
    treeViewFileSystem->setUniformRowHeights(true);
    treeViewFileSystem->setModel(modelView);
    treeViewFileSystem->setSelectionModel(listSelectionModel);

    pathEdit = new QComboBox();
    pathEdit->setEditable(true);
    pathEdit->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    treeSelectionModel = treeViewFileSystem->selectionModel();

    connect(treeSelectionModel, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(treeSelectionChanged(QModelIndex, QModelIndex)));
    treeViewFolders->setCurrentIndex(modelTree->mapFromSource(modelList->index(startPath)));
    treeViewFolders->scrollTo(treeViewFolders->currentIndex());

    createActions();
    createToolBars();
    createMenus();

    restoreState(Properties::Instance()->windowState, 1);
    resize(Properties::Instance()->windowSize);

    setWindowIcon(QIcon(":/images/qtfm.png"));


    modelBookmarks = new BookmarkModel(modelList->folderIcons);

    Properties::Instance()->settings.beginGroup("bookmarks");
    foreach(QString key, Properties::Instance()->settings.childKeys())
    {
        QStringList temp(Properties::Instance()->settings.value(key).toStringList());
        modelBookmarks->addBookmark(temp.at(0), temp.at(1), temp.at(2), temp.last());
    }
    Properties::Instance()->settings.endGroup();

    autoBookmarkMounts();
    listViewPlaces->setModel(modelBookmarks);
    listViewPlaces->setResizeMode(QListView::Adjust);
    listViewPlaces->setFlow(QListView::TopToBottom);
    listViewPlaces->setIconSize(QSize(24, 24));

    wrapBookmarksAct->setChecked(Properties::Instance()->wrapBookmarks);
    listViewPlaces->setWrapping(wrapBookmarksAct->isChecked());

    lockLayoutAct->setChecked(Properties::Instance()->lockLayout);
    toggleLockLayout();

    zoom = Properties::Instance()->zoom;
    zoomTree = Properties::Instance()->zoomTree;
    zoomList = Properties::Instance()->zoomList;
    zoomDetail = Properties::Instance()->zoomDetail;

    treeViewFileSystem->setIconSize(QSize(zoomDetail, zoomDetail));
    treeViewFolders->setIconSize(QSize(zoomTree, zoomTree));

    thumbsAct->setChecked(Properties::Instance()->showThumbs);

    detailAct->setChecked(Properties::Instance()->viewMode);
    iconAct->setChecked(Properties::Instance()->iconMode);
    toggleDetails();

    hiddenAct->setChecked(Properties::Instance()->hiddenMode);
    toggleHidden();

    treeViewFolders->header()->restoreState(Properties::Instance()->header);
    treeViewFolders->setSortingEnabled(1);

    if(isDaemon)
        startDaemon();
    else
        show();

    QTimer::singleShot(0, this, SLOT(lateStart()));
}

void MainWindowFileManager::initializeTheme()
{
    QString temp = Properties::Instance()->forceTheme;
    if(temp.isNull())
    {
        //get theme from system (works for gnome/kde)
        temp = QIcon::themeName();

        //Qt doesn't detect the theme very well for non-DE systems,
        //so try reading the '~/.gtkrc-2.0' or '~/.config/gtk-3.0/settings.ini'

        if(temp == "hicolor")
        {
            //check for gtk-2.0 settings
            if(QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0"))
            {
                QSettings gtkFile(QDir::homePath() + "/.gtkrc-2.0",QSettings::IniFormat,this);
                temp = gtkFile.value("gtk-icon-theme-name").toString().remove("\"");
            }
            else
            {
                //try gtk-3.0
                QSettings gtkFile(QDir::homePath() +
                                  "/.config/gtk-3.0/settings.ini", QSettings::IniFormat, this);
                temp = gtkFile.value("gtk-fallback-icon-theme").toString().remove("\"");
            }

            //fallback
            if(temp.isNull())
            {
                if(QFile::exists("/usr/share/icons/gnome"))
                    temp = "gnome";
                else if(QFile::exists("/usr/share/icons/oxygen"))
                    temp = "oxygen";
                else
                    temp = "hicolor";

                Properties::Instance()->forceTheme = temp;
                Properties::Instance()->saveSettings();
            }
        }
    }

    QIcon::setThemeName(temp);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::lateStart()
{
    labelStatus->setText(getDriveInfo(curIndex.filePath()));

    listViewPlaces->setDragDropMode(QAbstractItemView::DragDrop);
    listViewPlaces->setDropIndicatorShown(true);
    listViewPlaces->setDefaultDropAction(Qt::MoveAction);
    listViewPlaces->setSelectionMode(QAbstractItemView::ExtendedSelection);

    treeViewFolders->setDragDropMode(QAbstractItemView::DragDrop);
    treeViewFolders->setDefaultDropAction(Qt::MoveAction);
    treeViewFolders->setDropIndicatorShown(true);
    treeViewFolders->setEditTriggers(
                QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    treeViewFileSystem->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeViewFileSystem->setDragDropMode(QAbstractItemView::DragDrop);
    treeViewFileSystem->setDefaultDropAction(Qt::MoveAction);
    treeViewFileSystem->setDropIndicatorShown(true);
    treeViewFileSystem->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    listViewFileSystem->setResizeMode(QListView::Adjust);
    listViewFileSystem->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listViewFileSystem->setSelectionRectVisible(true);
    listViewFileSystem->setFocus();

    listViewFileSystem->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    connect(listViewFileSystem, SIGNAL(activated(QModelIndex)),
            this, SLOT(listDoubleClicked(QModelIndex)));

    if(Properties::Instance()->singleClick)
    {
        connect(listViewFileSystem, SIGNAL(clicked(QModelIndex)),
                this, SLOT(listItemClicked(QModelIndex)));
        connect(treeViewFileSystem, SIGNAL(clicked(QModelIndex)),
                this, SLOT(listItemClicked(QModelIndex)));
    }
    else
    {
        connect(listViewFileSystem, SIGNAL(clicked(QModelIndex)),
                this, SLOT(listDoubleClicked(QModelIndex)));
        connect(treeViewFileSystem, SIGNAL(clicked(QModelIndex)),
                this, SLOT(listDoubleClicked(QModelIndex)));
    }

    customActions = new QMultiHash<QString,QAction*>;
    customMapper = new QSignalMapper();
    connect(customMapper, SIGNAL(mapped(QString)), this, SLOT(actionMapper(QString)));

    int fd = open("/proc/self/mounts", O_RDONLY, 0);
    notify = new QSocketNotifier(fd, QSocketNotifier::Write);
    connect(notify, SIGNAL(activated(int)), this, SLOT(mountWatcherTriggered()), Qt::QueuedConnection);

    progress = 0;
    clipboardChanged();

    customComplete = new myCompleter;
    customComplete->setModel(modelTree);
    customComplete->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    customComplete->setMaxVisibleItems(10);
    pathEdit->setCompleter(customComplete);

    tabs->setDrawBase(0);
    tabs->setExpanding(0);
    tabsOnTopAct->setChecked(Properties::Instance()->tabsOnTop);
    tabsOnTop();

    connect(pathEdit, SIGNAL(activated(QString)),
            this, SLOT(pathEditChanged(QString)));
    connect(customComplete, SIGNAL(activated(QString)),
            this, SLOT(pathEditChanged(QString)));
    connect(pathEdit->lineEdit(), SIGNAL(cursorPositionChanged(int, int)),
            this, SLOT(addressChanged(int, int)));

    connect(listViewPlaces, SIGNAL(activated(QModelIndex)),
            this, SLOT(bookmarkClicked(QModelIndex)));
    connect(listViewPlaces, SIGNAL(clicked(QModelIndex)),
            this, SLOT(bookmarkClicked(QModelIndex)));
    connect(listViewPlaces, SIGNAL(pressed(QModelIndex)),
            this, SLOT(bookmarkPressed(QModelIndex)));

    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)),
            this, SLOT(clipboardChanged()));
    connect(treeViewFileSystem, SIGNAL(activated(QModelIndex)),
            this, SLOT(listDoubleClicked(QModelIndex)));
    connect(listSelectionModel, SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),
            this, SLOT(listSelectionChanged(const QItemSelection, const QItemSelection)));

    connect(this, SIGNAL(copyProgressFinished(int,QStringList)),
            this, SLOT(progressFinished(int,QStringList)));

    connect(modelBookmarks, SIGNAL(bookmarkPaste(const QMimeData *, QString, QStringList)),
            this, SLOT(pasteLauncher(const QMimeData *, QString, QStringList)));
    connect(modelList, SIGNAL(dragDropPaste(const QMimeData *, QString, QStringList)),
            this, SLOT(pasteLauncher(const QMimeData *, QString, QStringList)));

    connect(tabs, SIGNAL(currentChanged(int)),
            this, SLOT(tabChanged(int)));
    connect(tabs, SIGNAL(dragDropTab(const QMimeData *, QString, QStringList)),
            this, SLOT(pasteLauncher(const QMimeData *, QString, QStringList)));
    connect(listViewFileSystem,SIGNAL(pressed(QModelIndex)),
            this, SLOT(listItemPressed(QModelIndex)));
    connect(treeViewFileSystem,SIGNAL(pressed(QModelIndex)),
            this, SLOT(listItemPressed(QModelIndex)));

    connect(modelList, SIGNAL(thumbUpdate(QModelIndex)),
            this, SLOT(thumbUpdate(QModelIndex)));

    qApp->setKeyboardInputInterval(1000);

    connect(&daemon, SIGNAL(newConnection()), this, SLOT(newConnection()));

    QTimer::singleShot(100, this, SLOT(readCustomActions()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::closeEvent(QCloseEvent *event)
{
    writeSettings();

    if(isDaemon)
    {
        this->setVisible(0);
        startDaemon();
        customComplete->setModel(0);
        modelList->refresh();           //clear model, reduce memory
        tabs->setCurrentIndex(0);

        treeViewFolders->setCurrentIndex(modelTree->mapFromSource(modelList->index(startPath)));
        treeViewFolders->scrollTo(treeViewFolders->currentIndex());
        customComplete->setModel(modelTree);

        event->ignore();
    }
    else
    {
        modelList->cacheInfo();
        event->accept();
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::exitAction()
{
    isDaemon = 0;
    close();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::treeSelectionChanged(QModelIndex current,QModelIndex previous)
{
    QFileInfo name = modelList->fileInfo(modelTree->mapToSource(current));
    if(!name.exists())
        return;

    curIndex = name;
    setWindowTitle(curIndex.fileName() + " - QtFileManager");

    if(treeViewFolders->hasFocus() && QApplication::mouseButtons() == Qt::MidButton)
    {
        listItemPressed(modelView->mapFromSource(modelList->index(name.filePath())));
        tabs->setCurrentIndex(tabs->count() - 1);
        if(currentView == 2)
            treeViewFileSystem->setFocus(Qt::TabFocusReason);
        else
            listViewFileSystem->setFocus(Qt::TabFocusReason);
    }

    if(curIndex.filePath() != pathEdit->itemText(0))
    {
        if(tabs->count())
            tabs->addHistory(curIndex.filePath());
        pathEdit->insertItem(0, curIndex.filePath());
        pathEdit->setCurrentIndex(0);
    }

    if(!listViewPlaces->hasFocus())
        listViewPlaces->clearSelection();

    if(modelList->setRootPath(name.filePath()))
        modelView->invalidate();

    //////
    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(name.filePath()));

    if(currentView == 2)
        treeViewFileSystem->setRootIndex(baseIndex);
    else
        listViewFileSystem->setRootIndex(baseIndex);

    if(tabs->count())
    {
        tabs->setTabText(tabs->currentIndex(), curIndex.fileName());
        tabs->setTabData(tabs->currentIndex(), curIndex.filePath());
        tabs->setIcon(tabs->currentIndex());
    }

    if(backIndex.isValid())
    {
        listSelectionModel->setCurrentIndex(modelView->mapFromSource(backIndex),
                                            QItemSelectionModel::ClearAndSelect);
        if(currentView == 2)
            treeViewFileSystem->scrollTo(modelView->mapFromSource(backIndex));
        else
            listViewFileSystem->scrollTo(modelView->mapFromSource(backIndex));
    }
    else
    {
        listSelectionModel->blockSignals(1);
        listSelectionModel->clear();
    }

    listSelectionModel->blockSignals(0);
    QTimer::singleShot(30, this, SLOT(dirLoaded()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::dirLoaded()
{
    if(backIndex.isValid())
    {
        backIndex = QModelIndex();
        return;
    }

    qint64 bytes = 0;
    QModelIndexList items;
    bool includeHidden = hiddenAct->isChecked();

    for(int x = 0; x < modelList->rowCount(modelList->index(pathEdit->currentText())); ++x)
        items.append(modelList->index(x, 0, modelList->index(pathEdit->currentText())));


    foreach(QModelIndex theItem,items)
    {
        if(includeHidden || !modelList->fileInfo(theItem).isHidden())
            bytes = bytes + modelList->size(theItem);
        else
            items.removeOne(theItem);
    }

    QString total;

    if(!bytes)
        total = "";
    else
        total = formatSize(bytes);

    if(thumbsAct->isChecked())
        QtConcurrent::run(modelList, &myModel::loadThumbs, items);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::thumbUpdate(QModelIndex index)
{
    if(currentView == 2)
        treeViewFileSystem->update(modelView->mapFromSource(index));
    else
        listViewFileSystem->update(modelView->mapFromSource(index));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::listSelectionChanged(const QItemSelection selected, const QItemSelection deselected)
{
    QModelIndexList items;

    if(listSelectionModel->selectedRows(0).count())
        items = listSelectionModel->selectedRows(0);
    else
        items = listSelectionModel->selectedIndexes();

    if(items.count() == 0)
    {
        curIndex = pathEdit->itemText(0);
        return;
    }

    if(QApplication::focusWidget() != listViewPlaces)
        listViewPlaces->clearSelection();

    curIndex = modelList->fileInfo(modelView->mapToSource(listSelectionModel->currentIndex()));

    qint64 bytes = 0;
    int folders = 0;
    int files = 0;

    foreach(QModelIndex theItem, items)
    {
        if(modelList->isDir(modelView->mapToSource(theItem)))
            folders++;
        else
            files++;

        bytes = bytes + modelList->size(modelView->mapToSource(theItem));
    }

    QString total, name;

    if(!bytes)
        total = "";
    else
        total = formatSize(bytes);

    if(items.count() == 1)
    {
        QFileInfo file(modelList->filePath(modelView->mapToSource(items.at(0))));

        name = file.fileName();
        if(file.isSymLink()) name = "Link --> " + file.symLinkTarget();

//        statusName->setText(name + "   ");
//        statusSize->setText(QString("%1   ").arg(total));
//        statusDate->setText(QString("%1").arg(file.lastModified().toString(Qt::SystemLocaleShortDate)));
    }
    else
    {
//        statusName->setText(total + "   ");
//        if(files) statusSize->setText(QString("%1 files  ").arg(files));
//        if(folders) statusDate->setText(QString("%1 folders").arg(folders));
    }
}

void MainWindowFileManager::listItemClicked(QModelIndex current)
{
    if(modelList->filePath(modelView->mapToSource(current)) == pathEdit->currentText())
        return;

    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if(mods == Qt::ControlModifier || mods == Qt::ShiftModifier)
        return;
    if(modelList->isDir(modelView->mapToSource(current)))
        treeViewFolders->setCurrentIndex(modelTree->mapFromSource(modelView->mapToSource(current)));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::listItemPressed(QModelIndex current)
{
    //middle-click -> open new tab
    //ctrl+middle-click -> open new instance

    if(QApplication::mouseButtons() == Qt::MidButton)
        if(modelList->isDir(modelView->mapToSource(current)))
        {
            if(QApplication::keyboardModifiers() == Qt::ControlModifier)
                openFile();
            else
                addTab(modelList->filePath(modelView->mapToSource(current)));
        }
        else
            openFile();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::openTab()
{
    if(curIndex.isDir())
        addTab(curIndex.filePath());
}

//---------------------------------------------------------------------------
int MainWindowFileManager::addTab(QString path)
{
    if(tabs->count() == 0)
        tabs->addNewTab(pathEdit->currentText(), currentView);

    return tabs->addNewTab(path, currentView);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::tabsOnTop()
{
    if(tabsOnTopAct->isChecked())
    {
        //mainLayout->setDirection(QBoxLayout::BottomToTop);
        tabs->setShape(QTabBar::RoundedNorth);
    }
    else
    {
        //mainLayout->setDirection(QBoxLayout::TopToBottom);
        tabs->setShape(QTabBar::RoundedSouth);
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::tabChanged(int index)
{
    if(tabs->count() == 0)
        return;

    pathEdit->clear();
    pathEdit->addItems(*tabs->getHistory(index));

    int type = tabs->getType(index);
    if(currentView != type)
    {
        if(type == 2) detailAct->setChecked(1);
        else
            detailAct->setChecked(0);

        if(type == 1)
            iconAct->setChecked(1);
        else
            iconAct->setChecked(0);

        toggleDetails();
    }

    if(!tabs->tabData(index).toString().isEmpty())
        treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(
                                                modelList->index(tabs->tabData(index).toString())));
}

void MainWindowFileManager::listDoubleClicked(QModelIndex current)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    if(mods == Qt::ControlModifier || mods == Qt::ShiftModifier) return;

    if(modelList->isDir(modelView->mapToSource(current)))
        treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(modelView->mapToSource(current)));
    else
        executeFile(current, 0);
}


//---------------------------------------------------------------------------
void MainWindowFileManager::executeFile(QModelIndex index, bool run)
{
    QProcess *myProcess = new QProcess(this);

    if(run)
        myProcess->startDetached(modelList->filePath(modelView->mapToSource(index)));    //is executable?
    else
    {
        QString type = modelList->getMimeType(modelView->mapToSource(index));

        QHashIterator<QString, QAction*> i(*customActions);
        while (i.hasNext())
        {
            i.next();
            if(type.contains(i.key()))
                if(i.value()->text() == "Open")
                {
                    i.value()->trigger();
                    return;
                }
        }

        myProcess->start("xdg-open", QStringList() << modelList->filePath(modelView->mapToSource(index)));
        myProcess->waitForFinished(1000);
        myProcess->terminate();
        if(myProcess->exitCode() != 0)
        {
            if(xdgConfig())
                executeFile(index,run);
        }
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::runFile()
{
    executeFile(listSelectionModel->currentIndex(), 1);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::openFolderAction()
{
    treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(listSelectionModel->currentIndex()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::openFile()
{
    QModelIndexList items;

    if(listSelectionModel->selectedRows(0).count())
        items = listSelectionModel->selectedRows(0);
    else
        items = listSelectionModel->selectedIndexes();

    foreach(QModelIndex index, items)
        executeFile(index,0);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::goUpDir()
{
    treeViewFileSystem->setCurrentIndex(treeViewFileSystem->currentIndex().parent());
}

//---------------------------------------------------------------------------
void MainWindowFileManager::goBackDir()
{
    if(pathEdit->count() == 1)
        return;

    QString current = pathEdit->currentText();

    if(current.contains(pathEdit->itemText(1)))
        backIndex = modelList->index(current);

    do
    {
        pathEdit->removeItem(0);
        if(tabs->count()) tabs->remHistory();
    }
    while(!QFileInfo(pathEdit->itemText(0)).exists() || pathEdit->itemText(0) == current);

    treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(modelList->index(pathEdit->itemText(0))));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::goHomeDir()
{
    treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(modelList->index(QDir::homePath())));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::pathEditChanged(QString path)
{
    QString info = path;

    if(!QFileInfo(path).exists())
        return;

    info.replace("~", QDir::homePath());

    QModelIndex temp = modelList->index(info);
    //modelTree->invalidate();

    treeViewFileSystem->setCurrentIndex(modelTree->mapFromSource(temp));
}

void MainWindowFileManager::initializeTerminal()
{
    termWidgetTerminal->setShellProgram(Properties::Instance()->shell);
    termWidgetTerminal->setColorScheme(Properties::Instance()->colorScheme);
    termWidgetTerminal->setFont(Properties::Instance()->font);
    if(Properties::Instance()->historyLimited)
        termWidgetTerminal->setHistorySize(Properties::Instance()->historyLimitedTo);
    else
        termWidgetTerminal->setHistorySize(0);
    termWidgetTerminal->setTerminalOpacity(Properties::Instance()->termOpacity);
    termWidgetTerminal->setScrollBarPosition((QTermWidget::ScrollBarPosition) Properties::Instance()->scrollBarPos);
}

void MainWindowFileManager::terminalRun()
{
    dockWidgetTerminal->setVisible(!dockWidgetTerminal->isVisible());
}

void MainWindowFileManager::newDir()
{
    QModelIndex newDir;

    if(!QFileInfo(pathEdit->itemText(0)).isWritable())
    {
        labelStatus->setText(tr("Read only...cannot create folder"));
        return;
    }

    newDir = modelView->mapFromSource(modelList->insertFolder(modelList->index(pathEdit->itemText(0))));
    listSelectionModel->setCurrentIndex(newDir,QItemSelectionModel::ClearAndSelect);

    if(stackedWidget->currentIndex() == 0)
        listViewFileSystem->edit(newDir);
    else
        treeViewFileSystem->edit(newDir);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::newFile()
{
    QModelIndex fileIndex;

    if(!QFileInfo(pathEdit->itemText(0)).isWritable())
    {
        labelStatus->setText(tr("Read only...cannot create file"));
        return;
    }

    fileIndex = modelView->mapFromSource(modelList->insertFile(modelList->index(pathEdit->itemText(0))));
    listSelectionModel->setCurrentIndex(fileIndex,QItemSelectionModel::ClearAndSelect);

    if(stackedWidget->currentIndex() == 0)
        listViewFileSystem->edit(fileIndex);
    else
        treeViewFileSystem->edit(fileIndex);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::deleteFile()
{
    QModelIndexList selList;
    bool yesToAll = false;

    if(focusWidget() == treeViewFileSystem)
        selList << modelList->index(pathEdit->itemText(0));
    else
    {
        QModelIndexList proxyList;
        if(listSelectionModel->selectedRows(0).count())
            proxyList = listSelectionModel->selectedRows(0);
        else
            proxyList = listSelectionModel->selectedIndexes();

        foreach(QModelIndex proxyItem, proxyList)
            selList.append(modelView->mapToSource(proxyItem));
    }

    bool ok = false;
    bool confirm;

    if(Properties::Instance()->confirmDelete)					//not defined yet
    {
        if(QMessageBox::question(this,
                                 tr("Delete confirmation"),
                                 tr("Do you want to confirm all delete operations?"),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            confirm = true;
        }
        else
        {
            confirm = false;
        }
        Properties::Instance()->confirmDelete = confirm;
    }
    else
    {
        confirm = Properties::Instance()->confirmDelete;
    }

    for(int i = 0; i < selList.count(); ++i)
    {
        QFileInfo file(modelList->filePath(selList.at(i)));
        if(file.isWritable())
        {
            if(file.isSymLink())
            {
                ok = QFile::remove(file.filePath());
            }
            else
            {
                if(yesToAll == false)
                {
                    if(confirm)
                    {
                        int ret = QMessageBox::information(this,
                                                           tr("Careful"),
                                                           tr("Are you sure you want to delete <p><b>\"") + file.filePath() + "</b>?",
                                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll);
                        if(ret == QMessageBox::YesToAll)
                            yesToAll = true;
                        if(ret == QMessageBox::No)
                            return;
                    }
                }
                ok = modelList->remove(selList.at(i));
            }
        }
        else if(file.isSymLink())
            ok = QFile::remove(file.filePath());
    }

    if(!ok)
        QMessageBox::information(this,
                                 tr("Failed"),
                                 tr("Could not delete some items...do you have the right permissions?"));

    return;
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleIcons()
{
    if(listViewFileSystem->rootIndex() != modelList->index(pathEdit->currentText()))
            listViewFileSystem->setRootIndex(modelView->mapFromSource(
                                                 modelList->index(pathEdit->currentText())));

    if(iconAct->isChecked())
    {
        currentView = 1;
        listViewFileSystem->setViewMode(QListView::IconMode);
        listViewFileSystem->setGridSize(QSize(zoom + 32, zoom + 32));
        listViewFileSystem->setIconSize(QSize(zoom, zoom));
        listViewFileSystem->setFlow(QListView::LeftToRight);

        modelList->setMode(thumbsAct->isChecked());

        stackedWidget->setCurrentIndex(0);
        detailAct->setChecked(0);
        treeViewFileSystem->setMouseTracking(false);

        listViewFileSystem->setMouseTracking(true);

        if(tabs->count())
            tabs->setType(1);
    }
    else
    {
        currentView = 0;
        listViewFileSystem->setViewMode(QListView::ListMode);
        listViewFileSystem->setGridSize(QSize());
        listViewFileSystem->setIconSize(QSize(zoomList, zoomList));
        listViewFileSystem->setFlow(QListView::TopToBottom);

        modelList->setMode(thumbsAct->isChecked());
        listViewFileSystem->setMouseTracking(false);

        if(tabs->count())
            tabs->setType(0);
    }

    listViewFileSystem->setDragDropMode(QAbstractItemView::DragDrop);
    listViewFileSystem->setDefaultDropAction(Qt::MoveAction);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleThumbs()
{
    if(currentView != 2)
        toggleIcons();
    else
        toggleDetails();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleDetails()
{
    if(detailAct->isChecked() == false)
    {
        toggleIcons();

        stackedWidget->setCurrentIndex(0);
        treeViewFileSystem->setMouseTracking(false);
    }
    else
    {
        currentView = 2;
        if(treeViewFileSystem->rootIndex() != modelList->index(pathEdit->currentText()))
           treeViewFileSystem->setRootIndex(modelView->mapFromSource(
                                                modelList->index(pathEdit->currentText())));

        treeViewFileSystem->setMouseTracking(true);

        stackedWidget->setCurrentIndex(1);
        modelList->setMode(thumbsAct->isChecked());
        iconAct->setChecked(0);

        if(tabs->count())
            tabs->setType(2);
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleHidden()
{
    if(hiddenAct->isChecked() == false)
    {
        if(curIndex.isHidden())
            listSelectionModel->clear();

        modelView->setFilterRegExp("no");
        modelTree->setFilterRegExp("no");
    }
    else
    {
        modelView->setFilterRegExp("");
        modelTree->setFilterRegExp("");
    }

    modelView->invalidate();
    dirLoaded();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::clipboardChanged()
{
    if(QApplication::clipboard()->mimeData()->hasUrls())
        pasteAct->setEnabled(true);
    else
    {
        modelList->clearCutItems();
        pasteAct->setEnabled(false);
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::cutFile()
{
    QModelIndexList selList;
    QStringList fileList;

    if(focusWidget() == treeViewFileSystem)
        selList << modelView->mapFromSource(modelList->index(pathEdit->itemText(0)));
    else if(listSelectionModel->selectedRows(0).count())
        selList = listSelectionModel->selectedRows(0);
    else
        selList = listSelectionModel->selectedIndexes();


    foreach(QModelIndex item, selList)
        fileList.append(modelList->filePath(modelView->mapToSource(item)));

    clearCutItems();
    modelList->addCutItems(fileList);

    //save a temp file to allow pasting in a different instance
    QFile tempFile(QDir::tempPath() + "/qtfm.temp");
    tempFile.open(QIODevice::WriteOnly);
    QDataStream out(&tempFile);
    out << fileList;
    tempFile.close();

    QApplication::clipboard()->setMimeData(modelView->mimeData(selList));

    modelTree->invalidate();
    listSelectionModel->clear();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::copyFile()
{
    QModelIndexList selList;
    if(listSelectionModel->selectedRows(0).count()) selList = listSelectionModel->selectedRows(0);
    else selList = listSelectionModel->selectedIndexes();

    if(selList.count() == 0)
        if(focusWidget() == treeViewFileSystem)
            selList << modelView->mapFromSource(modelList->index(pathEdit->itemText(0)));
    else
            return;

    clearCutItems();

    QStringList text;
    foreach(QModelIndex item, selList)
        text.append(modelList->filePath(modelView->mapToSource(item)));

    QApplication::clipboard()->setText(text.join("\n"), QClipboard::Selection);
    QApplication::clipboard()->setMimeData(modelView->mimeData(selList));

    cutAct->setData(0);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::pasteClipboard()
{
    QString newPath;
    QStringList cutList;

    if(curIndex.isDir())
        newPath = curIndex.filePath();
    else
        newPath = pathEdit->itemText(0);

    //check list of files that are to be cut
    QFile tempFile(QDir::tempPath() + "/qtfm.temp");
    if(tempFile.exists())
    {
        QModelIndexList list;
        tempFile.open(QIODevice::ReadOnly);
        QDataStream out(&tempFile);
        out >> cutList;
        tempFile.close();
    }

    pasteLauncher(QApplication::clipboard()->mimeData(), newPath, cutList);
}
//---------------------------------------------------------------------------
void MainWindowFileManager::pasteLauncher(const QMimeData * data, QString newPath, QStringList cutList)
{
    QList<QUrl> files = data->urls();

    if(!QFile(files.at(0).path()).exists())
    {
        QMessageBox::information(this,tr("No paste for you!"), tr("File no longer exists!"));
        pasteAct->setEnabled(false);
        return;
    }

    int replace = 0;
    QStringList completeList;
    QString baseName = QFileInfo(files.at(0).toLocalFile()).path();

    if(newPath != baseName)			    //only if not in same directory, otherwise we will do 'Copy(x) of'
    {
        foreach(QUrl file, files)
        {
            QFileInfo temp(file.toLocalFile());

            if(temp.isDir() && QFileInfo(newPath + "/" + temp.fileName()).exists())     //merge or replace?
            {
                QMessageBox message(QMessageBox::Question,
                                    tr("Existing folder"),
                                    QString("<b>%1</b><p>Already exists!<p>What do you want to do?")
                    .arg(newPath + "/" + temp.fileName()),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

                message.button(QMessageBox::Yes)->setText(tr("Merge"));
                message.button(QMessageBox::No)->setText(tr("Replace"));

                int merge = message.exec();
                if(merge == QMessageBox::Cancel)
                    return;
                if(merge == QMessageBox::Yes)
                    recurseFolder(temp.filePath(), temp.fileName(), &completeList);
                else
                {
                    //physically remove files from disk
                    QStringList children;
                    QDirIterator it(newPath + "/" + temp.fileName(),QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
                    while (it.hasNext())
                        children.prepend(it.next());
                    children.append(newPath + "/" + temp.fileName());
                    foreach(QString child,children)
                        QFile(child).remove();
                }
            }
            else
                completeList.append(temp.fileName());
        }

        foreach(QString file, completeList)
        {
            QFileInfo temp(newPath + "/" + file);
            if(temp.exists())
            {
                QFileInfo orig(baseName + "/" + file);
                if(replace != QMessageBox::YesToAll && replace != QMessageBox::NoToAll)
                        replace = QMessageBox::question(0,tr("Replace"),QString("Do you want to replace:<p><b>%1</p><p>Modified: %2<br>Size: %3 bytes</p><p>with:<p><b>%4</p><p>Modified: %5<br>Size: %6 bytes</p>")
                                .arg(temp.filePath()).arg(temp.lastModified().toString()).arg(temp.size())
                                .arg(orig.filePath()).arg(orig.lastModified().toString()).arg(orig.size()),QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);

                if(replace == QMessageBox::Cancel) return;
                if(replace == QMessageBox::Yes || replace == QMessageBox::YesToAll)
                    QFile(temp.filePath()).remove();
            }

        }
    }

    QString title;
    if(cutList.count() == 0) title = tr("Copying...");
    else title = tr("Moving...");

    progress = new myProgressDialog(title);
    connect(this,SIGNAL(updateCopyProgress(qint64, qint64, QString)),progress,SLOT(update(qint64, qint64, QString)));

    listSelectionModel->clear();
    QtConcurrent::run(this, &MainWindowFileManager::pasteFile, files, newPath, cutList);
}

//---------------------------------------------------------------------------
void MainWindowFileManager::recurseFolder(QString path, QString parent, QStringList *list)
{
    QDir dir(path);
    QStringList files = dir.entryList(QDir::AllEntries | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);

    for(int i = 0; i < files.count(); i++)
    {
        if(QFileInfo(files.at(i)).isDir()) recurseFolder(files.at(i),parent + "/" + files.at(i),list);
        else list->append(parent + "/" + files.at(i));
    }
}

//---------------------------------------------------------------------------
void MainWindowFileManager::progressFinished(int ret,QStringList newFiles)
{
    if(progress != 0)
    {
        progress->close();
        delete progress;
        progress = 0;
    }

    if(newFiles.count())
    {
        disconnect(listSelectionModel,SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),this,SLOT(listSelectionChanged(const QItemSelection, const QItemSelection)));

        qApp->processEvents();              //make sure notifier has added new files to the model

        if(QFileInfo(newFiles.first()).path() == pathEdit->currentText())       //highlight new files if visible
        {
            foreach(QString item, newFiles)
                listSelectionModel->select(modelView->mapFromSource(modelList->index(item)),QItemSelectionModel::Select);
        }

        connect(listSelectionModel,SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)),this,SLOT(listSelectionChanged(const QItemSelection, const QItemSelection)));
        curIndex.setFile(newFiles.first());

        if(currentView == 2)
            treeViewFileSystem->scrollTo(
                        modelView->mapFromSource(
                            modelList->index(newFiles.first())), QAbstractItemView::EnsureVisible);
        else
            listViewFileSystem->scrollTo(modelView->mapFromSource(
                                             modelList->index(newFiles.first())), QAbstractItemView::EnsureVisible);

        if(QFile(QDir::tempPath() + "/qtfm.temp").exists())
            QApplication::clipboard()->clear();

        clearCutItems();
    }

    if(ret == 1)
        QMessageBox::information(this,tr("Failed"),tr("Paste failed...do you have write permissions?"));
    if(ret == 2)
        QMessageBox::warning(this,tr("Too big!"),tr("There is not enough space on the destination drive!"));
}

//---------------------------------------------------------------------------
bool MainWindowFileManager::pasteFile(QList<QUrl> files,QString newPath, QStringList cutList)
{
    bool ok = true;
    QStringList newFiles;

    if(!QFileInfo(newPath).isWritable() || newPath == QDir(files.at(0).toLocalFile()).path())        //quit if folder not writable
    {
        emit copyProgressFinished(1, newFiles);
        return 0;
    }

    //get total size in bytes
    qint64 total = 1;
    foreach(QUrl url, files)
    {
        QFileInfo file = url.path();
        if(file.isFile()) total += file.size();
        else
        {
            QDirIterator it(url.path(),QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                it.next();
            total += it.fileInfo().size();
            }
        }
    }

    //check available space on destination before we start
    struct statfs info;
    statfs(newPath.toLocal8Bit(), &info);

    if((qint64) info.f_bavail*info.f_bsize < total)
    {
        //if it is a cut/move on the same device it doesn't matter
        if(cutList.count())
        {
            qint64 driveSize = (qint64) info.f_bavail*info.f_bsize;
            statfs(files.at(0).path().toLocal8Bit(),&info);
            if((qint64) info.f_bavail*info.f_bsize != driveSize)        //same device
            {
                emit copyProgressFinished(2,newFiles);
                return 0;
            }
        }
        else
        {
            emit copyProgressFinished(2,newFiles);
            return 0;
        }
    }


    //main loop
    for(int i = 0; i < files.count(); ++i)
    {
        if(progress->result() == 1)			//cancelled
        {
            emit copyProgressFinished(0,newFiles);
            return 1;
        }

        QFileInfo temp(files.at(i).toLocalFile());
        QString destName = temp.fileName();

        if(temp.path() == newPath)			// only do 'Copy(x) of' if same folder
        {
            int num = 1;
            while(QFile(newPath + "/" + destName).exists())
            {
                destName = QString("Copy (%1) of %2").arg(num).arg(temp.fileName());
                num++;
            }
        }

        destName = newPath + "/" + destName;
        QFileInfo dName(destName);

        if(!dName.exists() || dName.isDir())
        {
            newFiles.append(destName);				    //keep a list of new files so we can select them later

            if(cutList.contains(temp.filePath()))			    //cut action
            {
                if (temp.isFile()) //files
                {
                    QFSFileEngine file(temp.filePath());
                    if(!file.rename(destName))			    //rename will fail if across different filesystem, so use copy/remove method
                        ok = cutCopyFile(temp.filePath(), destName, total, true);
                }
                else
                {
                    ok = QFile(temp.filePath()).rename(destName);
                    if(!ok)	//file exists or move folder between different filesystems, so use copy/remove method
                    {
                        if(temp.isDir())
                        {
                            ok = true;
                            copyFolder(temp.filePath(), destName, total, true);
                            modelList->clearCutItems();
                        }
                        //file already exists, don't do anything
                    }
                }
            }
            else
            {
                if(temp.isDir())
                    copyFolder(temp.filePath(),destName,total,false);
                else
                    ok = cutCopyFile(temp.filePath(), destName, total, false);
            }
        }
    }

    emit copyProgressFinished(0,newFiles);
    return 1;
}

//---------------------------------------------------------------------------
bool MainWindowFileManager::cutCopyFile(QString source, QString dest, qint64 totalSize, bool cut)
{
    QFile in(source);
    QFile out(dest);

    if(out.exists()) return 1;  //file exists, don't do anything

    if (dest.length() > 50) dest = "/.../" + dest.split("/").last();

    in.open(QFile::ReadOnly);
    out.open(QFile::WriteOnly);

    char block[4096];
    qint64 total = in.size();
    qint64 steps = total >> 7;        //shift right 7, same as divide 128, much faster
    qint64 interTotal = 0;

    while(!in.atEnd())
    {
        if(progress->result() == 1) break;                 //cancelled

        qint64 inBytes = in.read(block, sizeof(block));
        out.write(block, inBytes);
        interTotal += inBytes;

        if(interTotal > steps)
        {
            emit updateCopyProgress(interTotal,totalSize,dest);
            interTotal = 0;
        }
    }

    emit updateCopyProgress(interTotal,totalSize,dest);

    out.close();
    in.close();

    if(out.size() != total) return 0;
    if(cut) in.remove();  //if file is cut remove the source
    return 1;
}

//---------------------------------------------------------------------------
bool MainWindowFileManager::copyFolder(QString sourceFolder, QString destFolder, qint64 total, bool cut)
{
    QDir sourceDir(sourceFolder);
    QDir destDir(QFileInfo(destFolder).path());
    QString folderName = QFileInfo(destFolder).fileName();

    bool ok = true;

    if(!QFileInfo(destFolder).exists()) destDir.mkdir(folderName);
    destDir = QDir(destFolder);

    QStringList files = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);

    for(int i = 0; i < files.count(); i++)
    {
        QString srcName = sourceDir.path() + "/" + files[i];
        QString destName = destDir.path() + "/" + files[i];
        if(!cutCopyFile(srcName, destName, total, cut))
            ok = false;     //don't remove source folder if all files not cut

        if(progress->result() == 1)
            return 0;                           //cancelled
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);

    for(int i = 0; i < files.count(); i++)
    {
        if(progress->result() == 1)
            return 0;			    //cancelled

        QString srcName = sourceDir.path() + "/" + files[i];
        QString destName = destDir.path() + "/" + files[i];
        copyFolder(srcName, destName, total, cut);
    }

    //remove source folder if all files moved ok
    if(cut && ok)
        sourceDir.rmdir(sourceFolder);
    return ok;
}

//---------------------------------------------------------------------------
void MainWindowFileManager::folderPropertiesLauncher()
{
    QModelIndexList selList;
    if(focusWidget() == listViewPlaces)
        selList.append(modelView->mapFromSource(
                           modelList->index(listViewPlaces->currentIndex().data(32).toString())));
    else if(focusWidget() == listViewPlaces || focusWidget() == treeViewFolders)
        if(listSelectionModel->selectedRows(0).count())
            selList = listSelectionModel->selectedRows(0);
        else
            selList = listSelectionModel->selectedIndexes();

    if(selList.count() == 0)
        selList << modelView->mapFromSource(modelList->index(pathEdit->currentText()));

    QStringList paths;

    foreach(QModelIndex item, selList)
        paths.append(modelList->filePath(modelView->mapToSource(item)));

    properties = new propertiesDialog(paths, modelList);
    connect(properties,SIGNAL(propertiesUpdated()),this,SLOT(clearCutItems()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::renameFile()
{
    if(focusWidget() == treeViewFileSystem)
        treeViewFileSystem->edit(treeSelectionModel->currentIndex());
    else if(focusWidget() == listViewPlaces)
            listViewPlaces->edit(listViewPlaces->currentIndex());
    else if(focusWidget() == listViewFileSystem)
            listViewFileSystem->edit(listSelectionModel->currentIndex());
    else if(focusWidget() == treeViewFolders)
            treeViewFolders->edit(listSelectionModel->currentIndex());
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleLockLayout()
{
    if(lockLayoutAct->isChecked())
    {
        QFrame *newTitle = new QFrame();
        newTitle->setFrameShape(QFrame::StyledPanel);
        newTitle->setMinimumSize(0,1);
        dockWidgetFolders->setTitleBarWidget(newTitle);

        newTitle = new QFrame();
        newTitle->setFrameShape(QFrame::StyledPanel);
        newTitle->setMinimumSize(0,1);
        dockWidgetPlaces->setTitleBarWidget(newTitle);

        toolBarMenu->setMovable(false);
        toolBarEdit->setMovable(false);
        toolBarView->setMovable(false);
        toolBarNavigation->setMovable(false);
        toolBarAddress->setMovable(false);
        lockLayoutAct->setText(tr("Unlock layout"));
    }
    else
    {
        dockWidgetFolders->setTitleBarWidget(0);
        dockWidgetPlaces->setTitleBarWidget(0);

        toolBarMenu->setMovable(true);
        toolBarEdit->setMovable(true);
        toolBarView->setMovable(true);
        toolBarNavigation->setMovable(true);
        toolBarAddress->setMovable(true);

        lockLayoutAct->setText(tr("Lock layout"));
    }
}

//---------------------------------------------------------------------------
bool MainWindowFileManager::xdgConfig()
{
    if(!listSelectionModel->currentIndex().isValid()) return 0;

    QDialog *xdgConfig = new QDialog(this);
    xdgConfig->setWindowTitle(tr("Configure filetype"));

    QString mimeType = gGetMimeType(curIndex.filePath());

    QLabel *label = new QLabel(tr("Filetype:") + "<b>" + mimeType + "</b><p>" + tr("Open with:"));
    QComboBox * appList = new QComboBox;

    QDialogButtonBox *buttons = new QDialogButtonBox;
    buttons->setStandardButtons(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), xdgConfig, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), xdgConfig, SLOT(reject()));

    appList->setEditable(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(appList);
    layout->addWidget(buttons);
    xdgConfig->setLayout(layout);

    QStringList apps;
    QDirIterator it("/usr/share/applications",QStringList("*.desktop"),QDir::Files | QDir::NoDotAndDotDot , QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        apps.append(it.fileName());
    }

    apps.replaceInStrings(".desktop","");
    apps.sort();
    appList->addItems(apps);

    //now get the icons
    QDir appIcons("/usr/share/pixmaps","",0,QDir::Files | QDir::NoDotAndDotDot);
    apps = appIcons.entryList();
    QIcon defaultIcon = QIcon::fromTheme("application-x-executable");

    for(int i = 0; i < appList->count(); ++i)
    {
        QString baseName = appList->itemText(i);
        QPixmap temp = QIcon::fromTheme(baseName).pixmap(16,16);

        if(!temp.isNull()) appList->setItemIcon(i,temp);
        else
        {
            QStringList searchIcons = apps.filter(baseName);
            if(searchIcons.count() > 0) appList->setItemIcon(i,QIcon("/usr/share/pixmaps/" + searchIcons.at(0)));
            else appList->setItemIcon(i,defaultIcon);
        }
    }

    QString xdgDefaults;

    //xdg changes -> now uses mimeapps.list instead of defaults.list
    if(QFileInfo(QDir::homePath() + "/.local/share/applications/mimeapps.list").exists())
        xdgDefaults = QDir::homePath() + "/.local/share/applications/mimeapps.list";
    else
        xdgDefaults = QDir::homePath() + "/.local/share/applications/defaults.list";

    QSettings defaults(xdgDefaults,QSettings::IniFormat,this);

    QString temp = defaults.value("Default Applications/" + mimeType).toString();
    appList->setCurrentIndex(appList->findText(temp.remove(".desktop"),Qt::MatchFixedString));

    bool ok = xdgConfig->exec();
    if(ok)
    {
        QProcess *myProcess = new QProcess(this);
        myProcess->start("xdg-mime",QStringList() << "default" << appList->currentText() + ".desktop" << mimeType);
    }

    delete xdgConfig;
    return ok;
}

//---------------------------------------------------------------------------
void MainWindowFileManager::readCustomActions()
{
    customMenus = new QMultiHash<QString,QMenu*>;

    Properties::Instance()->settings.beginGroup("customActions");
    QStringList keys = Properties::Instance()->settings.childKeys();

    for(int i = 0; i < keys.count(); ++i)
    {
        keys.insert(i, keys.takeLast());  //reverse order

        QStringList temp(Properties::Instance()->settings.value(keys.at(i)).toStringList());

        // temp.at(0) - FileType
        // temp.at(1) - Text
        // temp.at(2) - Icon
        // temp.at(3) - Command

        QAction *tempAction = new QAction(QIcon::fromTheme(temp.at(2)),temp.at(1),this);
        customMapper->setMapping(tempAction,temp.at(3));
        connect(tempAction, SIGNAL(triggered()), customMapper, SLOT(map()));

        actionList->append(tempAction);

        QStringList types = temp.at(0).split(",");

        foreach(QString type,types)
        {
            QStringList children(temp.at(1).split(" / "));
            if(children.count() > 1)
            {
                QMenu * parent = 0;
                tempAction->setText(children.at(1));

                foreach(QMenu *subMenu,customMenus->values(type))
                    if(subMenu->title() == children.at(0))
                        parent = subMenu;

                if(parent == 0)
                {
                    parent = new QMenu(children.at(0));
                    customMenus->insert(type,parent);
                }

                parent->addAction(tempAction);
                customActions->insert("null",tempAction);
            }
            else
                customActions->insert(type,tempAction);
        }
    }
    Properties::Instance()->settings.endGroup();

    readShortcuts();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::editCustomActions()
{
    //remove all custom actions from list because we will add them all again below.
    foreach(QAction *action, *actionList)
        if(customActions->values().contains(action))
        {
            actionList->removeOne(action);
            delete action;
        }

    QList<QMenu*> temp = customMenus->values();

    foreach(QMenu* menu, temp)
        delete menu;

    customActions->clear();
    customMenus->clear();

    customActionsDialog *dlg = new customActionsDialog(this);
    dlg->exec();
    readCustomActions();
    delete dlg;
}

//---------------------------------------------------------------------------
void MainWindowFileManager::writeSettings()
{
    Properties::Instance()->saveSettings();

    Properties::Instance()->settings.remove("bookmarks");
    Properties::Instance()->settings.beginGroup("bookmarks");

    for(int i = 0; i < modelBookmarks->rowCount(); i++)
    {
        QStringList temp;
        temp << modelBookmarks->item(i)->text()
             << modelBookmarks->item(i)->data(32).toString()
             << modelBookmarks->item(i)->data(34).toString()
             << modelBookmarks->item(i)->data(33).toString();

        Properties::Instance()->settings.setValue(QString(i),temp);
    }
    Properties::Instance()->settings.endGroup();
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu *popup;
    QWidget *widget = childAt(event->pos());

    if(widget == tabs)
    {
        popup = new QMenu(this);
        popup->addAction(closeTabAct);
        popup->exec(event->globalPos());
        return;
    }

    QList<QAction*> actions;
    popup = new QMenu(this);

    if(focusWidget() == listViewFileSystem || focusWidget() == treeViewFolders)
    {
        listViewPlaces->clearSelection();

        if(listSelectionModel->hasSelection())	    //could be file or folder
        {
            curIndex = modelList->filePath(modelView->mapToSource(listSelectionModel->currentIndex()));

            if(!curIndex.isDir())		    //file
            {
                QString type = modelList->getMimeType(modelList->index(curIndex.filePath()));

                QHashIterator<QString, QAction*> i(*customActions);
                while (i.hasNext())
                {
                    i.next();
                    if(type.contains(i.key())) actions.append(i.value());
                }

                foreach(QAction*action, actions)
                {
                    if(action->text() == "Open")
                    {
                        popup->addAction(action);
                        break;
                    }
                }

                if(popup->actions().count() == 0)
                    popup->addAction(openAct);

                if(curIndex.isExecutable())
                    popup->addAction(runAct);

                popup->addActions(actions);

                QHashIterator<QString, QMenu*> m(*customMenus);
                while (m.hasNext())
                {
                    m.next();
                    if(type.contains(m.key()))
                        popup->addMenu(m.value());
                }

                popup->addSeparator();
                popup->addAction(cutAct);
                popup->addAction(copyAct);
                popup->addAction(pasteAct);
                popup->addSeparator();
                popup->addAction(renameAct);
                popup->addSeparator();

                foreach(QMenu* parent, customMenus->values("*"))
                    popup->addMenu(parent);

                actions = (customActions->values("*"));
                popup->addActions(actions);
                popup->addAction(deleteAct);
                popup->addSeparator();
                actions = customActions->values(curIndex.path());    //children of $parent
                if(actions.count())
                {
                    popup->addActions(actions);
                    popup->addSeparator();
                }
            }
            else
            {
                //folder
                popup->addAction(openAct);
                popup->addSeparator();
                popup->addAction(cutAct);
                popup->addAction(copyAct);
                popup->addAction(pasteAct);
                popup->addSeparator();
                popup->addAction(renameAct);
                popup->addSeparator();

                foreach(QMenu* parent, customMenus->values("*"))
                    popup->addMenu(parent);

                actions = customActions->values("*");
                popup->addActions(actions);
                popup->addAction(deleteAct);
                popup->addSeparator();

                foreach(QMenu* parent, customMenus->values("folder"))
                    popup->addMenu(parent);

                actions = customActions->values(curIndex.fileName());               //specific folder
                actions.append(customActions->values(curIndex.path()));    //children of $parent
                actions.append(customActions->values("folder"));                    //all folders
                if(actions.count())
                {
                    popup->addActions(actions);
                    popup->addSeparator();
                }
            }

            popup->addAction(folderPropertiesAct);

        }
        else
        {   //whitespace
            popup->addAction(newDirAct);
            popup->addAction(newFileAct);
            popup->addSeparator();
            if(pasteAct->isEnabled())
            {
                popup->addAction(pasteAct);
                popup->addSeparator();
            }
            popup->addAction(addBookmarkAct);
            popup->addSeparator();

            foreach(QMenu* parent, customMenus->values("folder"))
                popup->addMenu(parent);
            actions = customActions->values(curIndex.fileName());
            actions.append(customActions->values("folder"));
            if(actions.count())
            {
                foreach(QAction*action, actions)
                    popup->addAction(action);
                popup->addSeparator();
            }

            popup->addAction(folderPropertiesAct);
        }
    }
    else
    {	//tree or bookmarks
        if(focusWidget() == listViewPlaces)
        {
            listSelectionModel->clearSelection();
            if(listViewPlaces->indexAt(listViewPlaces->mapFromGlobal(event->globalPos())).isValid())
            {
                curIndex = listViewPlaces->currentIndex().data(32).toString();
                popup->addAction(delBookmarkAct);
                popup->addAction(editBookmarkAct);	//icon
            }
            else
            {
                listViewPlaces->clearSelection();
                popup->addAction(addSeparatorAct);	//seperator
                popup->addAction(wrapBookmarksAct);
            }
            popup->addSeparator();
        }
        else
        {
            listViewPlaces->clearSelection();
            popup->addAction(newDirAct);
            popup->addAction(newFileAct);
            popup->addAction(openTabAct);
            popup->addSeparator();
            popup->addAction(cutAct);
            popup->addAction(copyAct);
            popup->addAction(pasteAct);
            popup->addSeparator();
            popup->addAction(renameAct);
            popup->addSeparator();
            popup->addAction(deleteAct);
        }

        popup->addSeparator();

        foreach(QMenu* parent, customMenus->values("folder"))
            popup->addMenu(parent);
        actions = customActions->values(curIndex.fileName());
        actions.append(customActions->values(curIndex.path()));
        actions.append(customActions->values("folder"));
        if(actions.count())
        {
            foreach(QAction*action, actions)
                popup->addAction(action);
            popup->addSeparator();
        }
        popup->addAction(folderPropertiesAct);
    }

    popup->exec(event->globalPos());
    delete popup;
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::actionMapper(QString cmd)
{
    QModelIndexList selList;
    QStringList temp;

    if(focusWidget() == listViewFileSystem || focusWidget() == treeViewFolders)
    {
        QFileInfo file = modelList->fileInfo(modelView->mapToSource(listSelectionModel->currentIndex()));

        if(file.isDir())
            cmd.replace("%n", file.fileName().replace(" ", "\\"));
        else
            cmd.replace("%n", file.baseName().replace(" ","\\"));

        if(listSelectionModel->selectedRows(0).count())
            selList = listSelectionModel->selectedRows(0);
        else
            selList = listSelectionModel->selectedIndexes();
    }
    else
        selList << modelView->mapFromSource(modelList->index(curIndex.filePath()));


    cmd.replace("~", QDir::homePath());


    //process any input tokens
    int pos = 0;
    while(pos >= 0)
    {
        pos = cmd.indexOf("%i", pos);
        if(pos != -1)
        {
            pos += 2;
            QString var = cmd.mid(pos, cmd.indexOf(" ", pos) - pos);
            QString input = QInputDialog::getText(this,tr("Input"), var, QLineEdit::Normal);
            if(input.isNull())
                return;              //cancelled
            else
                cmd.replace("%i" + var,input);
        }
    }


    foreach(QModelIndex index,selList)
        temp.append(modelList->fileName(modelView->mapToSource(index)).replace(" ","\\"));

    cmd.replace("%f", temp.join(" "));

    temp.clear();

    foreach(QModelIndex index,selList)
        temp.append(modelList->filePath(modelView->mapToSource(index)).replace(" ", "\\"));

    cmd.replace("%F", temp.join(" "));

    temp = cmd.split(" ");

    QString exec = temp.at(0);
    temp.removeAt(0);

    temp.replaceInStrings("\\", " ");

    QProcess *customProcess = new QProcess();
    customProcess->setWorkingDirectory(pathEdit->itemText(0));

    connect(customProcess,SIGNAL(error(QProcess::ProcessError)),this,SLOT(customActionError(QProcess::ProcessError)));
    connect(customProcess,SIGNAL(finished(int)),this,SLOT(customActionFinished(int)));

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if(exec.at(0) == '|')
    {
        exec.remove(0,1);
        env.insert("QTFM", "1");
        customProcess->setProcessEnvironment(env);
    }

    customProcess->start(exec,temp);
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::customActionError(QProcess::ProcessError error)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    QMessageBox::warning(this,"Error",process->errorString());
    customActionFinished(0);
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::customActionFinished(int ret)
{
    QProcess* process = qobject_cast<QProcess*>(sender());

    if(process->processEnvironment().contains("QTFM"))
    {
        QString output = process->readAllStandardError();
        if(!output.isEmpty()) QMessageBox::warning(this,tr("Error - Custom action"),output);

        output = process->readAllStandardOutput();
        if(!output.isEmpty()) QMessageBox::information(this,tr("Output - Custom action"),output);
    }

    QTimer::singleShot(100,this,SLOT(clearCutItems()));                //updates file sizes
    process->deleteLater();
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::refresh()
{
    QApplication::clipboard()->clear();
    listSelectionModel->clear();

    modelList->refreshItems();
    modelTree->invalidate();
    modelTree->sort(0,Qt::AscendingOrder);
    modelView->invalidate();
    dirLoaded();

    return;
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::newConnection()
{
    showNormal();
    daemon.close();
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::startDaemon()
{
    if(!daemon.listen("qtfilemanager"))
        isDaemon = 0;
}

//---------------------------------------------------------------------------------
void MainWindowFileManager::clearCutItems()
{
    //this refreshes existing items, sizes etc but doesn't re-sort
    modelList->clearCutItems();
    modelList->update();

    QModelIndex baseIndex = modelView->mapFromSource(modelList->index(pathEdit->currentText()));

    if(currentView == 2)
        treeViewFolders->setRootIndex(baseIndex);
    else
        listViewFileSystem->setRootIndex(baseIndex);

    QTimer::singleShot(50, this, SLOT(dirLoaded()));
}

//---------------------------------------------------------------------------------
bool mainTreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());

    if(fileModel->isDir(index0))
        if(this->filterRegExp().isEmpty() || fileModel->fileInfo(index0).isHidden() == 0)
            return true;

    return false;
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(this->filterRegExp().isEmpty())
        return true;

    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    myModel* fileModel = qobject_cast<myModel*>(sourceModel());

    if(fileModel->fileInfo(index0).isHidden())
        return false;

    return true;
}

//---------------------------------------------------------------------------------
bool viewsSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    myModel* fsModel = dynamic_cast<myModel*>(sourceModel());

    if((fsModel->isDir(left) && !fsModel->isDir(right)))
        return sortOrder() == Qt::AscendingOrder;
    else if(!fsModel->isDir(left) && fsModel->isDir(right))
        return sortOrder() == Qt::DescendingOrder;

    if(left.column() == 1)          //size
    {
        if(fsModel->size(left) > fsModel->size(right))
            return true;
        else
            return false;
    }
    else
    if(left.column() == 3)          //date
    {
        if(fsModel->fileInfo(left).lastModified() > fsModel->fileInfo(right).lastModified())
            return true;
        else
            return false;
    }

    return QSortFilterProxyModel::lessThan(left,right);
}

//---------------------------------------------------------------------------------
QStringList myCompleter::splitPath(const QString& path) const
{
    QStringList parts = path.split("/");
    parts[0] = "/";

    return parts;
}

//---------------------------------------------------------------------------------
QString myCompleter::pathFromIndex(const QModelIndex& index) const
{
    if(!index.isValid())
        return "";

    QModelIndex idx = index;
    QStringList list;
    do
    {
        QString t = model()->data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    }
    while (idx.isValid());

    list[0].clear() ; // the join below will provide the separator

    return list.join("/");
}
