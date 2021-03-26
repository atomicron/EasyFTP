#include "easyftp.h"
#include "ui_easyftp.h"

static int debug_function(CURL *handle, curl_infotype type,	char *data, size_t size, void *userp)
// must return 0
{
    switch (type) {
    case CURLINFO_TEXT: {
//        qDebug () << "TEXT\n";
        QString str2 = QString(data);
        LH::get_instance()->put(str2);
        break;
    }
    case CURLINFO_HEADER_IN:
//        qDebug () << "HEADER IN\n"; // << data << '\n';
        break;
    case CURLINFO_HEADER_OUT:
//        qDebug () << "HEADER OUT\n"; // << data << '\n';
        break;
    case CURLINFO_DATA_IN:
//        qDebug () << "DATA IN\n"; // << data << '\n';
        *(QString*) userp = QString(data);
        break;
    case CURLINFO_DATA_OUT:
//        qDebug () << "DATA OUT\n"; // << data << '\n';
        break;
    case CURLINFO_SSL_DATA_IN:
//        qDebug () << "SSL DATA IN\n"; // << data << '\n';
        // data is directory listing
        break;
    case CURLINFO_SSL_DATA_OUT:
//        qDebug () << "SSL DATA OUT\n"; // << data << '\n';
        break;
     case CURLINFO_END:
//        qDebug () << "END\n"; // << data << '\n';
        break;
     default:
        return 0;
        break;
    };

//    qDebug () << data;

    return 0;
}

EasyFTP::EasyFTP(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EasyFTP)
    , ftp(new FTP_Controller(this))
{
    ui->setupUi(this);
    ui_init();
    ftp->set_option(CURLOPT_VERBOSE, 1);
    ftp->set_option(CURLOPT_DEBUGFUNCTION, debug_function);

    // Feeding the log helper will emit a signal with a QString which we should put into a log
    connect (LH::get_instance(), SIGNAL(stack_changed(QString)), this, SLOT(log(QString)));

//    ui->remote_tree->setSortingEnabled(true);

    ftp->set_option(CURLOPT_DIRLISTONLY, 1);
    ui->in_host->setText("127.0.0.1");
//    ui->in_user->setText("localuser");
    ui->in_user->setText("test");
    ui->in_pass->setText("test");

}

EasyFTP::~EasyFTP()
{
    delete ui;
}

void EasyFTP::update_remote_root_listing(QString data)
{
//   qDebug () << "Data received: " << data;
   QStringList directories = data.split("\r\n");
   directories.pop_back();
//   qDebug () << directories;

   remote_fs_model = new QStandardItemModel;
   QStandardItem *parentItem = remote_fs_model->invisibleRootItem();

   for (auto x : directories)
   {
       QStandardItem* item = new QStandardItem(x);
       parentItem->appendRow(item);
   }

   remote_tree->tv->setModel(remote_fs_model);
}

void EasyFTP::on_btn_connect_clicked()
{
    QString p = ui->in_port->currentText();
    QString h = ui->in_host->text();
    QString u = ui->in_user->text();
    QString pass = ui->in_pass->text();

    url = p + "://" + h + "/";
    host = p + "://" + h + "/";

    ftp->set_url(url);
    ftp->set_logins(u, pass);

    QString data;
    ftp->set_option(CURLOPT_DEBUGDATA, &data);
    ftp->perform();

    update_remote_root_listing(data);
}

