// Thread demonstration: memory IS shared between threads 
//
// Created by Samvel Khalatyan, Sep 08, 2011
// Copyright 2011, All rights reserved

#include <iostream>

using namespace std;

// Global variable to be seen and used by thread: the value is shared by
// parent and child threads
//
int x = 0;

// thead function
//
void *thread(void *arg)
{
    x += 1;

    cout << "thread x: " << x << endl;

    // thread may exit with return statement or pthread_exit()
    //
    return NULL;
}

int main(int argc, char *argv[])
{
    // thread id
    //
    pthread_t tid;
    void *status;

    cout << "parent x: " << x << endl;
    cout << "start thread" << endl;

    // arguments of the pthread_create
    //  1. thread ID
    //  2. optional thread attributes of type pthread_attr_t
    //  3. thread function
    //  4. thread function arguments
    //
    if (0 != pthread_create(&tid, NULL, thread, NULL))
    {
        cerr << "failed to create thread" << endl;

        return 1;
    }
    
    // wait for thread to finish: thre threads return value will be stored in
    // status - it is either the value passed to the pthread_exit() or
    // the one used with return statement
    //
    if (0 != pthread_join(tid, &status))
    {
        cerr << "failed to join thread" << endl;

        return 1;
    }

    cout << "thread finished" << endl;
    cout << "parent x: " << x << endl;
    return 0;
}
