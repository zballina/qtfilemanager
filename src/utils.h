#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QString>
#include <QtCore/QFileInfo>

QString getDriveInfo(QString path);
QString gGetMimeType(QString path);
QString formatSize(qint64 num);
QString currentDirectory(QFileInfo path);

#endif // UTILS_H
