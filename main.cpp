#include <iostream>
#include <thread>
#include <atomic>

class ThreadID {
private:
    static int nextID;
    static thread_local int threadID;

public:
    static int get() {
        return threadID;
    }
    static void set(int newValue) {
        threadID = newValue;
    }
};

int ThreadID::nextID = 0;
thread_local int ThreadID::threadID = nextID++;

class StaticTreeBarrier {
    class Node;

    int radix;
    static std::atomic<int> sense;
    Node** nodes;
    int nodesSize = 0;
    static thread_local bool threadSense;

    class Node {
        int children;
        std::atomic<int> childCount;
        Node* parent;

    public:
        Node(Node* myParent, int count) : children{count}, childCount{count}, parent{myParent} {
            //std::cout <<"NEW NODE: parent="<<myParent<<" my child count=" << childCount.load()<<"\n";
        }

        void await() {
            while(childCount.load(std::memory_order_acquire) > 0);  //acquire
            childCount.store(children, std::memory_order_relaxed);  //relaxed
            if(parent != nullptr) {
                parent->childDone();
                while(sense.load(std::memory_order_acquire) != threadSense); //acquire
            }
            else {
                sense.fetch_xor(1, std::memory_order_acq_rel);  //release or acq_rel
            }
            threadSense = !threadSense;
        }

        void childDone() {
            childCount.fetch_sub(1, std::memory_order_release); //release
        }
    };

    void build(Node* parent, int depth) {
        if(depth == 0) {
            nodes[nodesSize++] = new Node(parent, 0);
        }
        else {
            Node* myNode = new Node(parent, radix);
            nodes[nodesSize++] = myNode;
            for(int i = 0 ; i < radix ; ++i) {
                build(myNode, depth - 1);
            }
        }
    }

public:
    StaticTreeBarrier(int size, int myRadix) : radix{myRadix}, nodes{new Node*[size]} {
        //TODO: Let's assume that there are exactly n = r^(d+1)-1 threads
        int depth = 0;
        while(size > 1) {
            ++depth;
            size /= radix;
        }
        build(nullptr, depth);
    }

    void await() {
        //Don't check for out-of-bound access for speed
        nodes[ThreadID::get()]->await();
    }

    ~StaticTreeBarrier() {
        for(int i = 0 ; i < nodesSize ; ++i) {
            delete nodes[i];
        }
        delete [] nodes;
    }
};

std::atomic<int> StaticTreeBarrier::sense{0};
thread_local bool StaticTreeBarrier::threadSense = true;

void first(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "First thread: " << i << "\n";
        barrier->await();
    }
}

void second(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "Second thread: " << i << "\n";
        barrier->await();
    }
}

void third(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 3 ; ++i) {
        std::cout << "Third thread: " << i << "\n";
        barrier->await();
    }
}

int main() {
    StaticTreeBarrier barrier(3, 2);

    std::thread a(first, &barrier);
    std::thread b(second, &barrier);
    std::thread c(third, &barrier);

    a.join();
    b.join();
    c.join();

    return EXIT_SUCCESS;
}
