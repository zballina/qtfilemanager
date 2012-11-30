#ifndef THUMBNAILICONPROVIDER_H
#define THUMBNAILICONPROVIDER_H

#include <QtGui/QFileIconProvider>
#include "icons.h"

class ThumbnailIconProvider : public QFileIconProvider
{
//    Q_OBJECT
public:
    ThumbnailIconProvider(Icons *icons);
    ~ThumbnailIconProvider();
    QIcon icon(const QFileInfo & info) const;

private:
    Icons *m_icons;
};

#endif // THUMBNAILICONPROVIDER_H
