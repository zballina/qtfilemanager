#include "filesystemmodel.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#define BLOCK_SIZE      4096

FileSystemModel::FileSystemModel(QObject *parent) :
    QFileSystemModel(parent)
{
}

bool FileSystemModel::copyFolder(QString source, QString target, qint64 total, bool cut)
{
    QDir sourceFolder(source);
    QDir targetFolder(QFileInfo(target).path());
    QString folderName = QFileInfo(target).fileName();

    bool ok = true;

    if(!QFileInfo(target).exists())
        targetFolder.mkdir(folderName);

    targetFolder = QDir(target);

    QStringList files = sourceFolder.entryList(QDir::Files
                                               | QDir::NoDotAndDotDot | QDir::Hidden);

    for(int i = 0; i < files.count(); i++)
    {
        QString srcName = sourceFolder.path() + "/" + files[i];
        QString destName = targetFolder.path() + "/" + files[i];
        if(!copyFile(srcName, destName, total, cut))
            ok = false;     //don't remove source folder if all files not cut

        if(m_cancel == true)
            return false;                           //cancelled
    }

    files.clear();
    files = sourceFolder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Hidden);

    for(int i = 0; i < files.count(); i++)
    {
        if(m_cancel)
            return false;    //cancelled

        QString srcName = sourceFolder.path() + "/" + files[i];
        QString destName = targetFolder.path() + "/" + files[i];
        copyFolder(srcName, destName, total, cut);
    }

    //remove source folder if all files moved ok
    if(cut && ok)
        sourceFolder.rmdir(source);
    return ok;
}

bool FileSystemModel::copyFile(QString source, QString target, qint64 totalSize, bool cut)
{
    QFile fileSource(source);
    QFile fileTarget(target);

    fileSource.open(QFile::ReadOnly);
    fileTarget.open(QFile::WriteOnly);

    char block[BLOCK_SIZE];
    qint64 total = fileSource.size();
    qint64 steps = total >> 7;        //shift right 7, same as divide 128, much faster
    qint64 bytesCopied = 0;

    while(!fileSource.atEnd())
    {
        if(m_cancel)
            break;

        qint64 inBytes = fileSource.read(block, sizeof(block));
        fileTarget.write(block, inBytes);
        bytesCopied += inBytes;

        if(bytesCopied > steps)
        {
            emit updateCopyProgress(target, bytesCopied, totalSize);
            bytesCopied = 0;
        }
    }

    emit updateCopyProgress(target, bytesCopied, totalSize);

    fileTarget.close();
    fileSource.close();

    if(fileTarget.size() != total)
        return false;

    if(cut)
        fileSource.remove();  //if file is cut remove the source

    return true;
}

void FileSystemModel::cancellationNotice(bool cancel)
{
    m_cancel = cancel;
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case Qt::ToolTipRole:
        return QFileSystemModel::data(index, Qt::DisplayRole).toString();
    }

    return QFileSystemModel::data(index, role);
}

/*
void FileSystemModel::copy(QModelIndex source, QModelIndex target)
{

}
*/
