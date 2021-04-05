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

#include <iostream>
CURLcode FTP_Controller::perform()
{
    return curl_easy_perform(curl_easy_handle);
}

static size_t upload_callback(void* ptr, size_t size, size_t nmemb, void* user)
{
    /* copy as much data as possible into the 'ptr' buffer, but no more than
     'size' * 'nmemb' bytes! */
    size_t written = fread(ptr, size, nmemb, (FILE*)user);
    return written;
}

static size_t download_callback(void *ptr, size_t size, size_t nmemb, void *user)
{
    size_t read = fwrite(ptr, size, nmemb, (FILE*)user);
    fclose((FILE*)user);
    return read;
}


void FTP_Controller::upload(QString source, QString destination)
{
    set_option(CURLOPT_READFUNCTION, upload_callback);
    set_option(CURLOPT_UPLOAD, 1);
    set_option(CURLOPT_URL, destination.toStdString().c_str());
    set_option(CURLOPT_FTP_USE_EPSV , 1);
    FILE *file = fopen(source.toStdString().c_str(), "rb");
//    fopen_s(&file, src.c_str(), "rb");
    set_option(CURLOPT_READDATA, (void*) file);
    set_option(CURLOPT_FTP_USE_EPSV , 0);
}

void FTP_Controller::download(QString source, QString destination)
{
    set_option(CURLOPT_UPLOAD, 0);
    set_option(CURLOPT_URL, source.toStdString().c_str());
    set_option(CURLOPT_WRITEFUNCTION, download_callback);
    set_option(CURLOPT_FTP_USE_EPSV, 1);
    FILE* file = fopen(destination.toStdString().c_str(), "wb");
//    fopen_s(&file, dest.c_str(), "wb");
//    fclose(file);
    set_option(CURLOPT_WRITEDATA, (void*) file);
    QString data;
    set_option(CURLOPT_DEBUGDATA, &data);
    set_option(CURLOPT_FTP_USE_EPSV, 0);
}
