#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QtGui/QWidget>
#include <QtGui/QFrame>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QAbstractItemView>

class ListView : public QWidget, QFrame
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = 0);

signals:

public slots:

};

#endif // LISTVIEW_H
