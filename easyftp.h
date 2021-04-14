#ifndef EASYFTP_H
#define EASYFTP_H

#include <QMainWindow>
#include "ftp_controller.h"
#include "misc/loghelper.h"
#include "ui/tree.h"
#include "ui/contentslist.h"
#include <QListWidget>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QDebug>
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

    LH* lh;
    void ui_init();
//    void update_remote_root_listing(QString data);
    // Returns the abs url ftp://host/path/to/file
    QString absolute_remote_url(QModelIndex index);
    // Returns abs path /path/to/file
    QString absolute_remote_path(QModelIndex index);



public slots:
    void log(QString str);
    void localTreeItemClicked(QModelIndex);
    void localTreeItemUploadClicked();
    void localListUploadClicked();
    void remoteTreeItemClicked(QModelIndex);
    void remoteTreeItemDownloadClicked();
    void remoteListDownloadClicked();
    void perform_queue();

    void add_for_upload(QString local_path, QString filename, QString remote_url);
    void add_for_download(QString remote_url, QString filename, QString local_path);

public:
    EasyFTP(QWidget *parent = nullptr);
    ~EasyFTP();

private slots:
    void on_btn_connect_clicked();

private:
    Ui::EasyFTP *ui;
    FTP_Controller *ftp;
    QFileSystemModel *local_fs_model;
    QStandardItemModel* remote_fs_model;
    Queue queue;

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
