//
// Created by mguzek on 4/25/20.
//

#ifndef STATICTREEBARRIER_STBARRIER_H
#define STATICTREEBARRIER_STBARRIER_H

#include <threads.h>
#include <atomic>

class StaticTreeBarrier {
    class Node;

    int radix;
    std::atomic<bool> sense;
    Node** nodes;
    int nodesSize = 0;

    class Node {
        int children;
        std::atomic<int> childCount;
        StaticTreeBarrier* enclosingBarrier;
        Node* parent;

    public:
        Node(Node* myParent, int count, StaticTreeBarrier* encBarrier) : children{count}, parent{myParent}, enclosingBarrier{encBarrier} {
            childCount=count;
        }

        void await() {
            bool mySense = !enclosingBarrier->sense.load(std::memory_order_relaxed);
            while(childCount.load(std::memory_order_acquire) > 0) {
                thrd_yield();
            }
            childCount.store(children, std::memory_order_relaxed);
            if(parent != nullptr) {
                parent->childDone();
                while(enclosingBarrier->sense.load(std::memory_order_acquire) != mySense) {
                    thrd_yield();
                }
            }
            else {
                enclosingBarrier->sense.store(mySense, std::memory_order_release);
            }
        }

        void childDone() {
            childCount.fetch_sub(1, std::memory_order_release);
        }
    };

    void build(Node* parent, int depth) {
        if(depth == 0) {
            nodes[nodesSize++] = new Node(parent, 0, this);
        }
        else {
            Node* myNode = new Node(parent, radix, this);
            nodes[nodesSize++] = myNode;
            for(int i = 0 ; i < radix ; ++i) {
                build(myNode, depth - 1);
            }
        }
    }

public:
    StaticTreeBarrier(int size, int myRadix) : radix{myRadix}, nodes{new Node*[size]} {
        //Let's assume that there are exactly n = r^(d+1)-1 threads (number of nodes in the full binary tree of depth d)
        sense = false;

        int depth = 0;
        while(size > 1) {
            ++depth;
            size /= radix;
        }
        build(nullptr, depth);
    }

    void await(int i) {
        //Don't check for out-of-bound access for speed
        nodes[i]->await();
    }

    ~StaticTreeBarrier() {
        for(int i = 0 ; i < nodesSize ; ++i) {
            delete nodes[i];
        }
        delete [] nodes;
    }
};

#endif //STATICTREEBARRIER_STBARRIER_H