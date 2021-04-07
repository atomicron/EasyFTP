#include "queue.h"

Queue::Queue()
{

}

void Queue::add_to_queue(QString src, QString dest, Item::Type type)
{
    queue.append(Item(src, dest, type));

    emit list_changed();
}
