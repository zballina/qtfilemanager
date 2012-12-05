#ifndef VIEWCONTENT_H
#define VIEWCONTENT_H

#include <QtCore/QStack>
#include <QtGui/QWidget>
#include <QtGui/QFileSystemModel>
#include "thumbnailiconprovider.h"

#include "ui_viewcontent.h"

class ViewContent : public QWidget, public Ui::ViewContent
{
    Q_OBJECT

public:
    ViewContent(QString startpath, ThumbnailIconProvider *icons, QWidget *parent = 0);
    ~ViewContent();

    QString currentDir();
    QString currentDirAbsolutePath();
    void upDir();
    void previousDir();
    void nextDir();
    void changeDir(QString path = QDir::homePath());
    bool hasNextDir();
    bool hasPreviousDir();
    void hide(bool view);

signals:
    void onChangeDir(QString dir);
    void onHistoryPrevious(bool exist);
    void onHistoryNext(bool exist);

private slots:
    void onView_NClicked(const QModelIndex &index);
    void onView_changeDir(QString path);

private:
    void executeFile(QModelIndex index);
    void openFile(QModelIndex index);
    void connectClickedModelIndex();

    QFileSystemModel *m_model;
    ThumbnailIconProvider *m_iconProvider;
    QString m_currentDir;
    QStack<QString> *m_historyPrevious;
    QStack<QString> *m_historyNext;
};

#endif // VIEWCONTENT_H
