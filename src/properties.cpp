/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qtermwidget.h>

#include "properties.h"

Properties * Properties::m_instance = 0;

Properties * Properties::Instance()
{
    if (!m_instance)
        m_instance = new Properties();
    return m_instance;
}

Properties::Properties()
{
    qDebug("Properties constructor called");
}

Properties::~Properties()
{
    qDebug("Properties destructor called");
    saveSettings();
    delete m_instance;
    m_instance = 0;
}

QFont Properties::defaultFont()
{
    QFont default_font = QApplication::font();
    default_font.setFamily("Monospace");
    default_font.setPointSize(10);
    default_font.setStyleHint(QFont::TypeWriter);

    return default_font;
}

void Properties::loadSettings()
{
    settings.beginGroup("File Manager");
    realMimeTypes = settings.value("realMimeTypes", true).toBool();
    forceTheme = settings.value("forceTheme", "oxygen").toString();
    windowState = settings.value("windowState").toByteArray();
    windowSize = settings.value("size", QSize(600, 400)).toSize();
    wrapBookmarks = settings.value("wrapBookmarks", false).toBool();
    lockLayout = settings.value("lockLayout", false).toBool();
    zoom = settings.value("zoom", 48).toInt();
    zoomTree = settings.value("zoomTree",16).toInt();
    zoomList = settings.value("zoomList",24).toInt();
    zoomDetail = settings.value("zoomDetail",16).toInt();
    showThumbs = settings.value("showThumbs", true).toBool();
    viewMode = settings.value("viewMode", false).toBool();
    iconMode = settings.value("iconMode", false).toBool();
    hiddenMode = settings.value("hiddenMode", false).toBool();
    header = settings.value("header").toByteArray();
    singleClick = settings.value("singleClick", false).toBool();
    tabsOnTop = settings.value("tabsOnTop", true).toBool();
    hideBookmarks = settings.value("hideBookmarks").toStringList();
    confirmDelete = settings.value("confirmDelete", true).toBool();
    appOpacity = settings.value("Opacity", 100).toInt();
    daemon = settings.value("daemon", false).toBool();
    startPath = settings.value("startPath", QDir::homePath()).toString();
    settings.endGroup();

    settings.beginGroup("Terminal");
    colorScheme = settings.value("colorScheme", "Linux").toString();
    font = qvariant_cast<QFont>(settings.value("font", defaultFont()));
    historyLimited = settings.value("HistoryLimited", true).toBool();
    historyLimitedTo = settings.value("HistoryLimitedTo", 1000).toUInt();

    emulation = settings.value("emulation", "default").toString();

    // sessions
    int size = settings.beginReadArray("Sessions");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString name(settings.value("name").toString());
        if (name.isEmpty())
            continue;
        sessions[name] = settings.value("state").toByteArray();
    }
    settings.endArray();

    termOpacity = settings.value("termOpacity", 100).toInt();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = settings.value("ScrollbarPosition", 2).toInt();

    settings.endGroup();
    settings.beginGroup("Shortcuts");
    QStringList keys = settings.childKeys();
    foreach( QString key, keys )
    {
        QKeySequence sequence = QKeySequence( settings.value( key ).toString() );
        if( Properties::Instance()->actions.contains( key ) )
            Properties::Instance()->actions[ key ]->setShortcut( sequence );
    }
    settings.endGroup();
}

void Properties::saveSettings()
{
    settings.beginGroup("File Manager");
    settings.setValue("realMimeTypes", realMimeTypes);
    settings.setValue("forceTheme", forceTheme);
    settings.setValue("windowState", windowState);
    settings.setValue("windowSize", windowSize);
    settings.setValue("wrapBookmarks", wrapBookmarks);
    settings.setValue("lockLayout", lockLayout);
    settings.setValue("zoom", zoom);
    settings.setValue("zoomTree", zoomTree);
    settings.setValue("zoomList", zoomList);
    settings.setValue("zoomDetail", zoomDetail);
    settings.setValue("showThumbs", showThumbs);
    settings.setValue("viewMode", viewMode);
    settings.setValue("iconMode", iconMode);
    settings.setValue("hiddenMode", hiddenMode);
    settings.setValue("header", header);
    settings.setValue("singleClick", singleClick);
    settings.setValue("tabsOnTop", tabsOnTop);
    settings.setValue("hideBookmarks", hideBookmarks);
    settings.setValue("confirmDelete", confirmDelete);
    settings.setValue("Opacity", appOpacity);
    settings.setValue("daemon", daemon);
    settings.setValue("startPath", startPath);
    settings.endGroup();

    settings.beginGroup("Terminal");
    settings.setValue("colorScheme", colorScheme);
    settings.setValue("font", font);
    settings.setValue("HistoryLimited", historyLimited);
    settings.setValue("HistoryLimitedTo", historyLimitedTo);

    settings.setValue("emulation", emulation);

    // sessions
    settings.beginWriteArray("Sessions");
    int i = 0;
    Sessions::iterator sit = sessions.begin();
    while (sit != sessions.end())
    {
        settings.setArrayIndex(i);
        settings.setValue("name", sit.key());
        settings.setValue("state", sit.value());
        ++sit;
        ++i;
    }
    settings.endArray();

    settings.setValue("Opacity", termOpacity);
    settings.setValue("ScrollbarPosition", scrollBarPos);
    settings.endGroup();

    settings.beginGroup("Shortcuts");
    QMapIterator< QString, QAction * > it(actions);
    while( it.hasNext() )
    {
        it.next();
        QKeySequence shortcut = it.value()->shortcut();
        settings.setValue( it.key(), shortcut.toString() );
    }
    settings.endGroup();
}
