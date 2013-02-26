#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    FileSystemModel(QObject *parent = 0);
    //bool copy(QModelIndex source, QModelIndex target);
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void cancellationNotice(bool cancel);

signals:
    void updateCopyProgress(QString target, qint64 bytesCopied, qint64 totalSize);

private:
    bool copyFile(QString source, QString target, qint64 totalSize, bool cut);
    bool copyFolder(QString source, QString target, qint64 totalSize, bool cut);

    bool m_cancel;
};

#endif // FILESYSTEMMODEL_H
