#include "addresspart.h"
#include <QtCore/QDebug>

AddressPart::AddressPart(QFileInfo file, QWidget *parent) :
    QWidget(parent),
    Ui::AddressPart(), m_file(file)
{
    setupUi(this);
    if(!m_file.isRoot())
        m_pushButtonParent->setText(m_file.fileName());
    else
        m_pushButtonParent->setText(tr("root"));
    updateGeometry();
}

AddressPart::~AddressPart()
{
}

void AddressPart::on_m_pushButtonParent_clicked()
{
    qDebug() << "In AddressPart" << m_file.absoluteFilePath();
    emit onClickPart(m_file);
}
