#include "easyftp.h"
#include "ui_easyftp.h"
#include <QDebug>

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
        qDebug () << "HEADER IN\n" << data << '\n';
        break;
    case CURLINFO_HEADER_OUT:
        qDebug () << "HEADER OUT\n" << data << '\n';
        break;
    case CURLINFO_DATA_IN:
        qDebug () << "DATA IN\n" << data << '\n';
        *(QString*) userp = QString(data);
        break;
    case CURLINFO_DATA_OUT:
        qDebug () << "DATA OUT\n" << data << '\n';
        break;
    case CURLINFO_SSL_DATA_IN:
        qDebug () << "SSL DATA IN\n" << data << '\n';
        // data is directory listing
        break;
    case CURLINFO_SSL_DATA_OUT:
        qDebug () << "SSL DATA OUT\n" << data << '\n';
        break;
     case CURLINFO_END:
        qDebug () << "END\n" << data << '\n';
        break;
     default:
        break;
    };

    return 0;
}

#include <QCommandLineParser>
#include <QFileSystemModel>

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




    QFileSystemModel *model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    ui->local_tree->setModel(model);



    ui->remote_tree->setSortingEnabled(true);

}

EasyFTP::~EasyFTP()
{
    delete ui;
}

//size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
//    std::ostringstream *stream = (std::ostringstream*)userdata;
//    size_t count = size * nmemb;
//    stream->write(ptr, count);
//    return count;
//}

void EasyFTP::on_btn_connect_clicked()
{
    QString p = ui->in_port->currentText();
    QString h = ui->in_host->text();
    QString u = ui->in_user->text();
    QString pass = ui->in_pass->text();

    url = p + "://" + h;

    ftp->set_url(url);
    ftp->set_logins(u, p);

    ftp->perform();
}

void EasyFTP::ui_init()
{
    QStringList list;
    list << "ftp" << "sftp";
    ui->in_port->addItems(list);
}

void EasyFTP::log(QString str)
// Append to the log box
{
    //    ui->log->append(str);
    ui->log->moveCursor (QTextCursor::End);
    ui->log->insertPlainText (str);
    ui->log->moveCursor (QTextCursor::End);
}
