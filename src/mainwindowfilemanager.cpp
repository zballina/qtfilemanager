#include "mainwindowfilemanager.h"

#include <qtxdg/xdgicon.h>
#include "viewcontent.h"
#include "thumbnailiconprovider.h"
#include "addressbar.h"
#include "properties.h"
#include "utils.h"

#define INDEX_LISTVIEW      0
#define INDEX_TREEVIEW      1
#define INDEX_TABLEVIEW     2

MainWindowFileManager::MainWindowFileManager(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindowFileManager()
{
    setupUi(this);

    Properties::Instance()->loadSettings();
    setTheme();
    QApplication::setWindowIcon(QIcon::fromTheme("system-file-manager",
                                                 QIcon(":images/system-file-manager")));

    m_icons = new Icons();
    m_iconProvider = new ThumbnailIconProvider(m_icons);
    setPropertiesTerminal();
    createAddressBar();
    // Creation and open tab initial
    on_actionAddTab_triggered();
}

MainWindowFileManager::~MainWindowFileManager()
{
    Properties::Instance()->saveSettings();
    m_icons->cacheInfo();
    delete m_icons;
    delete m_iconProvider;
    delete m_addresBar;
}

void MainWindowFileManager::setTheme()
{
    QString temp = Properties::Instance()->forceTheme;
    if(temp.isNull())
    {
        //get theme from system (works for gnome/kde)
        temp = QIcon::themeName();

        //Qt doesn't detect the theme very well for non-DE systems,
        //so try reading the '~/.gtkrc-2.0' or '~/.config/gtk-3.0/settings.ini'

        if(temp == "hicolor")
        {
            //check for gtk-2.0 settings
            if(QFile::exists(QDir::homePath() + "/" + ".gtkrc-2.0"))
            {
                QSettings gtkFile(QDir::homePath() + "/.gtkrc-2.0", QSettings::IniFormat, this);
                temp = gtkFile.value("gtk-icon-theme-name").toString().remove("\"");
            }
            else
            {
                //try gtk-3.0
                QSettings gtkFile(QDir::homePath() + "/.config/gtk-3.0/settings.ini",
                                  QSettings::IniFormat, this);
                temp = gtkFile.value("gtk-fallback-icon-theme").toString().remove("\"");
            }

            //fallback
            if(temp.isNull())
            {
                if(QFile::exists("/usr/share/icons/gnome"))
                    temp = "gnome";
                else if(QFile::exists("/usr/share/icons/oxygen"))
                    temp = "oxygen";
                else
                    temp = "hicolor";

                Properties::Instance()->forceTheme = temp;
            }
        }
    }

    XdgIcon::setThemeName(temp);
}

void MainWindowFileManager::setPropertiesTerminal()
{
    m_dockWidgetTerminal->setMinimumHeight(Properties::Instance()->heightTerminal);
    m_termWidgetTerminal->startShellProgram();
    m_termWidgetTerminal->setColorScheme(Properties::Instance()->colorScheme);
    m_termWidgetTerminal->setShellProgram(Properties::Instance()->shell);
    m_termWidgetTerminal->setKeyBindings(Properties::Instance()->emulation);
    m_termWidgetTerminal->setTerminalOpacity(Properties::Instance()->termOpacity/100.0);
    m_termWidgetTerminal->setTerminalFont(Properties::Instance()->font);

    switch(Properties::Instance()->scrollBarPos)
    {
    case 0:
        m_termWidgetTerminal->setScrollBarPosition(QTermWidget::NoScrollBar);
        break;
    case 1:
        m_termWidgetTerminal->setScrollBarPosition(QTermWidget::ScrollBarLeft);
        break;
    case 2:
    default:
        m_termWidgetTerminal->setScrollBarPosition(QTermWidget::ScrollBarRight);
        break;
    }

    if (Properties::Instance()->historyLimited)
    {
        m_termWidgetTerminal->setHistorySize(Properties::Instance()->historyLimitedTo);
    }
    else
    {
        // Unlimited history
        m_termWidgetTerminal->setHistorySize(-1);
    }
}

void MainWindowFileManager::createAddressBar()
{
    m_addresBar = new AddressBar(QFileInfo(Properties::Instance()->startPath), this);
    connect(m_addresBar, SIGNAL(onClickDirectory(QFileInfo)),
            this, SLOT(onClickedDirectoryAddressBar(QFileInfo)));
    m_toolBarNavigation->addWidget(m_addresBar);
}

void MainWindowFileManager::onClickedDirectoryAddressBar(QFileInfo info)
{
    qDebug() << "In MainWindowFileManager";
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    current->changeDir(info.absoluteFilePath());
}

void MainWindowFileManager::on_actionAddTab_triggered()
{
    ViewContent *view = new ViewContent(
                Properties::Instance()->startPath, m_iconProvider, m_tabWidget);
    connect(view, SIGNAL(onChangeDir(QString)), this, SLOT(onChangeDirCurrentView(QString)));
    connect(view, SIGNAL(onHistoryPrevious(bool)), this, SLOT(onPreviousDir(bool)));
    connect(view, SIGNAL(onHistoryNext(bool)), this, SLOT(onNextDir(bool)));

    int newtab = m_tabWidget->addTab(view,
                                   QIcon::fromTheme("system-file-manager"),
                                   view->currentDir());

    m_tabWidget->setCurrentIndex(newtab);
    m_termWidgetTerminal->changeDir(view->currentDirAbsolutePath());
}

void MainWindowFileManager::onChangeDirCurrentView(QString dir)
{
    if(dir == "/")
    {
        actionUpDir->setEnabled(false);
    }
    else if(!actionUpDir->isEnabled())
    {
        actionUpDir->setEnabled(true);
    }
    m_addresBar->changeAddress(dir);
    m_tabWidget->setTabText(m_tabWidget->currentIndex(), currentDirectory(QFileInfo(dir)));
    m_termWidgetTerminal->changeDir(QString("'%1'").arg(dir));
}

void MainWindowFileManager::onPreviousDir(bool activate)
{
    actionGoPreviousDir->setEnabled(activate);
}

void MainWindowFileManager::onNextDir(bool activate)
{
    actionGoNextDir->setEnabled(activate);
}

void MainWindowFileManager::on_actionIconView_triggered()
{
    if(actionIconView->isChecked())
    {
        ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
        current->stackedWidget->setCurrentIndex(INDEX_LISTVIEW);
        current->m_listView->setViewMode(QListView::IconMode);
    }
    else
    {
        actionIconView->setChecked(true);
    }
    actionListView->setChecked(false);
    actionTreeView->setChecked(false);
    actionDetailsView->setChecked(false);
}

void MainWindowFileManager::on_actionListView_triggered()
{
    if(actionListView->isChecked())
    {
        ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
        current->stackedWidget->setCurrentIndex(INDEX_LISTVIEW);
        current->m_listView->setViewMode(QListView::ListMode);
    }
    else
    {
        actionListView->setChecked(true);
    }
    actionIconView->setChecked(false);
    actionTreeView->setChecked(false);
    actionDetailsView->setChecked(false);
}

void MainWindowFileManager::on_actionTreeView_triggered()
{
    if(actionTreeView->isChecked())
    {
        ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
        current->stackedWidget->setCurrentIndex(INDEX_TREEVIEW);
    }
    else
    {
        actionTreeView->setChecked(true);
    }
    actionListView->setChecked(false);
    actionIconView->setChecked(false);
    actionDetailsView->setChecked(false);
}

void MainWindowFileManager::on_actionDetailsView_triggered()
{
    if(actionDetailsView->isChecked())
    {
        ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
        current->stackedWidget->setCurrentIndex(INDEX_TABLEVIEW);
    }
    else
    {
        actionDetailsView->setChecked(true);
    }
    actionTreeView->setChecked(false);
    actionListView->setChecked(false);
    actionIconView->setChecked(false);
}

void MainWindowFileManager::on_actionGoHome_triggered()
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    current->changeDir();
}

void MainWindowFileManager::on_actionGoPreviousDir_triggered()
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    qDebug() << "invoke pre dir in MainWindow";
    current->previousDir();
}

void MainWindowFileManager::on_actionGoNextDir_triggered()
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    qDebug() << "invoke next dir in MainWindow";
    current->nextDir();
}

void MainWindowFileManager::on_actionCloseTab_triggered()
{
    int index = m_tabWidget->currentIndex();
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    delete current;
    m_tabWidget->removeTab(index);
}

void MainWindowFileManager::on_actionUpDir_triggered()
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    current->upDir();
}

void MainWindowFileManager::on_m_tabWidget_tabCloseRequested(int index)
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->widget(index));
    delete current;
    m_tabWidget->removeTab(index);
}

void MainWindowFileManager::on_actionTerminal_triggered()
{
    m_dockWidgetTerminal->setVisible(!m_dockWidgetTerminal->isVisible());
}

void MainWindowFileManager::on_m_tabWidget_currentChanged(int index)
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->widget(index));
    actionGoNextDir->setEnabled(current->hasNextDir());
    actionGoPreviousDir->setEnabled(current->hasPreviousDir());
}
