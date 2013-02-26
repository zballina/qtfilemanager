#ifndef PLACES_H
#define PLACES_H

#include <QListView>
#include "filesystemmodel.h"

class Places : public QListView
{
    Q_OBJECT
public:
    Places(QWidget *parent = 0);
    ~Places();

private slots:
    void clickedItem(const QModelIndex &index);

signals:
    void changePlaces(QFileInfo info);

private:
    FileSystemModel *m_model;
};

#endif // PLACES_H
