#include "ftp_controller.h"
#include "easyftp.h"

FTP_Controller::FTP_Controller(EasyFTP *parent)
: QObject(parent)
, eftp(parent)
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl_easy_handle = curl_easy_init();}

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
    /* copy as much data as possible into
     * the 'ptr' buffer, but no more than
     * 'size' * 'nmemb' bytes! */
    size_t written = fread(ptr, size, nmemb, (FILE*)user);
    return written;
}

static size_t download_callback(void *ptr, size_t size, size_t nmemb, void *user)
{
    size_t read = fwrite(ptr, size, nmemb, (FILE*)user);
    return read;
}

static size_t cb_nothing(void *ptr, size_t size, size_t nmemb, void *data)
{
    return size * nmemb;
}

bool FTP_Controller::upload(QString source, QString destination)
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);
    if (!handle) throw std::runtime_error("Invalid duphandle");

    curl_easy_setopt(handle, CURLOPT_READFUNCTION, upload_callback);
    curl_easy_setopt(handle, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(handle, CURLOPT_URL, destination.toStdString().c_str());
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV , 1);
    local_file = fopen(source.toStdString().c_str(), "rb");
    curl_easy_setopt(handle, CURLOPT_READDATA, (void*) local_file);
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV , 0);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    fclose(local_file);

    if (result == CURLE_OK)
        return true;
    return false;
}

bool FTP_Controller::download(QString source, QString destination)
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);
    if (!handle) throw std::runtime_error("Invalid duphandle");

    curl_easy_setopt(handle, CURLOPT_UPLOAD, 0);
    curl_easy_setopt(handle, CURLOPT_URL, source.toStdString().c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, download_callback);
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV, 1);
    curl_easy_setopt(handle, CURLOPT_DIRLISTONLY, 0);
    local_file = fopen(destination.toStdString().c_str(), "wb");
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*) local_file);
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV, 0);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    fclose(local_file);

    if (result == CURLE_OK)
        return true;
    return false;
}

QStringList FTP_Controller::get_directory_listing(QString url)
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);

    if (!handle) throw std::runtime_error("Invalid duphandle");

    curl_easy_setopt(handle, CURLOPT_DIRLISTONLY, 1);
    url = add_trailing_slash(url);
    curl_easy_setopt(handle, CURLOPT_URL, url.toStdString().c_str());
    QString data;
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    QStringList list = data.split("\r\n");
    list.removeAll(QString(""));
    return list;
}

bool FTP_Controller::is_directory(QString url)
{
    // because we're looking for a directory
    url = add_trailing_slash(url);

    CURL* handle = curl_easy_duphandle(curl_easy_handle);

    if (!handle) throw std::runtime_error("Invalid duphandle");

    curl_easy_setopt(handle, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb_nothing);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    if (result == CURLE_OK)
        return true;

    return false;
}

bool FTP_Controller::mkdir(QString str)
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);

    if (!handle) throw std::runtime_error("Invalid duphandle");

    struct curl_slist *slist=NULL;
    QString command = "MKD " + str;
    qDebug() << "Command is: " << command;
    slist = curl_slist_append(slist, command.toStdString().c_str());
    //    if (slist == NULL)
    //        return false;

    curl_easy_setopt(handle, CURLOPT_QUOTE, slist);

    // Ready to get response
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb_nothing);

    CURLcode result = curl_easy_perform(handle);

    curl_slist_free_all(slist); /* free the list again */

    qDebug () << "CURL FROM MKDIR: " << result;

    if (result == CURLE_OK)
        return true;

    return false;
}
