/****************************************************************************
* This file is part of qtFM, a simple, fast file manager.
* Copyright (C) 2010,2011,2012Wittfella
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

#ifndef BOOKMARKS_CPP
#define BOOKMARKS_CPP

#include <QtGui>
#include "bookmarkmodel.h"
#include "icondlg.h"
#include "mainwindowfilemanager.h"
#include "utils.h"
#include "properties.h"

void MainWindowFileManager::addBookmarkAction()
{
    modelBookmarks->addBookmark(curIndex.fileName(),curIndex.filePath(),"0","");
}

//---------------------------------------------------------------------------
void MainWindowFileManager::addSeparatorAction()
{
    modelBookmarks->addBookmark("","","","");
}

void MainWindowFileManager::mountWatcherTriggered()
{
    QTimer::singleShot(1000,this,SLOT(autoBookmarkMounts()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::autoBookmarkMounts()
{
    QList<QStandardItem *> theBookmarks = modelBookmarks->findItems("*",Qt::MatchWildcard);

    QStringList autoBookmarks;

    foreach(QStandardItem *item, theBookmarks)
    {
        if(item->data(34).toString() == "1")		    //is an automount
            autoBookmarks.append(item->data(32).toString());
    }

    QStringList mtabMounts;
    QFile mtab("/etc/mtab");
    mtab.open(QFile::ReadOnly);
    QTextStream stream(&mtab);
    do mtabMounts.append(stream.readLine());
    while (!stream.atEnd());
    mtab.close();

    QStringList sysMounts = QStringList() << "/dev" << "/sys" << "/pro" << "/tmp" << "/run";
    QStringList dontShowList = Properties::Instance()->settings.value("hideBookmarks",0).toStringList();
    mounts.clear();

    foreach(QString item, mtabMounts)
    if(!sysMounts.contains(item.split(" ").at(1).left(4)))
        {
            QString path = item.split(" ").at(1);
            path.replace("\\040"," ");

            mounts.append(path);
            if(!dontShowList.contains(path))
                if(!autoBookmarks.contains(path))	    //add a new auto bookmark if it doesn't exist
                {
            autoBookmarks.append(path);
                    if(item.split(" ").at(2) == "iso9660") modelBookmarks->addBookmark(path,path,"1","drive-optical");
                    else if(item.split(" ").at(2).contains("fat")) modelBookmarks->addBookmark(path,path,"1","drive-removable-media");
                    else modelBookmarks->addBookmark(path,path,"1","drive-harddisk");
                }
        }

//remove existing automounts that no longer exist
    foreach(QStandardItem *item, theBookmarks)
        if(autoBookmarks.contains(item->data(32).toString()))
            if(!mounts.contains(item->data(32).toString()))
                modelBookmarks->removeRow(item->row());
}

//---------------------------------------------------------------------------
void MainWindowFileManager::delBookmark()
{
    QModelIndexList list = listViewPlaces->selectionModel()->selectedIndexes();

    while(!list.isEmpty())
    {
        if(list.first().data(34).toString() == "1")		//automount, add to dontShowList
        {
            QStringList temp = Properties::Instance()->hideBookmarks;
            temp.append(list.first().data(32).toString());
            Properties::Instance()->hideBookmarks = temp;
        }
        modelBookmarks->removeRow(list.first().row());
        list = listViewPlaces->selectionModel()->selectedIndexes();
    }

}

//---------------------------------------------------------------------------------
void MainWindowFileManager::editBookmark()
{
    icondlg * themeIcons = new icondlg;
    if(themeIcons->exec() == 1)
    {
        QStandardItem * item = modelBookmarks->itemFromIndex(listViewPlaces->currentIndex());
        item->setData(themeIcons->result,33);
        item->setIcon(QIcon::fromTheme(themeIcons->result));
    }
    delete themeIcons;
}

//---------------------------------------------------------------------------
void MainWindowFileManager::toggleWrapBookmarks()
{
    listViewPlaces->setWrapping(wrapBookmarksAct->isChecked());
    Properties::Instance()->wrapBookmarks = wrapBookmarksAct->isChecked();
}

//---------------------------------------------------------------------------
void MainWindowFileManager::bookmarkPressed(QModelIndex current)
{
    if(QApplication::mouseButtons() == Qt::MidButton)
        tabs->setCurrentIndex(addTab(current.data(32).toString()));
}

//---------------------------------------------------------------------------
void MainWindowFileManager::bookmarkClicked(QModelIndex item)
{
    if(item.data(32).toString() == pathEdit->currentText())
        return;

    QString info(item.data(32).toString());
    if(info.isEmpty())
        return;                                  //separator
    if(info.contains("/."))
        modelList->setRootPath(info);       //hidden folders

    treeViewFileSystem->setCurrentIndex(
                modelTree->mapFromSource(modelList->index(item.data(32).toString())));
    labelStatus->setText(getDriveInfo(curIndex.filePath()));
}

#endif
