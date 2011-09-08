// Thread RW lock performance measurement. Create N readers that use reader
// lock to access variable.
//
// Each thread acquires lock, holds it for some time (emulation of work done),
// and then releases it. After that thread waits for certain amount of time
// doing post-work.
//
// Work and think times can be adjusted. The measured value is the average
// thread number of lock divided by the number of locks acquired by a
// "single" thread application.
//
// Ideally, this number should be 1.0
//
// Created by Samvel Khalatyan, Sep 08, 2011
// Copyright 2011, All rights reserved

#include <iostream>
#include <poll.h>

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif

#ifndef barrier
#define barrier() __asm__ __volatile__("": : :"memory")
#endif

using namespace std;

// to initialize the lock use either: PTHREAD_MUTEX_INITIALIZER or
// pthread_mutex_init()
//
pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

int holdtime = 0;   // time to hold the lock by each thread
int thinktime = 0;  // time between lock release and acquisition by each thread
long long *readcounts;  // array in which each thread puts number of reads
int nreadersrunning = 0;

#define GOFLAG_INIT 0
#define GOFLAG_RUN 1
#define GOFLAG_STOP 2

char goflag = GOFLAG_INIT;  // Synchronization of the test

// thead function
//
void *reader(void *arg)
{
    long long loopcnt = 0;
    int me = *((int *) arg);

    // Atomic function to fetch and change variable
    //
    __sync_fetch_and_add(&nreadersrunning, 1);

    // ACCESS_ONCE makes sure that compiler does not optimize the code and
    // value is retrieved every time
    //
    while(GOFLAG_INIT == ACCESS_ONCE(goflag))
    {
        continue;
    }

    while(GOFLAG_RUN == ACCESS_ONCE(goflag))
    {
        if (0 != pthread_rwlock_rdlock(&lock))
        {
            cerr << "failed to lock reader" << endl;

            exit(-1);
        }

        for(int i = 1; holdtime > i; ++i)
        {
            barrier();
        }

        if (0 != pthread_rwlock_unlock(&lock))
        {
            cerr << "failed to unlock reader" << endl;
        }

        for(int i = 1; thinktime > i; ++i)
        {
            barrier();
        }

        ++loopcnt;
    }

    readcounts[me] = loopcnt;

    return NULL;
}

int main(int argc, char *argv[])
{
    holdtime=1e3;

    const int THREADS = 20;

    long long counts[THREADS];  // store number of counts per thread
    int thread_ids[THREADS];    // we need to store index for each thread

    readcounts = counts;        // setup the pointer

    // thread ids
    //
    pthread_t tid[THREADS];

    cout << "start threads" << endl;

    for(int i = 0; THREADS > i; ++i)
    {
        // the thread_ids array is needed to cache i value. If one writes:
        //
        //  if (0 != pthread_create(&tid[i], NULL, reader, &i))
        //
        // then i would be changed before thread is started and extracts the
        // value....
        //
        thread_ids[i] = i;
        if (0 != pthread_create(&tid[i], NULL, reader, &thread_ids[i]))
        {
            cerr << "failed to create thread: " << i << endl;

            return 1;
        }
    }

    cout << "let threads run" << endl;

    // Atomic flag change
    //
    __sync_val_compare_and_swap(&goflag, GOFLAG_INIT, GOFLAG_RUN);

    // Let all threads run for quite a bit of time
    //
    for(int i = 0; 1000 > i; ++i)
    {
        poll(NULL, 0, 1);
    }

    cout << "stop threads" << endl;

    // Atomic flag change
    //
    __sync_val_compare_and_swap(&goflag, GOFLAG_RUN, GOFLAG_STOP);

    cout << "join threads" << endl;
    
    // wait for threads to finish
    //
    void *status;
    for(int i = 0; THREADS > i; ++i)
    {
        if (0 != pthread_join(tid[i], &status))
        {
            cerr << "failed to join thread: " << i << endl;

            return 1;
        }
    }
    cout << "threads finished" << endl;

    if (1 == THREADS)
    {
        cout << "l1: " << counts[0] << endl;
    }
    else
    {
        // Calcualte measured value
        //
        long long total = 0;
        for(int i = 0; THREADS > i; ++i)
        {
            cout << "reader " << i << " " << counts[i] << endl;
            total += counts[i];
        }

        // The constant corresponds to the number of counts in the
        // "single" thread application
        //
        cout << 1.0 * total / (THREADS * 378797) << endl;
    }

    return 0;
}
