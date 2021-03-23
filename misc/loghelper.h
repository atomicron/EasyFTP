#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QObject>
#include <QStack>

class LH : public QObject {
   Q_OBJECT

   static LH* instance;
   QStack<QString> stack;
public:
   static LH* get_instance();

   void put(QString arg);
signals:
   void stack_changed(QString);

};

#endif // LOGHELPER_H
