#ifndef EASYFTP_H
#define EASYFTP_H

#include <QMainWindow>
#include "ftp_controller.h"
#include "misc/loghelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EasyFTP; }
QT_END_NAMESPACE

class EasyFTP : public QMainWindow
{
    Q_OBJECT

    QString url;

    LH* lh;
    void ui_init();

public slots:
    void log(QString str);

public:
    EasyFTP(QWidget *parent = nullptr);
    ~EasyFTP();

private slots:
    void on_btn_connect_clicked();

private:
    Ui::EasyFTP *ui;
    FTP_Controller *ftp;


};
#endif // EASYFTP_H
