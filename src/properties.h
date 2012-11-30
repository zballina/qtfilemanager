#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtGui/QFont>
#include <QtGui/QAction>

typedef QString Session;
typedef QMap<QString,Session> Sessions;
typedef QMap<QString,QString> ShortcutMap;

class Properties
{
    public:
        static Properties *Instance();

        void saveSettings();
        void loadSettings();
        QFont defaultFont();

        // File manager
        QString forceTheme;
        bool realMimeTypes;
        QByteArray windowState;
        QSize windowSize;
        bool wrapBookmarks;
        bool lockLayout;
        int zoom;
        int zoomTree;
        int zoomList;
        int zoomDetail;
        bool showThumbs;
        bool viewMode;
        bool iconMode;
        bool hiddenMode;
        QByteArray header;
        bool singleClick;
        bool tabsOnTop;
        QStringList hideBookmarks;
        bool confirmDelete;
        bool daemon;
        int appOpacity;
        QString startPath;

        // Docks
        int heightTerminal;

        // Bookmarks
        // Group and childkeys

        // Terminal
        QString shell;
        QFont font;
        QString colorScheme;
        bool historyLimited;
        unsigned historyLimitedTo;
        QString emulation;
        Sessions sessions;
        int termOpacity;
        int scrollBarPos;
        QMap< QString, QAction * > actions;

    private:
        // Singleton handling
        static Properties *m_instance;

        Properties();
        Properties(const Properties &) {}
        ~Properties();
};

#endif // PROPERTIES_H
