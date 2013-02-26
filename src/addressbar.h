#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QtGui/QWidget>
#include <QtCore/QFileInfo>
#include <QtGui/QFileSystemModel>
#include <QtGui/QScrollArea>
#include <QtGui/QHBoxLayout>
#include <QtCore/QList>
#include "addresspart.h"

class AddressBar : public QWidget
{
    Q_OBJECT

public:
    AddressBar(QFileInfo file, QWidget *parent = 0);
    ~AddressBar();

    void changeAddress(QFileInfo info);

signals:
    void onClickDirectory(QFileInfo info);

private slots:
    void onClickPart(QFileInfo info);
private:
    void removeParts();
    void addPart();

    QHBoxLayout *m_horizontalLayout;
    QList<AddressPart *> m_children;
    QFileInfo m_file;
    QSpacerItem *m_spacer;
};

#endif // ADDRESSBAR_H
