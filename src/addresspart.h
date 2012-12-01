#ifndef ADDRESSPART_H
#define ADDRESSPART_H

#include <QtGui/QWidget>
#include <QtCore/QFileInfo>
#include "ui_addresspart.h"

class AddressPart : public QWidget, public Ui::AddressPart
{
    Q_OBJECT

public:
    AddressPart(QFileInfo file, QWidget *parent = 0);
    ~AddressPart();

private slots:
    void on_m_pushButtonParent_clicked();

signals:
    void onClickPart(QFileInfo info);
private:
    QFileInfo m_file;
};

#endif // ADDRESSBAR_H