void EasyFTP::ui_init()
{
    QStringList list;
    list << "ftp" << "sftp";
    ui->in_port->addItems(list);

    // Local site: tree and list
    local_tree = new Tree(this);
    local_tree->l->setText("Local site:");
    local_list = new QListWidget(this);
    ui->splitter_local->addWidget(local_tree);
    ui->splitter_local->addWidget(local_list);

    // Fill local tree with local stuff
    local_fs_model = new QFileSystemModel;
    local_fs_model->setRootPath(QDir::currentPath());
    local_tree->tv->setModel(local_fs_model);

    // Add Right Click Menu to the local_tree
    RClickMenu *local_rclick_menu = new RClickMenu;
    action_upload = new QAction("Upload", local_rclick_menu);
    action_upload->setEnabled(false);
    connect (action_upload, SIGNAL(triggered(bool)), this, SLOT(localTreeItemUploadClicked()));
    local_rclick_menu->addAction(action_upload);
    local_tree->add_rclick_menu(local_rclick_menu);
    connect (local_tree->tv, SIGNAL(itemClicked(QModelIndex)), this, SLOT(localTreeItemClicked(QModelIndex)));

    // Remote site: tree and list
    remote_tree = new Tree(this);
    remote_tree->l->setText("Remote site:");
    remote_list = new QListWidget(this);
    ui->splitter_remote->addWidget(remote_tree);
    ui->splitter_remote->addWidget(remote_list);

    // Fill remote tree
    // ???
    // Obviously can't fill it here

    // Add context menu for items
    RClickMenu *remote_rclick_menu = new RClickMenu;
    action_download = new QAction("Download", local_rclick_menu);
    action_download->setEnabled(false);
    connect (action_download, SIGNAL(triggered(bool)), this, SLOT(remoteTreeItemDownloadClicked()));
    remote_rclick_menu->addAction(action_download);
    remote_tree->add_rclick_menu(remote_rclick_menu);
    connect (remote_tree->tv, SIGNAL(itemClicked(QModelIndex)), this, SLOT(remoteTreeItemClicked(QModelIndex)));

}

void EasyFTP::log(QString str)
// Append to the log box
{
    // ui->log->append(str);
    // The below is an alternative to the above
    // so that a new line isn't added automatically
    ui->log->moveCursor (QTextCursor::End);
    ui->log->insertPlainText (str);
    ui->log->moveCursor (QTextCursor::End);
}

void EasyFTP::localTreeItemClicked(QModelIndex index)
{
    QFileInfo info = local_fs_model->fileInfo(index);

    if (info.isDir()) {
        local_tree->le->setText(info.absoluteFilePath());
    } else {
        local_tree->le->setText(info.absolutePath());
    }

    action_download->setEnabled(true);

}

void EasyFTP::localTreeItemUploadClicked()
{
    QFileInfo info = local_fs_model->fileInfo(local_tree->selectedItem());

    // Enable downloading action since we've clicked a path locally
    QString dest;
//    dest = abs_remote_file_path(remote_tree->selectedItem());
    dest = remote_tree->le->text();
    if (!dest.endsWith('/')) dest += "/";
    dest += info.fileName();

    qDebug () << "Uploading " << info.absoluteFilePath() << " to destination " << dest;

    ftp->upload(info.absoluteFilePath(), dest);
    ftp->perform();
    ftp->set_option(CURLOPT_UPLOAD, 0);
}

QString EasyFTP::abs_remote_file_path(QModelIndex index)
// returns abs path of the file without a trailing slash
{
    QString abs_file_path = "";
    abs_file_path += host;
    QString path = remote_fs_model->data(index).toString();
    while (index.parent().isValid()) {
        index = index.parent();
        path = remote_fs_model->data(index).toString() + "/" + path;
    }
    abs_file_path += path;
    return abs_file_path;
}

