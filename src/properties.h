#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtCore>
#include <QFont>
#include <QAction>

typedef QString Session;

typedef QMap<QString,Session> Sessions;

typedef QMap<QString,QString> ShortcutMap;


class Properties
{
    public:
        static Properties *Instance();

        void saveSettings();
        void loadSettings();


        QString shell;
        QFont font;
        QString colorScheme;

        bool historyLimited;
        unsigned historyLimitedTo;

        QString emulation;

        Sessions sessions;

        int appOpacity;
        int termOpacity;
        int scrollBarPos;

        QKeySequence dropShortCut;
        bool dropKeepOpen;
        bool dropShowOnStart;
        int dropWidht;
        int dropHeight;

        QMap< QString, QAction * > actions;

    private:

        // Singleton handling
        static Properties *m_instance;

        Properties();
        Properties(const Properties &) {}
        ~Properties();

};

#endif // PROPERTIES_H
