#ifndef QUEUE_H
#define QUEUE_H

#include <QObject>
#include <QList>

class Queue : public QObject
{
    Q_OBJECT


public:
    Queue();

    struct Item {
        enum Type { DOWNLOAD, UPLOAD } type;
        Item(QString s, QString d, Type t) : source(s), destination(d), type(t) {}
        QString source;
        QString destination;
    };
    void add_to_queue(QString src, QString dest, Item::Type t);
    const QList<Item>& list() { return queue; }
    auto front() { return queue.front(); }
    void pop_front() { queue.pop_front(); }

signals:
    void list_changed();

private:
    QList<Item> queue;

};

#endif // QUEUE_H
