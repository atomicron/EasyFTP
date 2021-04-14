#include "helpers.h"

QString add_trailing_slash(const QString arg)
{
    if (!arg.endsWith("/"))
        return arg + "/";
    return arg;
}

QString remove_trailing_slash(const QString arg)
{
    while (arg.endsWith("/"))
        return arg.left(arg.length()-1);
    return arg;
}
