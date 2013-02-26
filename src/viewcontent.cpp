#include "viewcontent.h"

#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QItemSelectionModel>
#include "properties.h"
#include "utils.h"

#define INDEX_LISTVIEW      0
#define INDEX_ICONVIEW      1
#define INDEX_TREEVIEW      2
#define INDEX_TABLEVIEW     3

ViewContent::ViewContent(QString startpath, ThumbnailIconProvider *icons, QWidget *parent) :
    QWidget(parent), Ui::ViewContent(),
    m_model(new FileSystemModel()), m_currentDir(startpath),
    m_iconProvider(icons),
    m_historyPrevious(new QStack<QString>()),
    m_historyNext(new QStack<QString>())
{
    setupUi(this);

    m_addresBar = new AddressBar(QFileInfo(m_currentDir), this);
    verticalLayout->insertWidget(0, m_addresBar);
    connect(m_addresBar, SIGNAL(onClickDirectory(QFileInfo)),
            this, SLOT(onClickedDirectoryAddressBar(QFileInfo)));

    m_model->setIconProvider(m_iconProvider);
    connect(m_model, SIGNAL(rootPathChanged(QString)),
            this, SLOT(onView_changeDir(QString)));

    m_model->setResolveSymlinks(true);

    m_listView->setModel(m_model);
    m_iconView->setModel(m_model);
    m_treeView->setModel(m_model);
    m_tableView->setModel(m_model);

    connectClickedModelIndex();

    m_model->setRootPath(m_currentDir);
}

ViewContent::~ViewContent()
{
    delete m_model;
    delete m_historyNext;
    delete m_historyPrevious;
}

void ViewContent::onClickedDirectoryAddressBar(QFileInfo info)
{
    changeDir(info.absoluteFilePath());
}

