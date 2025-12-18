#ifndef ALLLOCKS_H // Start include guard
#define ALLLOCKS_H

#include <atomic>
#include <mutex>
#include <thread> 
#include <barrier>

using namespace std;

class node {
public:
    atomic<node*> next;
    atomic<bool> wait;
};

class Parent_Lock {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class TAS_Lock : public Parent_Lock {
    atomic<bool> flag;
public:
    TAS_Lock();
    void lock(); 
    void unlock();
};

class TTAS_Lock : public Parent_Lock {
    atomic<bool> flag;
public:
    TTAS_Lock();
    void lock();
    void unlock();
};

class Ticket_Lock : public Parent_Lock {
    atomic<int> next_num;
    atomic<int> now_serving;
public:
    Ticket_Lock();
    void lock();
    void unlock();
};

// --- MCS Lock Class ---
class MSC_Lock : public Parent_Lock {
    atomic<node*> tail;
    static thread_local node myNode; 

public:
    MSC_Lock();
    void lock();
    void unlock();
private:
    void priv_lock(node* myNode);
    void priv_unlock(node* myNode);
};

class Orig_Lock : public Parent_Lock {
    mutex lk;
public:
    void lock();
    void unlock();
};


class Parent_Barrier {
    public: 
        virtual void arrive_and_wait() = 0;
};

class Sense_Reversal_Barrier : public Parent_Barrier { 
    atomic<int> cnt;
    atomic<int> sense;
    int numThreads;
    
    public:
        Sense_Reversal_Barrier(int num_threads);
        void arrive_and_wait();
};

class Orig_Barrier : public Parent_Barrier {
    barrier<>* bar;
    public:
        Orig_Barrier(int num_threads);
        void arrive_and_wait();
};



#endif
