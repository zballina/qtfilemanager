#ifndef VIEWCONTENT_H
#define VIEWCONTENT_H

#include <QtCore/QStack>
#include <QtGui/QWidget>
#include "filesystemmodel.h"
#include "thumbnailiconprovider.h"
#include "addressbar.h"

#include "ui_viewcontent.h"

class ViewContent : public QWidget, protected Ui::ViewContent
{
    Q_OBJECT

public:
    ViewContent(QString startpath, ThumbnailIconProvider *icons, QWidget *parent = 0);
    ~ViewContent();

    enum Mode
    {
        /**
         * The items are shown as icons with a name-label below.
         */
        IconsView = 0,

        /**
         * The items are shown as icons with a name-label below.
         */
        ListView,


        /**
         * The items are shown as icons with the name-label aligned
         * to the right side.
         */
        TreeView,

        /**
         * The icon, the name and the size of the items are
         * shown per default as a table.
         */
        DetailsView
    };

    QString currentDir();
    QString currentDirAbsolutePath();
    void upDir();
    void previousDir();
    void nextDir();
    void changeDir(QString path = QDir::homePath());
    bool hasNextDir();
    bool hasPreviousDir();
    void hide(bool view);
    void update();
    void setMode(Mode mode);
    void copy();
    void move();
    void mkdir(QString name);

signals:
    void onChangeDir(QString dir);
    void onHistoryPrevious(bool exist);
    void onHistoryNext(bool exist);

private slots:
    void onView_NClicked(const QModelIndex &index);
    void onView_changeDir(QString path);
    void onClickedDirectoryAddressBar(QFileInfo info);

private:
    void executeFile(QModelIndex index);
    void openFile(QModelIndex index);
    void connectClickedModelIndex();

    AddressBar *m_addresBar;
    FileSystemModel *m_model;
    ThumbnailIconProvider *m_iconProvider;
    QString m_currentDir;
    QStack<QString> *m_historyPrevious;
    QStack<QString> *m_historyNext;
};

#endif // VIEWCONTENT_H
