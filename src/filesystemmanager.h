#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class FileSystemManager : public QObject
{
    Q_OBJECT
public:

    FileSystemManager();
    bool copy(QString source, QString target);

public slots:
    void cancellationNotice(bool cancel);

signals:
    void finishCopy(bool sucess);
    void updateCopyProgress(QString target, qint64 bytesCopied, qint64 totalSize);

private:
    bool copyFile(QString source, QString target, qint64 totalSize, bool cut);
    bool copyFolder(QString source, QString target, qint64 totalSize, bool cut);

    bool m_cancel;
};

#endif // FILESYSTEMMANAGER_H
