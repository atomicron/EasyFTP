#ifndef EASYFTP_H
#define EASYFTP_H

#include <QMainWindow>

#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QFileIconProvider>

#include "ftp_controller.h"

#include "ui/tree.h"
#include "ui/contentslist.h"

#include "misc/loghelper.h"
#include <misc/queue.h>
#include <misc/helpers.h>

QT_BEGIN_NAMESPACE
namespace Ui { class EasyFTP; }
QT_END_NAMESPACE

class EasyFTP : public QMainWindow
{
    Q_OBJECT

    // Host is the server hostname or IP with a trailing backslack
    QString host;
    // URL is formed by the host + path we want/need/are. Directories should end in trailing slack, files - without
    QString url;

    // Returns the abs url ftp://host/path/to/file
    QString absolute_remote_url(QModelIndex index);
    // Returns abs path /path/to/file
    QString absolute_remote_path(QModelIndex index);

    LH* lh;
    void ui_init();

public:
    EasyFTP(QWidget *parent = nullptr);
    ~EasyFTP();

public slots:
    void log(QString str);
    void localTreeItemClicked(QModelIndex);
    void localTreeItemUploadClicked();
    void localListUploadClicked();
    void remoteTreeItemClicked(QModelIndex);
    void remoteTreeItemDownloadClicked();
    void remoteListDownloadClicked();

    void add_for_upload(QString local_path, QString filename, QString remote_url);
    void add_for_download(QString remote_url, QString filename, QString local_path);

    void perform_queue();
    void queue_changed();

    int queue_append(QString src, QString dest);
    void queue_at(int n);

private slots:
    void on_btn_connect_clicked();

private:
    Ui::EasyFTP *ui;
    FTP_Controller *ftp;
    QFileSystemModel *local_fs_model;
    QStandardItemModel* remote_fs_model = nullptr;
    Queue queue;
    QTimer* timer;

private:
    Tree* local_tree;
    ContentsList* local_list;
    Tree* remote_tree;
    ContentsList* remote_list;

    QAction * action_download;
    QAction * action_upload;
    QAction * action_download_items;
    QAction * action_upload_items;

};
#endif // EASYFTP_H
