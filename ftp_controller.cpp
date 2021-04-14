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
//    if (local_file)
//        fclose(local_file);
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
//    qDebug () << "ptr of download file: " << (char*)ptr;
    size_t read = fwrite(ptr, size, nmemb, (FILE*)user);
//    fclose((FILE*)user);
    return read;
}

static size_t cb_nothing(void *ptr, size_t size, size_t nmemb, void *data)
{
    return size * nmemb;
}

/*
void FTP_Controller::upload(QString source, QString destination)
{
    set_option(CURLOPT_READFUNCTION, upload_callback);
    set_option(CURLOPT_UPLOAD, 1);
    set_option(CURLOPT_URL, destination.toStdString().c_str());
    set_option(CURLOPT_FTP_USE_EPSV , 1);
//    FILE *file = fopen(source.toStdString().c_str(), "rb");
    local_file = fopen(source.toStdString().c_str(), "rb");
//    fopen_s(&file, src.c_str(), "rb");
    set_option(CURLOPT_READDATA, (void*) local_file);
    set_option(CURLOPT_FTP_USE_EPSV , 0);
}
*/

bool FTP_Controller::upload(QString source, QString destination)
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);
    if (!handle) throw std::runtime_error("Invalid duphandle");

    curl_easy_setopt(handle, CURLOPT_READFUNCTION, upload_callback);
    curl_easy_setopt(handle, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(handle, CURLOPT_URL, destination.toStdString().c_str());
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV , 1);
//    FILE *file = fopen(source.toStdString().c_str(), "rb");
    local_file = fopen(source.toStdString().c_str(), "rb");
//    fopen_s(&file, src.c_str(), "rb");
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
//    FILE* file = fopen(destination.toStdString().c_str(), "wb");
    local_file = fopen(destination.toStdString().c_str(), "wb");
//    fopen_s(&file, dest.c_str(), "wb");
//    fclose(file);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*) local_file);
//    QString data;
//    set_option(CURLOPT_DEBUGDATA, &data);
    curl_easy_setopt(handle, CURLOPT_FTP_USE_EPSV, 0);
    //    set_option(CURLOPT_DIRLISTONLY, 1);

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
//    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb_nothing);
    QString data;
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);
//    qDebug () << "result: " << result;
//    qDebug () << "data: " << data;

    QStringList list = data.split("\r\n");
    list.removeAll(QString(""));
    return list;
}

bool FTP_Controller::is_directory(QString url)
{
    // because we're looking for a directory
    //    if (!url.endsWith("/"))
    //        url += "/";
    url = add_trailing_slash(url);

    CURL* handle = curl_easy_duphandle(curl_easy_handle);

    if (!handle) throw std::runtime_error("Invalid duphandle");

    //    curl_easy_setopt(handle, CURLOPT_DIRLISTONLY, 0);
    //    qDebug () << "CHECKING URL: " << url.toStdString().c_str();
    curl_easy_setopt(handle, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, cb_nothing);

    CURLcode result = curl_easy_perform(handle);
    curl_easy_cleanup(handle);
    //    qDebug () << "result: " << result;

    if (result == CURLE_OK)
        return true;

    return false;
}

//bool FTP_Controller::is_file(QString url)
//{
//    // because we're looking for a file
//    url = remove_trailing_slash(url);

//    CURL* handle = curl_easy_duphandle(curl_easy_handle);

//    if (!handle) throw std::runtime_error("Invalid duphandle");

//    curl_easy_setopt(handle, CURLOPT_URL, url.toStdString().c_str());
//    CURLcode result = curl_easy_perform(handle);
//    curl_easy_cleanup(handle);

//    if (result == CURLE_OK)
//        return true;

//    return false;
//}


bool FTP_Controller::mkdir(QString str)
// creates dir path in str
{
    CURL* handle = curl_easy_duphandle(curl_easy_handle);

    if (!handle) throw std::runtime_error("Invalid duphandle");

    struct curl_slist *slist=NULL;
    //    if (!str.startsWith("/")) str = str;
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
