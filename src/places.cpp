#include "places.h"
#include <QtCore/QDebug>

Places::Places(QWidget *parent) :
    QListView(parent), m_model(new FileSystemModel(parent))
{
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clickedItem(QModelIndex)));
    setModel(m_model);
    m_model->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    m_model->setRootPath("/");
}

Places::~Places()
{
    delete m_model;
}

void Places::clickedItem(const QModelIndex &index)
{
    emit changePlaces(m_model->fileInfo(index));
    qDebug() << m_model->fileInfo(index).absoluteFilePath();
}
