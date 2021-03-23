#include "ftp_controller.h"
#include "easyftp.h"

FTP_Controller::FTP_Controller(EasyFTP *parent)
: QObject(parent)
, eftp(parent)
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl_easy_handle = curl_easy_init();
}

FTP_Controller::~FTP_Controller()
{
    curl_easy_cleanup(curl_easy_handle);
}

void FTP_Controller::set_url(QString url)
{
   set_option(CURLOPT_URL, url.toStdString().c_str());
}

void FTP_Controller::set_logins(QString u, QString p)
{
   set_option(CURLOPT_USERNAME, u.toStdString().c_str());
   set_option(CURLOPT_PASSWORD, p.toStdString().c_str());
}

void FTP_Controller::perform()
{
   curl_easy_perform(curl_easy_handle);
}
