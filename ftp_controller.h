#ifndef FTP_CONTROLLER_H
#define FTP_CONTROLLER_H

#include <QObject>
#include <curl/curl.h>
#include <misc/helpers.h>

class EasyFTP;

class FTP_Controller : public QObject
{
    Q_OBJECT

    CURL* curl_easy_handle;
    EasyFTP *eftp;
    FILE *local_file;


    public:
        explicit FTP_Controller(EasyFTP *parent = nullptr);
    ~FTP_Controller();

    template<typename T>
    CURLcode set_option(CURLoption opt, T par)
    {
        return curl_easy_setopt(curl_easy_handle, opt, par);
    }
    void set_url(QString url);
    void set_logins(QString u, QString p);
    CURLcode perform();

    bool upload(QString source, QString destination);
    bool download(QString source, QString destination);

    QStringList get_directory_listing(QString url);
    bool is_directory(QString url);
//    bool is_file(QString url);
    bool mkdir(QString str);

    void clear_all_settings() { curl_easy_reset(curl_easy_handle); }

public slots:

signals:

};

#endif // FTP_CONTROLLER_H
