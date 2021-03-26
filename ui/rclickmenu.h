#ifndef RCLICKMENU_H
#define RCLICKMENU_H

#include <QMenu>
#include <QModelIndex>

class RClickMenu : public QMenu
{
    Q_OBJECT
public:
    explicit RClickMenu(QWidget *parent = nullptr);

signals:

};

#endif // RCLICKMENU_H
