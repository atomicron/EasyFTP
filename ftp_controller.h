#ifndef FTP_CONTROLLER_H
#define FTP_CONTROLLER_H

#include <QObject>
#include <curl/curl.h>

class EasyFTP;

class FTP_Controller : public QObject
{
    Q_OBJECT

    CURL* curl_easy_handle;
    EasyFTP *eftp;


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
    void perform();

public slots:

signals:

};

#endif // FTP_CONTROLLER_H