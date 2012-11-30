#include "icons.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QBuffer>
#include <QtGui/QFileIconProvider>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QPalette>
#include <QtGui/QImageWriter>
#include <QtCore/QDir>
#include <qtxdg/xdgmime.h>
#include <magic.h>

#define FILE_CACHE      "/.config/qtfilemanager/file.cache"
#define FOLDER_CACHE    "/.config/qtfilemanager/folder.cache"
#define THUMBS_CACHE    "/.config/qtfilemanager/thumbs.cache"

Icons::Icons()
{
    mimeIcons = new QHash<QString, QIcon>;
    folderIcons = new QHash<QString, QIcon>;
    icons = new QCache<QString, QIcon>;
    mimeGeneric = new QHash<QString, QString>;
    mimeGlob = new QHash<QString, QString>;

    icons->setMaxCost(500);

    loadMimeTypes();

    QFile fileIcons(QDir::homePath() + FILE_CACHE);
    fileIcons.open(QIODevice::ReadOnly);
    QDataStream out(&fileIcons);
    out >> *mimeIcons;
    fileIcons.close();

    fileIcons.setFileName(QDir::homePath() + FOLDER_CACHE);
    fileIcons.open(QIODevice::ReadOnly);
    out.setDevice(&fileIcons);
    out >> *folderIcons;
    fileIcons.close();
}

QString Icons::getMimeType(QString path)
{
    magic_t cookie = magic_open(MAGIC_MIME);
    magic_load(cookie, 0);
    QString temp = magic_file(cookie, path.toLocal8Bit());
    magic_close(cookie);

    qDebug() << "Magic" << temp;

    return temp.left(temp.indexOf(";"));
}

void Icons::loadMimeTypes() const
{
    QFile mimeInfo("/usr/share/mime/globs");
    mimeInfo.open(QIODevice::ReadOnly);
    QTextStream out(&mimeInfo);

    do
    {
        QStringList line = out.readLine().split(":");
        if(line.count() == 2)
        {
            QString suffix = line.at(1);
            suffix.remove("*.");
            QString mimeName = line.at(0);
            mimeName.replace("/", "-");
            mimeGlob->insert(suffix, mimeName);
        }
    }
    while (!out.atEnd());

    mimeInfo.close();

    mimeInfo.setFileName("/usr/share/mime/generic-icons");
    mimeInfo.open(QIODevice::ReadOnly);
    out.setDevice(&mimeInfo);

    do
    {
        QStringList line = out.readLine().split(":");
        if(line.count() == 2)
        {
            QString mimeName = line.at(0);
            mimeName.replace("/","-");
            QString icon = line.at(1);
            mimeGeneric->insert(mimeName, icon);
        }
    }
    while (!out.atEnd());

    mimeInfo.close();
}

void Icons::cacheInfo()
{
    QFile fileIcons(QDir::homePath() + FILE_CACHE);
    fileIcons.open(QIODevice::WriteOnly);
    QDataStream out(&fileIcons);
    out << *mimeIcons;
    fileIcons.close();

    fileIcons.setFileName(QDir::homePath() + FOLDER_CACHE);
    fileIcons.open(QIODevice::WriteOnly);
    out.setDevice(&fileIcons);
    out << *folderIcons;
    fileIcons.close();
}

QIcon Icons::icon(const QFileInfo &file)
{
    QIcon theIcon;

    if(file.isDir())
    {
        if(folderIcons->contains(file.absoluteFilePath()))
        {
            return folderIcons->value(file.absoluteFilePath());
        }
        else if(file.absoluteFilePath() == QDir::homePath())
        {
            theIcon = QIcon::fromTheme("user-home");
            folderIcons->insert(file.absoluteFilePath(), theIcon);
            return theIcon;
        }

        QFileIconProvider iconFactory;
        folderIcons->insert(file.absoluteFilePath(), iconFactory.icon(file));

        return iconFactory.icon(file);
    }
    else
    {
        if(icons->contains(file.absoluteFilePath()))
        {
            return *icons->object(file.absoluteFilePath());
        }

        if(file.isExecutable())
        {
            QIcon *icon = new QIcon(QIcon::fromTheme("application-x-executable"));
            icons->insert(file.absoluteFilePath(), icon, 1);
            return *icon;
        }

        XdgMimeInfo mime(file);

        if(mime.iconName() == "unknown" && file.absoluteFilePath().length() > 0)
        {
            QString suffix = file.suffix();
            if(mimeIcons->contains(suffix))
                return mimeIcons->value(suffix);

            if(suffix.isEmpty())
            {
                if(file.isExecutable())
                    suffix = "exec";
                else
                    suffix = "none";

                if(mimeIcons->contains(suffix))
                    return mimeIcons->value(suffix);

                theIcon = QIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon));
                if(suffix == "exec")
                    theIcon = QIcon::fromTheme("application-x-executable", theIcon);
            }
            else
            {
                if(mimeGlob->count() == 0)
                    loadMimeTypes();

                //try mimeType as it is
                QString mimeType = mimeGlob->value(file.suffix().toLower());
                if(QIcon::hasThemeIcon(mimeType))
                    theIcon = QIcon::fromTheme(mimeType);
                else
                {
                    //try matching generic icon
                    if(QIcon::hasThemeIcon(mimeGeneric->value(mimeType)))
                        theIcon = QIcon::fromTheme(mimeGeneric->value(mimeType));
                    else
                    {
                        //last resort try adding "-x-generic" to base type
                        if(QIcon::hasThemeIcon(mimeType.split("-").at(0) + "-x-generic"))
                            theIcon = QIcon::fromTheme(mimeType.split("-").at(0) + "-x-generic");
                        else
                            theIcon = QIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon));
                    }
                }
            }

            mimeIcons->insert(suffix, theIcon);
        }
        else if(mime.iconName() != "unknown")
        {
            mimeIcons->insert(file.suffix(), mime.icon());
            return mime.icon();
        }
    }
    return theIcon;
}
