#include "easyftp.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EasyFTP w;
    w.show();
    return a.exec();
}
