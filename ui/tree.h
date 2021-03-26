#ifndef TREE_H
#define TREE_H

#include "rclickmenu.h"

#include <stdexcept>
#include <QWidget>

#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QModelIndex>
#include <QDebug>
#include <QMouseEvent>

// needed a way to get the selected item from the treeview
class CustomTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit CustomTreeView(QWidget *parent = nullptr) : QTreeView(parent)
    {

    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) { // qDebug () << "Emitting";
            emit (itemClicked(index));
        }
    }

signals:
    void itemClicked(QModelIndex);

};

class Tree : public QWidget
{
    Q_OBJECT
public:
    explicit Tree(QWidget *parent = nullptr);
    ~Tree();

    QModelIndex current_item;

signals:
    void itemClicked(QModelIndex item);

public slots:
    void onCustomContextMenu(const QPoint& point);

public:
    QLabel* l;
    QLineEdit* le;
    CustomTreeView* tv;

    // after adding the menu the class is responsible for deleting it later
    // the current rclick menu is destroyed and replaced by the new one
    void add_rclick_menu(RClickMenu* menu);
    RClickMenu* menu() { if (rclick_menu) return rclick_menu; else throw std::runtime_error("No menu set"); }
    void setCurrentItem(QModelIndex item);
    QModelIndex selectedItem() { return current_item; }

private:
    QHBoxLayout *hlayout;
    QVBoxLayout *vlayout;
    RClickMenu* rclick_menu = nullptr;

};

#endif // TREE_H
