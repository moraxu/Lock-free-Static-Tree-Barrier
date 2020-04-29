//
// Created by mguzek on 4/26/20.
//

#include <iostream>
#include <cstdlib>
#include "librace.h"
#include "model-assert.h"
#include "STBarrier.h"

struct Args {
    StaticTreeBarrier* barrier;
    int threadInd;
};

int sharedVariableA = 0;
int sharedVariableB = 0;
int sharedVariableC = 0;
int sharedVariableD = 0;

void first(Args* args)
{
    StaticTreeBarrier* barrier = args->barrier;
    int threadInd = args->threadInd;
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "First thread: " << i << "\n";

        int tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableB);
        store_32(&sharedVariableB, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableC);
        store_32(&sharedVariableC, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableD);
        store_32(&sharedVariableD, tmp+1);
        barrier->await(threadInd);
    }
}

void second(Args* args)
{
    StaticTreeBarrier* barrier = args->barrier;
    int threadInd = args->threadInd;
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "Second thread: " << i << "\n";

        int tmp = load_32(&sharedVariableB);
        store_32(&sharedVariableB, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableD);
        store_32(&sharedVariableD, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableC);
        store_32(&sharedVariableC, tmp+1);
        barrier->await(threadInd);
    }
}

void third(Args* args)
{
    StaticTreeBarrier* barrier = args->barrier;
    int threadInd = args->threadInd;
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "Third thread: " << i << "\n";

        int tmp = load_32(&sharedVariableC);
        store_32(&sharedVariableC, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableD);
        store_32(&sharedVariableD, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableB);
        store_32(&sharedVariableB, tmp+1);
        barrier->await(threadInd);
    }
}

void fourth(Args* args)
{
    StaticTreeBarrier* barrier = args->barrier;
    int threadInd = args->threadInd;
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "Fourth thread: " << i << "\n";

        int tmp = load_32(&sharedVariableD);
        store_32(&sharedVariableD, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableC);
        store_32(&sharedVariableC, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableB);
        store_32(&sharedVariableB, tmp+1);
        barrier->await(threadInd);

        tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);
        barrier->await(threadInd);
    }
}

int user_main(int, char**) {
    constexpr int THREADS_NO = 4;

    StaticTreeBarrier barrier(THREADS_NO, 3);

    thrd_t threads[THREADS_NO];

    Args args[THREADS_NO] = {    {&barrier, 0},
                                 {&barrier, 1},
                                 {&barrier, 2},
                                 {&barrier, 3} };

    thrd_create(threads+0, (thrd_start_t)first, args+0);
    thrd_create(threads+1, (thrd_start_t)second, args+1);
    thrd_create(threads+2, (thrd_start_t)third, args+2);
    thrd_create(threads+3, (thrd_start_t)fourth, args+3);

    thrd_join(threads[0]);
    thrd_join(threads[1]);
    thrd_join(threads[2]);
    thrd_join(threads[3]);

    std::cout << "sharedVariableA = " << sharedVariableA << "\n"
              << "sharedVariableB = " << sharedVariableB << "\n"
              << "sharedVariableC = " << sharedVariableC << "\n"
              << "sharedVariableD = " << sharedVariableD << "\n";

    MODEL_ASSERT(sharedVariableA==12);
    MODEL_ASSERT(sharedVariableB==12);
    MODEL_ASSERT(sharedVariableC==12);
    MODEL_ASSERT(sharedVariableD==12);

    return EXIT_SUCCESS;
}