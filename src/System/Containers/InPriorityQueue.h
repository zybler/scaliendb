#ifndef INPRIORITYQUEUE_H
#define INPRIORITYQUEUE_H

#include <stdlib.h>
#include "System/Macros.h"

/*
===============================================================================================

 InPriorityQueue for storing objects with pre-allocated next pointer.

===============================================================================================
*/

template<class T>
class InPriorityQueue
{
public:
    InPriorityQueue();

    void    Enqueue(T* elem);   
    void    EnqueuePriority(T* elem);   
    T*      Dequeue();

    void    Clear();
    
    T*      First() const;
    T*      Last() const;
    int     GetLength() const;
    
    T*      Next(T* t) const;

    bool    Contains(T* t);

private:
    T*      head;
    T*      tail;
    T*      prio;
    int     length;
};

/*
===============================================================================================
*/

template<class T>
InPriorityQueue<T>::InPriorityQueue()
{
    length = 0;
    head = NULL;
    tail = NULL;
    prio = NULL;
    Clear();
}


template<class T>
void InPriorityQueue<T>::Enqueue(T* elem)
{
    ASSERT(elem != NULL);
    
    elem->next = NULL;
    if (tail)
        tail->next = elem;
    else
        head = elem;
    tail = elem;
    length++;
}

template<class T>
void InPriorityQueue<T>::EnqueuePriority(T* elem)
{
    ASSERT(elem != NULL);
    
    elem->next = NULL;
    if (prio)
    {
        elem->next = prio->next;
        prio->next = elem;
    }
    else
    {
        elem->next = head;
        head = elem;
    }
    if (prio == tail)
        tail = elem;
    prio = elem;
    length++;
}

template<class T>
T* InPriorityQueue<T>::Dequeue()
{
    T* elem;
    elem = head;
    if (elem)
    {
        head = dynamic_cast<T*>(elem->next);
        if (tail == elem)
            tail = NULL;
        if (prio == elem)
            prio = NULL;
        elem->next = NULL;
        length--;
    }
    return elem;
}

template<class T>
void InPriorityQueue<T>::Clear()
{
    T* elem;

    do
    {
        elem = Dequeue();
    } while (elem);
}

template<class T>
T* InPriorityQueue<T>::First() const
{
    return head;
}

template<class T>
T* InPriorityQueue<T>::Last() const
{
    return tail;
}

template<class T>
T* InPriorityQueue<T>::Next(T* t) const
{
    return dynamic_cast<T*>(t->next);
}

template<class T>
int InPriorityQueue<T>::GetLength() const
{
    return length;
}

template<class T>
bool InPriorityQueue<T>::Contains(T* t)
{
    T* it;
    
    FOREACH (it, *this)
    {
        if (it == t)
            return true;
    }
    
    return false;
}

#endif
