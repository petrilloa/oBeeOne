#ifndef _Memento_h
#define _Memento_h

#include "LinkedList.h"
#include "application.h"

typedef struct
{
  int thingspeakChannel;
  String thingpeakWriteAPI;
  double publishTime;

  LinkedList<char> items = LinkedList<char>();


} mementoItem;

class Memento
{
    public:
        Memento();

        mementoItem item;

        void GetMemento();
        void SetMemento();

};

#endif
