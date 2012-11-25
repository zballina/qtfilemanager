#include "bookmarkmodel.h"

#include <QtCore/QFileInfo>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>

BookmarkModel::BookmarkModel(QHash<QString,QIcon> * icons)
{
    folderIcons = icons;
}

//---------------------------------------------------------------------------
void BookmarkModel::addBookmark(QString name, QString path, QString isAuto, QString icon)
{
    if(path.isEmpty())	    //add seperator
    {
        QStandardItem *item = new QStandardItem(QIcon::fromTheme(icon),"");
        item->setData(QBrush(QPixmap(":/images/sep.png")),Qt::BackgroundRole);
        QFlags<Qt::ItemFlag> flags = item->flags();
        flags ^= Qt::ItemIsEditable;                    //not editable
        item->setFlags(flags);
        item->setFont(QFont("sans",8));                 //force size to prevent 2 rows of background tiling
        this->appendRow(item);
        return;
    }

    QIcon theIcon;
    theIcon = QIcon::fromTheme(icon, QApplication::style()->standardIcon(QStyle::SP_DirIcon));

    if(icon.isEmpty())
        if(folderIcons->contains(name))
            theIcon = folderIcons->value(name);

    if(name.isEmpty())
        name = "/";
    QStandardItem *item = new QStandardItem(theIcon,name);
    item->setData(path,32);
    item->setData(icon,33);
    item->setData(isAuto,34);
    this->appendRow(item);
}

QStringList BookmarkModel::mimeTypes() const
{
    return QStringList() << "application/x-qstandarditemmodeldatalist" << "text/uri-list";
}

//---------------------------------------------------------------------------------
bool BookmarkModel::dropMimeData(const QMimeData * data,Qt::DropAction action,int row,int column,const QModelIndex & parent )
{
    //moving its own items around
    if(data->hasFormat("application/x-qstandarditemmodeldatalist"))
    if(parent.column() == -1)
        return QStandardItemModel::dropMimeData(data,action,row,column,parent);


    QList<QUrl> files = data->urls();
    QStringList cutList;

    foreach(QUrl path, files)
    {
        QFileInfo file(path.toLocalFile());

        //drag to bookmark window, add a new bookmark
        if(parent.column() == -1)
        {
            if(file.isDir())
                this->addBookmark(file.fileName(), file.filePath(), 0, "");
            return false;
        }
        else
            if(action == 2)                             //cut
                cutList.append(file.filePath());
    }

    emit bookmarkPaste(data, parent.data(32).toString(), cutList);

    return false;
}
