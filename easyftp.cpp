#include "easyftp.h"
#include "ui_easyftp.h"


EasyFTP::EasyFTP(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EasyFTP)
    , ftp(new FTP_Controller(this))
{
    ui->setupUi(this);
    ui_init();


    // Feeding the log helper will emit a signal with a QString which we should put into a log
    connect (LH::get_instance(), SIGNAL(stack_changed(QString)), this, SLOT(log(QString)));

    ui->in_host->setText("127.0.0.1");
    ui->in_user->setText("test");
    ui->in_pass->setText("test");

    connect (&queue, SIGNAL(list_changed()), this, SLOT(perform_queue()));
}

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
//        *(QString*) userp = QString(data);
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

static size_t cb_get_response(void *ptr, size_t size, size_t nmemb, void *data)
{
    *(QString*)data = QString((char*)ptr);
    return size * nmemb;
}

EasyFTP::~EasyFTP()
{
    delete ui;
}

void EasyFTP::update_remote_root_listing(QString data)
{
   QStringList files = data.split("\r\n");
   files.pop_back();

   remote_fs_model = new QStandardItemModel;

   QStandardItem *root = new QStandardItem("/");
   remote_fs_model->appendRow(root);

//   QStandardItem *parentItem = remote_fs_model->invisibleRootItem();

   for (auto x : files)
   {
       QStandardItem* item = new QStandardItem(x);
       root->appendRow(item);
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

    // Reset any previous configuration when initiating connection
    ftp->clear_all_settings();
    // To log
    ftp->set_option(CURLOPT_VERBOSE, 1);
    ftp->set_option(CURLOPT_DEBUGFUNCTION, debug_function);
    // To list directories
    ftp->set_option(CURLOPT_DIRLISTONLY, 1);

    ftp->set_url(url);
    ftp->set_logins(u, pass);

    QString data;
    ftp->set_option(CURLOPT_WRITEFUNCTION, cb_get_response);
    ftp->set_option(CURLOPT_WRITEDATA, &data);
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
    local_tree->le->setEnabled(false);
//    local_tree->tv->setSelectionMode(QAbstractItemView::ExtendedSelection);

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
    remote_tree->le->setEnabled(false);
//    remote_tree->tv->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

//    qDebug () << "Uploading " << info.absoluteFilePath() << " to destination " << dest;

    queue.add_to_queue(info.absoluteFilePath(), dest, Queue::Item::Type::UPLOAD);
//    ftp->upload(info.absoluteFilePath(), dest);
//    ftp->perform();
//    ftp->set_option(CURLOPT_UPLOAD, 0);
}

// ABS PATH
// AND ABS URL should be two functions

QString EasyFTP::absolute_remote_url(QModelIndex index)
{
    QString host_without_slash = host;
    if (host_without_slash.endsWith("/")) host_without_slash.remove(host_without_slash.length() - 1, 1);

    QString path = absolute_remote_path(index);
    if (path == "/") path = ""; // special case when the root dir is selected
//    qDebug () << "Returning abs url: " << host_without_slash + path;
    return host_without_slash + path;
}

QString EasyFTP::absolute_remote_path(QModelIndex index)
// returns abs path of the file without a trailing slash
{
    QString abs_file_path = "";
    QString path = remote_fs_model->data(index).toString();
    while (index.parent().isValid()) {
        index = index.parent();
        QString parent_name = remote_fs_model->data(index).toString();
        if (parent_name == "/") {
            path =  parent_name + path;
        } else {
            path =  parent_name + "/" + path;
        }
    }
    abs_file_path += path;
//    qDebug () << "Returning abs path: " << abs_file_path;
    return abs_file_path;
}

void EasyFTP::remoteTreeItemClicked(QModelIndex index)
{
    action_upload->setEnabled(true);

    QModelIndex temp = index;
    QStack<int> stack;
    while (temp.isValid()) {
        stack.push_back(temp.row());
        temp = temp.parent();
    }
    // ^ ???

    // abs URL
    QString abs_path = absolute_remote_url(index);

    abs_path += "/";
//    qDebug() << "Making request to: " << abs_path;
    ftp->set_url(abs_path);
    QString data;
//    ftp->set_option(CURLOPT_DEBUGDATA, &data);

//    ftp->set_option(CURLOPT_DEBUGFUNCTION, debug_function);
    ftp->set_option(CURLOPT_WRITEFUNCTION, cb_get_response);
    ftp->set_option(CURLOPT_WRITEDATA, &data);
    CURLcode reslt = ftp->perform();
//    qDebug () << "Result is " << reslt;

    if (reslt != CURLE_OK)
    // if result not OK, assume it's a file (remove slash)
    {
//        qDebug () << "Assuming file, removing slash";
        abs_path.remove(abs_path.length()-1, 1); //"/";
//        qDebug() << "Making request to: " << abs_path;
        ftp->set_url(abs_path);
        reslt = ftp->perform();

        if (reslt == CURLE_OK)
        {
//            qDebug () << "File conn successful";
        }
        else
        {
//            qDebug () << "Error, cannot file: " << reslt;
        }
    }
    else
    {
        QStringList contents = data.split("\r\n");
        contents.pop_back();

        // get to the proper item by traversing using the stack
        // ^ ???
        if (stack.count())
        {
            QStandardItem *parent = remote_fs_model->item(stack.back());
            stack.pop_back();
            while (stack.count()) {
                parent = parent->child(stack.back());
                stack.pop_back();
            }

            if (!parent->hasChildren())
                if (contents.count())
                    for (auto x : contents)
                        parent->appendRow(new QStandardItem(x));

            remote_tree->le->setText(abs_path);
        }

    }
}

void EasyFTP::remoteTreeItemDownloadClicked()
{
//    qDebug () << "remote download";
    // try to get path of the indexed item lol
    //    qDebug () << remote_fs_model->data(remote_tree->selectedItem());

    QString abs = absolute_remote_url(remote_tree->selectedItem());
    int i = abs.lastIndexOf("/");
    QString filename = abs.right(abs.length() -i-1);
//    qDebug () << "Filename" << filename;
//    qDebug () << abs;

    //    QFileInfo info = local_fs_model->fileInfo(local_tree->selectedItem());
    //    QString dest = info.absolutePath();
    QString dest = local_tree->le->text();
    if (!dest.endsWith("/"))
        dest += '/';
    dest += filename;

//    qDebug () << "Downloading " << abs << " to destination " << dest;

    queue.add_to_queue(abs, dest, Queue::Item::Type::DOWNLOAD);
//    ftp->download(abs, dest);
//    ftp->perform();
}

void EasyFTP::perform_queue()
{
//    qDebug () << "Im here to perform queue";

    Queue::Item item = queue.front();
    qDebug () << "Source: " << item.source << " Destination: " << item.destination;
    queue.pop_front();

    if (item.type == Queue::Item::Type::UPLOAD)
    {
        ftp->upload(item.source, item.destination);
        ui->queue->append(
            "Upload:\t" +
            item.source +
            "\t>\t" +
            item.destination
        );
    }
    else if (item.type == Queue::Item::Type::DOWNLOAD)
    {
        ftp->download(item.source, item.destination);
        ui->queue->append(
            "Download:\t" +
            item.source +
            "\t>\t" +
            item.destination
        );
    }
    CURLcode result = ftp->perform();
    if (result == CURLE_OK)
        ui->queue->append("|OKAY|");
    else {
        ui->queue->append("|NOT OKAY|");
        qDebug () << "Not okay with code " << result;
    }
    ftp->set_option(CURLOPT_DIRLISTONLY, 1);
}
