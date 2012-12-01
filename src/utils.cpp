#include "utils.h"

#include <QtCore/QDir>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <magic.h>

QString getDriveInfo(QString path)
{
    struct statfs info;
    statfs(path.toLocal8Bit(), &info);

    if(info.f_blocks == 0)
        return "";

    return QString("%1  /  %2  (%3%)").arg(formatSize((qint64) (info.f_blocks - info.f_bavail)*info.f_bsize))
                       .arg(formatSize((qint64) info.f_blocks*info.f_bsize))
                       .arg((info.f_blocks - info.f_bavail)*100/info.f_blocks);
}

QString gGetMimeType(QString path)
{
    magic_t cookie = magic_open(MAGIC_MIME);
    magic_load(cookie,0);
    QString temp = magic_file(cookie,path.toLocal8Bit());
    magic_close(cookie);

    return temp.left(temp.indexOf(";"));
}

QString formatSize(qint64 num)
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if (num >= tb)
        total = QString("%1TB").arg(QString::number(qreal(num) / tb, 'f', 2));
    else if(num >= gb)
        total = QString("%1GB").arg(QString::number(qreal(num) / gb, 'f', 2));
    else if(num >= mb)
        total = QString("%1MB").arg(QString::number(qreal(num) / mb, 'f', 1));
    else if(num >= kb)
        total = QString("%1KB").arg(QString::number(qreal(num) / kb,'f', 1));
    else
        total = QString("%1 bytes").arg(num);

    return total;
}

QString currentDirectory(QFileInfo path)
{
    if(path.absoluteFilePath() != "/")
    {
        QStringList split = path.absoluteFilePath().split(QDir::separator());
        return split.at(split.size() - 1);
    }

    return path.absoluteFilePath();
}
