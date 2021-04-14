#include "contentslist.h"

ContentsList::ContentsList(QWidget *parent) : QListWidget(parent)
{
   setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
}

ContentsList::~ContentsList()
{
    if (rclick_menu)
        delete rclick_menu;
}

void ContentsList::add_rclick_menu(RClickMenu *menu)
{
    if (rclick_menu)
        delete rclick_menu;
    rclick_menu = menu;
}

void ContentsList::onCustomContextMenu(const QPoint &point)
{
    if (!currentIndex().isValid()) return;
    menu()->move(QCursor::pos());
    menu()->show();
}
