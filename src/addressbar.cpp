#include "addressbar.h"
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QScrollBar>

AddressBar::AddressBar(QFileInfo file, QWidget *parent) :
    QWidget(parent),
    m_file(file), m_children(),
    m_spacer(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum)),
    m_horizontalLayout(new QHBoxLayout(this))
{
    resize(16, 16);
    setLayoutDirection(Qt::LeftToRight);
    m_horizontalLayout->setContentsMargins(0, 0, 0, 0);

    addPart();
}

AddressBar::~AddressBar()
{
    removeParts();
    delete m_spacer;
    delete m_horizontalLayout;
}

void AddressBar::changeAddress(QFileInfo file)
{
    m_file = file;
    addPart();
}

void AddressBar::addPart()
{
    removeParts();

    qDebug() << m_file.absoluteFilePath();
    qDebug() << parentWidget()->geometry()
             << parentWidget()->width() / 2
             << this->geometry()
             << m_file.absoluteFilePath().length();

    AddressPart *child;

    if(!m_file.isRoot())
    {
        QStringList path = m_file.absoluteFilePath().split(QDir::separator());

        for(int i = 0; i < path.size(); i++)
        {
            QString fullpath = "/";
            for(int j = 1; j <= i; j++)
            {
                fullpath += QDir::separator() + path.at(j);
            }
            child = new AddressPart(QFileInfo(fullpath), this);
            connect(child, SIGNAL(onClickPart(QFileInfo)), this, SLOT(onClickPart(QFileInfo)));
            m_horizontalLayout->addWidget(child);
            qDebug() << i << child->geometry();
            m_children.push_back(child);
        }
    }
    else
    {
        child = new AddressPart(m_file, this);
        connect(child, SIGNAL(onClickPart(QFileInfo)), this, SLOT(onClickPart(QFileInfo)));
        m_horizontalLayout->addWidget(child);
        qDebug() << child->geometry();
        m_children.push_back(child);
    }
    m_horizontalLayout->addItem(m_spacer);

    qDebug() << parentWidget()->geometry()
             << parentWidget()->width() / 2
             << this->geometry()
             << m_file.absoluteFilePath().length();
}

void AddressBar::removeParts()
{
    foreach(QObject *child, m_children)
        delete child;

    m_children.clear();
    m_horizontalLayout->removeItem(m_spacer);
}

void AddressBar::onClickPart(QFileInfo info)
{
    qDebug() << "In AddressBar" << info.absoluteFilePath();
    emit onClickDirectory(info);
}
