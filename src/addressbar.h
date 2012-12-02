#ifndef ADDRESSBAR_H
#define ADDRESSBAR_H

#include <QtGui/QWidget>
#include <QtCore/QFileInfo>
#include <QtGui/QFileSystemModel>
#include <QtGui/QScrollArea>
#include <QtCore/QList>
#include "addresspart.h"

#include "ui_addressbar.h"

class AddressBar : public QWidget, public Ui::AddressBar
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

    QList<AddressPart *> m_children;
    QFileInfo m_file;
    QSpacerItem *m_spacer;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollAreaWidgetContents;
    QHBoxLayout *m_horizontalLayout;
};

#endif // ADDRESSBAR_H