void EasyFTP::remoteTreeItemClicked(QModelIndex index)
{
    action_upload->setEnabled(true);

    QStack<int> stack;

    QModelIndex temp = index;
    while (temp.isValid()) {
       stack.push_back(temp.row());
       temp = temp.parent();
    }

    // query the server
    // if the url has children, add them to the index
    QString abs_path = abs_remote_file_path(index);

    // query remote abs path
    if (!abs_path.endsWith("/"))
        abs_path += "/";

    ftp->set_option(CURLOPT_DIRLISTONLY, 1);

    ftp->set_url(abs_path);
    QString data;
    ftp->set_option(CURLOPT_DEBUGDATA, &data);
    CURLcode reslt = ftp->perform();
    qDebug () << "Result is " << reslt;

    if (reslt == CURLE_OK) { // no error
        if (abs_path.endsWith("/"))
            abs_path.remove(abs_path.length()-1, 1);
    } else if (reslt == CURLE_REMOTE_ACCESS_DENIED) { // 9, can't access "dir"
        abs_path.remove(abs_path.length()-1, 1);
        int i = abs_path.lastIndexOf("/");
        abs_path.remove(i, abs_path.length()-i);
    }

    // populate the dir if it's a dir ;D
    QStringList directories = data.split("\r\n");
    directories.pop_back();

    //    qDebug () << "Directories : " << directories;

    // get to the proper item by traversing using the stack
    if (stack.count()) {
        QStandardItem *parent = remote_fs_model->item(stack.back());
        stack.pop_back();
        while (stack.count()) {
            parent = parent->child(stack.back());
            stack.pop_back();
        }

        if (parent->hasChildren()) {
            parent->removeRows(0, parent->rowCount()-1);
        }

        if (directories.count())
            for (auto x : directories) {
                parent->appendRow(new QStandardItem(x));
            }
        remote_tree->le->setText(abs_path);
    }
}

void EasyFTP::remoteTreeItemDownloadClicked()
{
    qDebug () << "remote download";
    // try to get path of the indexed item lol
    //    qDebug () << remote_fs_model->data(remote_tree->selectedItem());

    QString abs = abs_remote_file_path(remote_tree->selectedItem());
    int i = abs.lastIndexOf("/");
    QString filename = abs.right(abs.length() -i-1);
    qDebug () << "Filename" << filename;
    qDebug () << abs;

//    QFileInfo info = local_fs_model->fileInfo(local_tree->selectedItem());
//    QString dest = info.absolutePath();
    QString dest = local_tree->le->text();
    if (!dest.endsWith("/"))
        dest += '/';
    dest += filename;

    qDebug () << "Downloading " << abs << " to destination " << dest;

    ftp->download(abs, dest);
    ftp->perform();
}


//void EasyFTP::update_listing(QTreeWidgetItem* item, int col)
//{
////    if (item->queried)
////        return;

////    item->queried = true;

//    QString url_add = "";
//    QTreeWidgetItem *parent;
//    QTreeWidgetItem *current;
//    current = item;
//    while (parent = current->parent()) {
//        QString line = parent->text(0);
//        QStringList list = line.split(" ", Qt::SkipEmptyParts);

//        for (int i = 0; i<8; ++i) {
//            list.pop_front();
//        }
//        QString name="";
//        for (int i=0; i<list.count(); ++i)
//            name += list[i];

//        url_add = name + "/" + url_add;

//        current = parent;
//        parent = current->parent();

//    }

//    url = host + url_add;


//    QString text = item->text(col);
//    //drwxr-xr-x 1 ftp ftp              0 Mar 16 16:51 filezilla-3.53.0
//    //          1 2   3    4             5   6  7     8
//    bool is_dir;
//    QStringList list = text.split(" ", Qt::SkipEmptyParts);
//    if (list.front().startsWith("d"))
//        is_dir = true;
//    else
//        is_dir = false;

//    for (int i = 0; i<8; ++i) {
//        list.pop_front();
//    }
//    QString name="";
//    for (int i=0; i<list.count(); ++i)
//        name += list[i];

//    if (is_dir) {
//        url += name + "/";
//        qDebug () << "URL is now: " << url;
//        ftp->set_url(url);
//        QString data;
//        ftp->set_option(CURLOPT_DEBUGDATA, &data);
//        ftp->perform();
//        update_listing(item, data);
//    }
//}

//void EasyFTP::update_listing(QTreeWidgetItem* item, QString data)
//{
//    QStringList directories = data.split("\r\n");
//    directories.pop_back();
//    for (auto x : directories) {
//        item->addChild(new QTreeWidgetItem(QStringList(x)));
//        directories.pop_front();
//    }
//}
