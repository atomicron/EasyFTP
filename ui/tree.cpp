#include "tree.h"

Tree::Tree(QWidget *parent) : QWidget(parent)
  , l(new QLabel(this))
  , le(new QLineEdit(this))
  , tv(new CustomTreeView(this))
  , hlayout(new QHBoxLayout())
  , vlayout(new QVBoxLayout(this))
{
   hlayout->addWidget(l);
   hlayout->addWidget(le);

   vlayout->addLayout(hlayout);

   vlayout->addWidget(tv);
   vlayout->setContentsMargins(0,0,0,0);

   tv->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(tv, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
}

Tree::~Tree()
{
    if (rclick_menu)
        delete rclick_menu;
}

void Tree::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = tv->currentIndex();
    if (!index.isValid()) return; // dont open context menu if an item isn't clicked
    setCurrentItem(index);
    menu()->move(QCursor::pos());
    menu()->show();
}

void Tree::add_rclick_menu(RClickMenu *menu)
{
    //    qDebug() << "Checking for menu";
    if (rclick_menu){
        //        qDebug () << "Attempting deletion";
        delete rclick_menu;
        //        qDebug () << "Deleted";
    }
    //    qDebug () << "Setting new menu";
    rclick_menu = menu;
    //   qDebug () << "New menu set";
}

void Tree::setCurrentItem(QModelIndex item)
{
    current_item = item;
}
