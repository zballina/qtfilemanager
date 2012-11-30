#include "mainwindowfilemanager.h"

#include <qtxdg/xdgicon.h>
#include "viewcontent.h"
#include "thumbnailiconprovider.h"
#include "properties.h"

#define INDEX_LISTVIEW      0
#define INDEX_TREEVIEW      1
#define INDEX_TABLEVIEW     2

MainWindowFileManager::MainWindowFileManager(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindowFileManager()
{
    setupUi(this);

    Properties::Instance()->loadSettings();
    XdgIcon::setThemeName(Properties::Instance()->forceTheme);
    QIcon::setThemeName(XdgIcon::themeName());
    QApplication::setWindowIcon(QIcon::fromTheme("system-file-manager"));

    m_icons = new Icons();
    m_iconProvider = new ThumbnailIconProvider(m_icons);
    setPropertiesTerminal();

    // Creation and open tab initial
    on_actionAddTab_triggered();
}

MainWindowFileManager::~MainWindowFileManager()
{
    Properties::Instance()->saveSettings();
    m_icons->cacheInfo();
    delete m_icons;
    delete m_iconProvider;
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

void MainWindowFileManager::on_actionAddTab_triggered()
{
    ViewContent *view = new ViewContent(
                Properties::Instance()->startPath, m_iconProvider, m_tabWidget);
    connect(view, SIGNAL(onChangeDir(QString)), this, SLOT(onChangeDirCurrentView(QString)));
    connect(view, SIGNAL(onHistoryPrevious(bool)), this, SLOT(onDirPrevious(bool)));
    connect(view, SIGNAL(onHistoryFollow(bool)), this, SLOT(onDirFollow(bool)));

    int newtab = m_tabWidget->addTab(view,
                                   QIcon::fromTheme("system-file-manager"),
                                   view->currentDir());

    m_tabWidget->setCurrentIndex(newtab);
    m_termWidgetTerminal->changeDir(view->currentDir());
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
    m_tabWidget->setTabText(m_tabWidget->currentIndex(), dir);
    m_termWidgetTerminal->changeDir(QString("'%1'").arg(dir));
}

void MainWindowFileManager::onDirPrevious(bool activate)
{
    actionPreviosDir->setEnabled(activate);
}

void MainWindowFileManager::onDirFollow(bool activate)
{

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

void MainWindowFileManager::on_actionPreviosDir_triggered()
{
    ViewContent *current = qobject_cast<ViewContent *> (m_tabWidget->currentWidget());
    current->previousDir();
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
