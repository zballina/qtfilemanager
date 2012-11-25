#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QString>

QString getDriveInfo(QString path);
QString gGetMimeType(QString path);
QString formatSize(qint64 num);

#endif // UTILS_H
