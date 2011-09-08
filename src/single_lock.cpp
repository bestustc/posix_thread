// Thread lock demonstration: two threads are created - writer and reader.
// Both threads access the same variable and use the same mutex to lock in
// order to solve data-race condition. Each thread locks mutex and then
// performs the work. It unlocks mutex at the end.
//
// This executable demonstrates, that with current approach reader is not
// going to see changes to the variable, b/c writer is created second and
// waits for the mutex to be unlocked.
//
// Created by Samvel Khalatyan, Sep 08, 2011
// Copyright 2011, All rights reserved

#include <iostream>
#include <poll.h>

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif

using namespace std;

// to initialize the lock use either: PTHREAD_MUTEX_INITIALIZER or
// pthread_mutex_init()
//
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Global variable to be seen and used by thread: the value is shared by
// parent and child threads
//
int x = 0;

// thead function
//
void *reader(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *) arg;

    if (0 != pthread_mutex_lock(mutex))
    {
        cerr << "failed to lock reader" << endl;

        exit(-1);
    }

    for(int i = 0, newx = -1, oldx = -1; 100 > i; ++i)
    {
        newx = ACCESS_ONCE(x);
        if (newx != oldx)
        {
            cout << "reader: " << newx << endl;
            oldx = newx;
        }
        poll(NULL, 0, 1);
    }

    if (0 != pthread_mutex_unlock(mutex))
    {
        cerr << "failed to unlock reader" << endl;

        exit(-1);
    }

    return NULL;
}

void *writer(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *) arg;

    if (0 != pthread_mutex_lock(mutex))
    {
        cerr << "failed to lock writer" << endl;

        exit(-1);
    }

    for(int i = 0; 3 > i; ++i)
    {
        ++ACCESS_ONCE(x);

        poll(NULL, 0, 5);
    }

    if (0 != pthread_mutex_unlock(mutex))
    {
        cerr << "failed to unlock writer" << endl;

        exit(-1);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // thread id
    //
    pthread_t tid_reader;
    pthread_t tid_writer;

    cout << "parent x: " << x << endl;
    cout << "start threads using the same lock" << endl;

    // arguments of the pthread_create
    //  1. thread ID
    //  2. optional thread attributes of type pthread_attr_t
    //  3. thread function
    //  4. thread function arguments
    //
    if (0 != pthread_create(&tid_reader, NULL, reader, &lock))
    {
        cerr << "failed to create reader" << endl;

        return 1;
    }

    if (0 != pthread_create(&tid_writer, NULL, writer, &lock))
    {
        cerr << "failed to create writer" << endl;

        return 1;
    }
    
    // wait for threads to finish: thre threads return value will be stored in
    // status - it is either the value passed to the exit() or
    // the one used with return statement
    //
    void *status;
    if (0 != pthread_join(tid_reader, &status))
    {
        cerr << "failed to join reader thread" << endl;

        return 1;
    }

    if (0 != pthread_join(tid_writer, &status))
    {
        cerr << "failed to join writer thread" << endl;

        return 1;
    }

    cout << "threads finished" << endl;
    cout << "parent x: " << x << endl;

    return 0;
}
