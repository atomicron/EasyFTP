#include "easyftp.h"
#include "ui_easyftp.h"

static int debug_function(CURL *handle, curl_infotype type,	char *data, size_t size, void *userp)
// must return 0
{
    switch (type) {
    case CURLINFO_TEXT: {
//        qDebug () << "TEXT\n";
//        QString str2 = QString(data);
//        LH::get_instance()->put(str2);
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

EasyFTP::~EasyFTP()
{
    delete ui;
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
    CURLcode result = ftp->perform();
    if (result == CURLE_OK)
        log("Connection successful\n");
    else
    {
        log("Cannot connect, error: " + QString(result) + "\n");
        return;
    }

    // adding root
    if (remote_fs_model) delete remote_fs_model;
    remote_fs_model = new QStandardItemModel;
    QStandardItem *root = new QStandardItem("/");
    remote_fs_model->appendRow(root);
    remote_tree->tv->setModel(remote_fs_model);

    // do something with response data;

    // select the root
    // only the children who have children should be listed
    // expand it
    // all children go into the remote_list

    // select the first index
    QModelIndex root_index = remote_tree->tv->indexAt(QPoint(0,0));
    remote_tree->tv->setCurrentIndex(root_index);
    remoteTreeItemClicked(root_index);
    remote_tree->tv->setExpanded(root_index, true);
    // to enable upload, now that the root at least is selected
    action_upload->setEnabled(true);
}

void EasyFTP::ui_init()
{
    QStringList list;
    list << "ftp" << "sftp";
    ui->in_port->addItems(list);

    // Local site: tree and list
    local_tree = new Tree(this);
    local_tree->l->setText("Local site:");
    local_list = new ContentsList(this);
    ui->splitter_local->addWidget(local_tree);
    ui->splitter_local->addWidget(local_list);

    // Fill local tree with local stuff
    local_fs_model = new QFileSystemModel;
    local_fs_model->setRootPath(QDir::currentPath());
    local_tree->tv->setModel(local_fs_model);
    local_tree->le->setEnabled(false);

    // Add Right Click Menu to the local_tree
    RClickMenu *local_rclick_menu = new RClickMenu;
    action_upload = new QAction("Upload", local_rclick_menu);
    action_upload->setEnabled(false);
    connect (action_upload, SIGNAL(triggered(bool)), this, SLOT(localTreeItemUploadClicked()));
    local_rclick_menu->addAction(action_upload);
    local_tree->add_rclick_menu(local_rclick_menu);
    connect (local_tree->tv, SIGNAL(itemClicked(QModelIndex)), this, SLOT(localTreeItemClicked(QModelIndex)));

    // Add Right Click Menu to local_list
    RClickMenu* local_contents_menu = new RClickMenu;
    action_upload_items = new QAction("Upload", local_contents_menu);
    action_upload_items->setEnabled(false);
    local_contents_menu->addAction(action_upload_items);
    local_list->add_rclick_menu(local_contents_menu);
    connect(action_upload_items, SIGNAL(triggered(bool)), this, SLOT(localListUploadClicked()));
    local_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Remote site: tree and list
    remote_tree = new Tree(this);
    remote_tree->l->setText("Remote site:");
    remote_list = new ContentsList(this);
    ui->splitter_remote->addWidget(remote_tree);
    ui->splitter_remote->addWidget(remote_list);
    remote_tree->le->setEnabled(false);

    // Add context menu for items
    RClickMenu *remote_rclick_menu = new RClickMenu;
    action_download = new QAction("Download", remote_rclick_menu);
    action_download->setEnabled(false);
    connect (action_download, SIGNAL(triggered(bool)), this, SLOT(remoteTreeItemDownloadClicked()));
    remote_rclick_menu->addAction(action_download);
    remote_tree->add_rclick_menu(remote_rclick_menu);
    connect (remote_tree->tv, SIGNAL(itemClicked(QModelIndex)), this, SLOT(remoteTreeItemClicked(QModelIndex)));

    // add context menu for contents
    RClickMenu* remote_contents_menu = new RClickMenu;
    action_download_items = new QAction("Download", remote_contents_menu);
    action_download_items->setEnabled(false);
    remote_contents_menu->addAction(action_download_items);
    remote_list->add_rclick_menu(remote_contents_menu);
    connect(action_download_items, SIGNAL(triggered(bool)), this, SLOT(remoteListDownloadClicked()));
    remote_list->setSelectionMode(QAbstractItemView::ExtendedSelection);


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

QString EasyFTP::absolute_remote_url(QModelIndex index)
{
    QString host_without_slash = remove_trailing_slash(host);

    QString path = absolute_remote_path(index);
    if (path == "/") path = ""; // special case when the root dir is selected
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
    return abs_file_path;
}

void EasyFTP::localTreeItemClicked(QModelIndex index)
{
    QFileInfo info = local_fs_model->fileInfo(index);
    QString path = info.absoluteFilePath();

    if (info.isDir()) {
        local_tree->le->setText(path);
        QStringList contents = QDir(path).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        local_list->clear();
        local_list->addItems(contents);
    } else {
        local_tree->le->setText(path);
    }

    action_download->setEnabled(true);

    local_tree->tv->resizeColumnToContents(0);
}

void EasyFTP::localTreeItemUploadClicked()
{
    QFileInfo info = local_fs_model->fileInfo(local_tree->selectedItem());

    QString local_path = local_tree->le->text();
    QString remote_path = remote_tree->le->text();
    add_for_upload(info.absolutePath(), info.fileName(), remote_path);
}

void EasyFTP::localListUploadClicked()
{
    QString local_path = local_tree->le->text();
    QString remote_path = remote_tree->le->text();
    for (QListWidgetItem *item : local_list->selectedItems())
        add_for_upload(local_path, item->text(), remote_path);
}

void EasyFTP::add_for_upload(QString local_path, QString filename, QString remote_path)
// Takes local_path/filename, queries it, uploads it and any children into remote_path/filename
// Creates directories if needed
// local_path best to be absolute
// remote_path best to be URL so it work with ftp controller, i.e. ftp://1.2.3.4/...
{
    local_path = add_trailing_slash(local_path);
    QString file_path = local_path + filename;

    if (QFileInfo(file_path).isFile())
    {

        if (!remote_path.startsWith(remove_trailing_slash(host)))
            remote_path.push_front(remove_trailing_slash(host));
        log("Uploading file: " + file_path + " to destination: " + remote_path + filename+"\n");
        if (!ftp->upload(file_path,  remote_path + filename))
            log("!!!\tFailed to upload file: " + file_path + " to destination: " + remote_path + filename + "\t!!!\n");
        else
            log("Uploaded file: " + file_path + " to destination: " + remote_path + filename+"\n");
    }
    else if (QFileInfo(file_path).isDir())
    {
        remote_path = add_trailing_slash(remote_path);

        if (remote_path.startsWith(host))
            remote_path.remove(0, host.length());

        if (!remote_path.startsWith("/"))
            remote_path = "/" + remote_path;

        QString remote_file_path = remote_path + filename;
        remote_file_path = remove_trailing_slash(remote_file_path);
        if (!ftp->mkdir(remote_file_path))
            log("Failed to create remote directory: " + remote_file_path+"\n");
        else
            log("Created remote directory: " + remote_file_path+"\n");

        filename = add_trailing_slash(filename);
        QStringList contents = QDir(file_path).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (QString child : contents)
        {
            add_for_upload(local_path, filename+child, remote_path);
        }
    }
}

void EasyFTP::remoteTreeItemClicked(QModelIndex index)
{
    remote_tree->tv->setCurrentIndex(index);
    // Get depth and position at each level
    QModelIndex temp = index;
    QStack<int> stack;
    while (temp.isValid()) {
        stack.push_back(temp.row());
        temp = temp.parent();
    }

    QString abs_remote_url = absolute_remote_url(index);

    // Try for directory
    abs_remote_url += "/";
    ftp->set_url(abs_remote_url);

    // Ready to get response
    QString data;
    ftp->set_option(CURLOPT_WRITEFUNCTION, cb_get_response);
    ftp->set_option(CURLOPT_WRITEDATA, &data);

    // Perform
    CURLcode result = ftp->perform();

    if (result != CURLE_OK)
        // if result not OK, assume it's a file (remove slash)
    {
        abs_remote_url.remove(abs_remote_url.length()-1, 1); // remove trailing slash
        ftp->set_url(abs_remote_url);
        result = ftp->perform();
    }
    else
    {
        QStringList contents = data.split("\r\n", Qt::SkipEmptyParts);

        // Fill contents list
        remote_list->clear();
        remote_list->addItems(contents);

        // get to the proper item by traversing using the stack
        if (stack.count())
        {
            QStandardItem *parent = remote_fs_model->item(stack.back());
            stack.pop_back();
            while (stack.count()) {
                parent = parent->child(stack.back());
                stack.pop_back();
            }

            if (!parent->hasChildren() && contents.count())
            {
                for (auto x : contents)
                {
                    if (ftp->is_directory(add_trailing_slash(abs_remote_url) + x))
                    {
                        parent->appendRow(new QStandardItem(x));
                    }
                }
            }
        }
    }
    remote_tree->le->setText(abs_remote_url);
}

void EasyFTP::remoteTreeItemDownloadClicked()
{
    QString remote_abs_url = absolute_remote_url(remote_tree->selectedItem());
    int i = remote_abs_url.lastIndexOf("/");
    QString filename = remote_abs_url.right(remote_abs_url.length()-i-1);
    QString remote_url = remote_tree->le->text();
    if (remote_url.endsWith(add_trailing_slash(filename)))
        remote_url = remote_url.left(remote_url.length()-add_trailing_slash(filename).length());
    QString local_path = local_tree->le->text();

    add_for_download(remote_url, filename, local_path);
}

void EasyFTP::remoteListDownloadClicked()
{
    QString remote_url = remote_tree->le->text();
    QString local_path = local_tree->le->text();

    QList<QListWidgetItem*> items = remote_list->selectedItems();

    for (auto x : items)
        add_for_download(remote_url, x->text(), local_path);
}


void EasyFTP::add_for_download(QString remote_url, QString filename, QString local_path)
// downloads remote_url/filename to local_path/filename
{

    /* Check if remote_url/filename is a directory or a file
     * if a file -> download it
     * if a directory -> create it locally and call add_for_download with a new path
     * */

    local_path = add_trailing_slash(local_path);
    remote_url = add_trailing_slash(remote_url);
    QString remote_file_url = add_trailing_slash(remote_url) + filename;
    QString local_file_path = local_path + filename;

    if (ftp->is_directory(remote_file_url))
    {
        if (!QDir(local_file_path).exists())
        {
            log("Attempting to create local directory: " + local_file_path+"\n");
            if (!QDir().mkdir(local_file_path))
                log("Cannot create local directory: " + local_file_path+"\n");
            else
                log("Created local directory: " + local_file_path+"\n");
        }
        QStringList contents = ftp->get_directory_listing(remote_file_url);
        contents.removeAll(QString(""));
        for (QString child : contents)
            add_for_download(remote_url+filename,	child, local_path+filename);
    }
    else
    {
        QString local_file_path = local_path + filename;
        log("Downloading file: " + remote_file_url + " to: " + local_file_path+"\n");
        if (!ftp->download(remote_file_url, local_file_path))
            log("!!!\tFailed to download file: " + remote_file_url + " to: " + local_file_path + "\t!!!\n");
        else
            log("Downloaded file: " + remote_file_url + " to: " + local_file_path+"\n");
    }
}

void EasyFTP::perform_queue()
{
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
