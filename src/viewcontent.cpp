#include "viewcontent.h"

#include <QtGui/QFileSystemModel>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include "properties.h"

ViewContent::ViewContent(QString startpath, ThumbnailIconProvider *icons, QWidget *parent) :
    QWidget(parent), Ui::ViewContent(),
    m_model(new QFileSystemModel()), m_currentDir(startpath),
    m_iconProvider(icons),
    m_historyPrevious(new QStack<QString>()),
    m_historyFollow(new QStack<QString>())
{
    setupUi(this);

    m_model->setIconProvider(m_iconProvider);
    connect(m_model, SIGNAL(rootPathChanged(QString)),
            this, SLOT(onView_changeDir(QString)));

    m_model->setResolveSymlinks(true);

    m_listView->setModel(m_model);
    m_treeView->setModel(m_model);
    m_tableView->setModel(m_model);

    connectClickedModelIndex();

    m_model->setRootPath(m_currentDir);
}

ViewContent::~ViewContent()
{
    delete m_model;
    delete m_historyFollow;
    delete m_historyPrevious;
}

void ViewContent::connectClickedModelIndex()
{
    if(Properties::Instance()->singleClick)
    {
        connect(m_listView, SIGNAL(clicked(QModelIndex)),
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
        connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
        connect(m_tableView, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(onView_NClicked(QModelIndex)));
    }
}

QString ViewContent::currentDir()
{
    return m_currentDir;
}

void ViewContent::onView_changeDir(QString path)
{
    m_currentDir = path;
    m_listView->setRootIndex(m_model->index(m_currentDir));
    m_treeView->setRootIndex(m_model->index(m_currentDir));
    m_tableView->setRootIndex(m_model->index(m_currentDir));
    emit onChangeDir(m_currentDir);
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
        m_historyFollow->push(m_historyPrevious->top());
        emit onHistoryFollow(true);
        m_model->setRootPath(m_historyPrevious->pop());
    }
    else
    {
        emit onHistoryPrevious(false);
    }
}

void ViewContent::followDir()
{
    if(!m_historyFollow->isEmpty())
    {
        m_historyPrevious->push(m_historyFollow->top());
        emit onHistoryPrevious(true);
        m_model->setRootPath(m_historyFollow->pop());
    }
    else
    {
        emit onHistoryFollow(false);
    }
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
