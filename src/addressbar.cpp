#include "addressbar.h"
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QScrollBar>

AddressBar::AddressBar(QFileInfo file, QWidget *parent) :
    QWidget(parent),
    Ui::AddressBar(), m_file(file), m_children(),
    m_spacer(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum))
{
    setupUi(this);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setFrameShadow(QFrame::Plain);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignRight);
    m_scrollAreaWidgetContents = new QWidget();

    m_horizontalLayout = new QHBoxLayout(m_scrollAreaWidgetContents);
    m_horizontalLayout->setContentsMargins(0, 0, 0, 0);
    m_scrollArea->setWidget(m_scrollAreaWidgetContents);

    verticalLayout->addWidget(m_scrollArea);
    addPart();
}

AddressBar::~AddressBar()
{
    removeParts();
    delete m_spacer;
    delete m_horizontalLayout;
    delete m_scrollAreaWidgetContents;
    delete m_scrollArea;
}

void AddressBar::changeAddress(QFileInfo file)
{
    m_file = file;
    addPart();
}

void AddressBar::addPart()
{
    removeParts();

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
            m_children.push_back(child);
        }
    }
    else
    {
        child = new AddressPart(m_file, this);
        connect(child, SIGNAL(onClickPart(QFileInfo)), this, SLOT(onClickPart(QFileInfo)));
        m_horizontalLayout->addWidget(child);
        m_children.push_back(child);
    }
    m_horizontalLayout->addItem(m_spacer);
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


/*
scrollArea = new QScrollArea(AddressBar);
scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
sizePolicy.setHorizontalStretch(0);
sizePolicy.setVerticalStretch(0);
sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
scrollArea->setSizePolicy(sizePolicy);
scrollArea->setFrameShape(QFrame::NoFrame);
scrollArea->setFrameShadow(QFrame::Plain);
scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
scrollArea->setWidgetResizable(true);
scrollArea->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
scrollAreaWidgetContents = new QWidget();
scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
scrollAreaWidgetContents->setGeometry(QRect(0, 0, 72, 72));
horizontalLayout = new QHBoxLayout(scrollAreaWidgetContents);
horizontalLayout->setContentsMargins(0, 0, 0, 0);
horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
scrollArea->setWidget(scrollAreaWidgetContents);

verticalLayout->addWidget(scrollArea);
*/
