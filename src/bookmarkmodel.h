#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include <QStandardItemModel>

class BookmarkModel : public QStandardItemModel
{
    Q_OBJECT
public:
    BookmarkModel(QHash<QString,QIcon> *);
    QStringList mimeTypes () const;
    bool dropMimeData(const QMimeData * data,Qt::DropAction action,int row,int column,const QModelIndex & parent);
    void addBookmark(QString name, QString path, QString isAuto, QString icon);

signals:
    void bookmarkPaste(const QMimeData * data, QString newPath, QStringList cutList);

private:
    QHash<QString,QIcon> *folderIcons;
};

#endif // BOOKMARKMODEL_H
