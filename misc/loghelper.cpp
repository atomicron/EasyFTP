#include "loghelper.h"

LH* LH::instance = nullptr;

LH* LH::get_instance()
{
    if (!instance)
        instance = new LH;
    return instance;
}

void LH::put(QString arg)
{
    stack.push(arg);
    emit stack_changed(arg);
}
