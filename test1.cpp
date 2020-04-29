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

        tmp = load_32(&sharedVariableC);
        store_32(&sharedVariableC, tmp+1);

        barrier->await(threadInd);

        tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);

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

        tmp = load_32(&sharedVariableA);
        store_32(&sharedVariableA, tmp+1);

        barrier->await(threadInd);

        tmp = load_32(&sharedVariableB);
        store_32(&sharedVariableB, tmp+1);

        barrier->await(threadInd);
    }
}

int user_main(int, char**) {
    constexpr int THREADS_NO = 3;

    StaticTreeBarrier barrier(THREADS_NO, 2);

    thrd_t thread1, thread2, thread3;

    Args args[THREADS_NO] = {   {&barrier, 0},
                                {&barrier, 1},
                                {&barrier, 2} };

    thrd_create(&thread1, (thrd_start_t)first, args+0);
    thrd_create(&thread2, (thrd_start_t)second, args+1);
    thrd_create(&thread3, (thrd_start_t)third, args+2);

    thrd_join(thread1);
    thrd_join(thread2);
    thrd_join(thread3);

    std::cout << "sharedVariableA = " << sharedVariableA << "\n"
              << "sharedVariableB = " << sharedVariableB << "\n"
              << "sharedVariableC = " << sharedVariableC;

    MODEL_ASSERT(sharedVariableA==9);
    MODEL_ASSERT(sharedVariableB==9);
    MODEL_ASSERT(sharedVariableC==9);

    return EXIT_SUCCESS;
}