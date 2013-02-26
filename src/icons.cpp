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
#include <qtxdg/xdgicon.h>
#include <magic.h>

#define FILE_CACHE      "/.config/qtfilemanager/file.cache"
#define FOLDER_CACHE    "/.config/qtfilemanager/folder.cache"

Icons::Icons()
{
    mimeIcons = new QHash<QString, QIcon>;
    folderIcons = new QHash<QString, QIcon>;
    mimeGeneric = new QHash<QString, QString>;
    mimeGlob = new QHash<QString, QString>;

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

    qDebug() << path << "Magic" << temp;

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
    QIcon theIcon = QIcon::fromTheme("user-home");;

    if(file.isDir())
    {
        if(folderIcons->contains(file.absoluteFilePath()))
        {
            return folderIcons->value(file.absoluteFilePath());
        }
        else if(file.absoluteFilePath() == QDir::homePath())
        {
            folderIcons->insert(file.absoluteFilePath(), theIcon);
            return theIcon;
        }

        QFileIconProvider iconFactory;
        folderIcons->insert(file.absoluteFilePath(), iconFactory.icon(file));

        return iconFactory.icon(file);
    }
    else if(file.suffix() > 0)
    {
        QString suffix = file.suffix().toLower();

        if(mimeIcons->contains(suffix))
        {
            return mimeIcons->value(suffix);
        }
        //try mimeType as it is

        if(mimeGlob->count() == 0)
            loadMimeTypes();

        QString mimeType = mimeGlob->value(suffix);
        if(QIcon::hasThemeIcon(mimeType))
        {
            theIcon = QIcon::fromTheme(mimeType);
            mimeIcons->insert(suffix, theIcon);
            return theIcon;
        }
        else
        {
            //try matching generic icon
            if(QIcon::hasThemeIcon(mimeGeneric->value(mimeType)))
                theIcon = QIcon::fromTheme(mimeGeneric->value(mimeType));
            else
            {
                //last resort try adding "-x-generic" to base type
                if(QIcon::hasThemeIcon(mimeType.split("-").at(0) + "-x-generic"))
                {
                    theIcon = QIcon::fromTheme(mimeType.split("-").at(0) + "-x-generic");
                    mimeIcons->insert(suffix, theIcon);
                    return theIcon;
                }
                else
                {
                    /*
                    XdgMimeInfo mime = XdgMimeInfo(mimeType);
                    if(mime.iconName() != "unknown")
                    {
                        mimeIcons->insert(suffix, mime.icon());
                        return mime.icon();
                    }
                    mime = XdgMimeInfo(file);
                    if(mime.iconName() != "unknown")
                    {
                        mimeIcons->insert(suffix, mime.icon());
                        return mime.icon();
                    }
                    else
                    {
                        if(QIcon::hasThemeIcon(mimeType.split("-").at(0) + "-x-generic"))
                            theIcon = QIcon::fromTheme(mimeType.split("-").at(0) + "-x-generic");
                        else
                            theIcon = QIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon));
                    }
                    */
                }
            }
        }

        mimeIcons->insert(suffix, theIcon);
    }
    else if(file.isSymLink())
    {
        return icon(QFileInfo(file.symLinkTarget()));
    }
    else if(!file.isExecutable())
    {
        if(mimeIcons->contains(file.absoluteFilePath()))
        {
            return mimeIcons->value(file.absoluteFilePath());
        }

        XdgMimeInfo mime = XdgMimeInfo(file);
        if(mime.iconName() != "unknown")
        {
            mimeIcons->insert(file.absoluteFilePath(), mime.icon());
            return mime.icon();
        }
        else
        {
            QString mimeType = getMimeType(file.absoluteFilePath());
            if(QIcon::hasThemeIcon(mimeType))
            {
                theIcon = QIcon::fromTheme(mimeType);
                mimeIcons->insert(file.absoluteFilePath(), theIcon);
                return theIcon;
            }
            else if(QIcon::hasThemeIcon(mimeType.split("-").at(0) + "-x-generic"))
            {
                theIcon = QIcon::fromTheme(mimeType.split("-").at(0) + "-x-generic");
                mimeIcons->insert(file.absoluteFilePath(), theIcon);
                return XdgMimeInfo(file).icon();
            }
        }
    }
    else if(file.isExecutable())
    {
        QIcon icon = QIcon::fromTheme("application-x-executable");
         return icon;
    }

    return theIcon;
}
