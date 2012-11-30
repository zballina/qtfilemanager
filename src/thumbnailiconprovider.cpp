#include "thumbnailiconprovider.h"

#include <QtCore/QDebug>

ThumbnailIconProvider::ThumbnailIconProvider(Icons *icons)
    : m_icons(icons)
{
}

ThumbnailIconProvider::~ThumbnailIconProvider()
{
}

QIcon ThumbnailIconProvider::icon(const QFileInfo & info) const
{
    return m_icons->icon(info);
}
