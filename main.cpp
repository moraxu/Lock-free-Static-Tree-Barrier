#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

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
    std::vector<Node*> nodes;
    static thread_local bool threadSense;

    class Node {
        int children;
        std::atomic<int> childCount;
        Node* parent;

    public:
        Node(Node* myParent, int count) : children{count}, childCount{count}, parent{myParent} {
            std::cout <<"NEW NODE: parent="<<myParent<<" my child count=" << childCount.load()<<"\n";

        }

        void await() {
            std::cout <<"Pre0";
            while(childCount.load(std::memory_order_seq_cst) > 0);  //acquire
            std::cout <<"Pre";
            childCount.store(children, std::memory_order_seq_cst);  //relaxed
            std::cout <<"Success0";
            if(parent != nullptr) {
                std::cout <<"Success1";
                parent->childDone();
                std::cout <<"Success2";
                while(sense.load(std::memory_order_seq_cst) != threadSense); //acquire
            }
            else {
                std::cout <<"Success3";
                sense.fetch_xor(1, std::memory_order_seq_cst);  //release or acq_rel
            }
            threadSense = !threadSense;
        }

        void childDone() {
            childCount.fetch_sub(1, std::memory_order_seq_cst); //release
        }
    };

    void build(Node* parent, int depth) {
        if(depth == 0) {
            nodes.push_back(new Node(parent, 0));
        }
        else {
            nodes.push_back(new Node(parent, radix));
            Node* last = nodes.back();
            for(int i = 0 ; i < radix ; ++i) {
                build(last, depth - 1);
            }
        }
    }

public:
    StaticTreeBarrier(int size, int myRadix) : radix{myRadix}, nodes(size) {
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
        for(Node* node : nodes) {
            delete node;
        }
    }
};

std::atomic<int> StaticTreeBarrier::sense{0};
thread_local bool StaticTreeBarrier::threadSense = true;

void first(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 1 ; ++i) {
        //std::cout << "First thread: " << i << "\n";
        barrier->await();
    }
}

void second(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 1 ; ++i) {
        //std::cout << "Second thread: " << i << "\n";
        barrier->await();
    }
}

void third(StaticTreeBarrier* barrier)
{
    for(int i = 0 ; i < 1 ; ++i) {
        //std::cout << "Third thread: " << i << "\n";
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
