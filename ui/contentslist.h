#ifndef CONTENTSLIST_H
#define CONTENTSLIST_H

#include <QListWidget>
#include "rclickmenu.h"
#include <iostream>

class ContentsList : public QListWidget
{
    Q_OBJECT

    RClickMenu* rclick_menu = nullptr;

public:
    explicit ContentsList(QWidget* parent = nullptr);
    ~ContentsList();

    void add_rclick_menu(RClickMenu* menu);
    RClickMenu* menu() { if (rclick_menu) return rclick_menu; else throw std::runtime_error("No menu set"); }

public slots:
    //    void onCustomContextMenu(const QPoint& point);
    void onCustomContextMenu(const QPoint& point);


};

#endif // CONTENTSLIST_H
