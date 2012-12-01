#include "addressbar.h"
#include <QtCore/QDir>
#include <QtCore/QDebug>

AddressBar::AddressBar(QFileInfo file, QWidget *parent) :
    QWidget(parent),
    Ui::AddressBar(), m_file(file),
    m_spacer(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
    setupUi(this);
    addPart();
}

AddressBar::~AddressBar()
{
    foreach(QObject *child, m_children)
        delete child;

    delete m_spacer;
}

void AddressBar::changeAddress(QFileInfo file)
{
    m_file = file;
    addPart();
}

void AddressBar::addPart()
{
    removeParts();

    if(m_file.absoluteFilePath() != "/")
    {
        QStringList path = m_file.absoluteFilePath().split(QDir::separator());

        for(int i = 0; i < path.size(); i++)
        {
            QString fullpath = "/";
            for(int j = 1; j <= i; j++)
            {
                fullpath += QDir::separator() + path.at(j);
            }
            AddressPart *child = new AddressPart(QFileInfo(fullpath), this);
            connect(child, SIGNAL(onClickPart(QFileInfo)), this, SLOT(onClickPart(QFileInfo)));
            horizontalLayout->addWidget(child);
            m_children.push_back(child);
        }
    }
    else
    {
        AddressPart *child = new AddressPart(m_file, this);
        connect(child, SIGNAL(onClickPart(QFileInfo)), this, SLOT(onClickPart(QFileInfo)));
        horizontalLayout->addWidget(child);
        m_children.push_back(child);
    }
    horizontalLayout->addItem(m_spacer);
}

void AddressBar::removeParts()
{
    foreach(QObject *child, m_children)
        delete child;
    m_children.clear();

    horizontalLayout->removeItem(m_spacer);
}

void AddressBar::onClickPart(QFileInfo info)
{
    qDebug() << "In AddressBar" << info.absoluteFilePath();
    emit onClickDirectory(info);
}