void ViewContent::connectClickedModelIndex()
{
    if(Properties::Instance()->singleClick)
    {
        connect(m_listView, SIGNAL(clicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_iconView, SIGNAL(clicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_treeView, SIGNAL(clicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_tableView, SIGNAL(clicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
    }
    else
    {
        connect(m_listView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_iconView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_tableView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
    }
}

QString ViewContent::currentDir()
{
    return currentDirectory(QFileInfo(m_currentDir));
}

QString ViewContent::currentDirAbsolutePath()
{
    return m_currentDir;
}

void ViewContent::onView_changeDir(QString path)
{
    m_currentDir = path;
    m_listView->setRootIndex(m_model->index(m_currentDir));
    m_iconView->setRootIndex(m_model->index(m_currentDir));
    m_treeView->setRootIndex(m_model->index(m_currentDir));
    m_tableView->setRootIndex(m_model->index(m_currentDir));
    m_addresBar->changeAddress(m_model->fileInfo(m_model->index(path)));
    emit onChangeDir(currentDirAbsolutePath());
}

void ViewContent::changeDir(QString path)
{
    if(QFileInfo(path).exists())
    {
        m_historyPrevious->push(m_currentDir);
        m_model->setRootPath(path);
        emit onHistoryPrevious(true);
    }
}

void ViewContent::upDir()
{
    QString parent = m_model->fileInfo(
                m_model->index(m_model->rootPath()).parent()).absoluteFilePath();
    changeDir(parent);
}

void ViewContent::previousDir()
{
    if(!m_historyPrevious->isEmpty())
    {
        qDebug() << m_currentDir << "invoke previous dir" << m_historyPrevious->top();
        m_historyNext->push(m_historyPrevious->top());
        if(m_historyPrevious->isEmpty())
            emit onHistoryPrevious(false);
        else if (m_historyPrevious->top() == m_currentDir)
        {
            m_historyPrevious->pop();
            emit onHistoryPrevious(false);
        }

        emit onHistoryNext(true);
        m_model->setRootPath(m_historyPrevious->pop());
    }
    else
    {
        emit onHistoryPrevious(false);
    }
}

bool ViewContent::hasPreviousDir()
{
    return !m_historyPrevious->isEmpty();
}

void ViewContent::nextDir()
{
    if(!m_historyNext->isEmpty())
    {
        qDebug() << "invoke next dir" << m_historyNext->top();
        m_historyPrevious->push(m_historyNext->top());
        if(m_historyNext->isEmpty())
            emit onHistoryNext(false);

        emit onHistoryPrevious(true);
        m_model->setRootPath(m_historyNext->pop());
    }
    else
    {
        emit onHistoryNext(false);
    }
}

bool ViewContent::hasNextDir()
{
    return !m_historyNext->isEmpty();
}

void ViewContent::onView_NClicked(const QModelIndex &index)
{
    QFileInfo info = m_model->fileInfo(index);
    if(info.isDir())
    {
        QString path = m_model->fileInfo(index).absoluteFilePath();
        changeDir(path);
    }
    else if(info.isExecutable())
    {
        executeFile(index);
    }
    else
    {
        openFile(index);
    }
}

void ViewContent::executeFile(QModelIndex index)
{
    QProcess *process = new QProcess;
    process->start(m_model->fileInfo(index).absoluteFilePath());
}

void ViewContent::openFile(QModelIndex index)
{
    QProcess *process = new QProcess;
    process->start("xdg-open", QStringList() << m_model->fileInfo(index).absoluteFilePath());
}

void ViewContent::hide(bool view)
{
    if(view)
        m_model->setFilter(QDir::Dirs | QDir::AllDirs | QDir::Files
                           | QDir::Drives | QDir::NoDotAndDotDot
                           | QDir::AllEntries |QDir::Hidden);
    else
        m_model->setFilter(QDir::Dirs | QDir::AllDirs | QDir::Files
                           | QDir::Drives | QDir::NoDotAndDotDot | QDir::AllEntries);
}

void ViewContent::update()
{
    switch(stackedWidget->currentIndex())
    {
    case INDEX_LISTVIEW:
        // Listview
        m_listView->update(m_model->index(m_model->rootPath()));
    case INDEX_ICONVIEW:
        m_iconView->update(m_model->index(m_model->rootPath()));
    case INDEX_TREEVIEW:
        // Treeview
        m_treeView->update(m_model->index(m_model->rootPath()));
    case INDEX_TABLEVIEW:
        // Tableview
        m_tableView->update(m_model->index(m_model->rootPath()));
    }
}

void ViewContent::setMode(Mode mode)
{
    switch(mode)
    {
    case ViewContent::ListView:
        stackedWidget->setCurrentIndex(INDEX_LISTVIEW);
        //m_listView->setViewMode(QListView::ListMode);
        break;
    case ViewContent::IconsView:
        stackedWidget->setCurrentIndex(INDEX_ICONVIEW);
        //m_iconView->setViewMode(QListView::IconMode);
        break;
    case ViewContent::DetailsView:
        stackedWidget->setCurrentIndex(INDEX_TABLEVIEW);
        break;
    case ViewContent::TreeView:
        stackedWidget->setCurrentIndex(INDEX_TREEVIEW);
        break;
    }
}

void ViewContent::copy()
{
    /*
    QItemSelectionModel listSelectionModel = m_listView->selectionModel();

    QModelIndexList selList;
    if(listSelectionModel->selectedRows(0).count())
        selList = listSelectionModel->selectedRows(0);
    else
        selList = listSelectionModel->selectedIndexes();

    if(selList.count() == 0)
        if(focusWidget() == tree)
            selList << modelView->mapFromSource(modelList->index(pathEdit->itemText(0)));
        else
            return;

    QStringList text;
    foreach(QModelIndex item,selList)
        text.append(modelList->filePath(modelView->mapToSource(item)));

    QApplication::clipboard()->setText(text.join("\n"),QClipboard::Selection);
    QApplication::clipboard()->setMimeData(modelView->mimeData(selList));
    */
}

void ViewContent::move()
{

}

void ViewContent::mkdir(QString name)
{
    m_model->mkdir(m_model->index(QDir::currentPath()), name);
}
