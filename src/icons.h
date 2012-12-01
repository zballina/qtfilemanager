#ifndef ICONS_H
#define ICONS_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QCache>
#include <QtGui/QIcon>
#include <QtCore/QFileInfo>
#include <QtGui/QFileSystemModel>

class Icons
{
public:
    Icons();

    QIcon icon(const QFileInfo &file);
    void cacheInfo();

private:
    QString getMimeType(QString path);
    void loadMimeTypes() const;

    QHash<QString, QIcon> *mimeIcons;
    QHash<QString, QIcon> *folderIcons;

    QHash<QString, QString> *mimeGlob;
    QHash<QString, QString> *mimeGeneric;
};

#endif // ICONS_H
