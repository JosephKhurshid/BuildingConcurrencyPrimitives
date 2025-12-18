#include "alllocks.h"
#define SEQ_CST memory_order_seq_cst
#define RELAXED memory_order_relaxed
thread_local node MSC_Lock::myNode;


TAS_Lock::TAS_Lock() {
    flag.store(false,SEQ_CST);
}

void TAS_Lock::lock() {
    bool expected = false;
    //while loop acts as a spin lock, keeps trying to  get lock. So if flag is false, then it matches expected
    //then value returned is true, so we break out of the loop Which then meansthe crit section will execute. 
    //However, if flag is true, then it will return false whic is == to false. So the while loop (spin lock) executes.
    while (flag.compare_exchange_strong(expected, true, SEQ_CST) == false) {
        expected = false; //necessary b/c apparatently the compare exchange fn changes the expected value to true.        
    }
}

void TAS_Lock::unlock() {
    flag.store(false, SEQ_CST); 

}


TTAS_Lock::TTAS_Lock() {
    flag.store(false, SEQ_CST);//I think the store here could be relaxed.
}

void TTAS_Lock::lock() {
    bool expected = false;
    while (true) {
        while (flag.load(SEQ_CST)) {}//if flag true (someone has lock), then we keep spinning
        expected = false;//I don't think I need this.Turns out I'm dumn and 
        if (flag.compare_exchange_strong(expected, true, SEQ_CST)) {
            break; //breaking us out of the outer while loop. So crit section will run
        }
    }
}

void TTAS_Lock::unlock() {
    flag.store(false, SEQ_CST); 
}





Ticket_Lock::Ticket_Lock() {
    next_num.store(0, SEQ_CST);
    now_serving.store(0, SEQ_CST);
}

void Ticket_Lock::lock() {
    int my_num = next_num.fetch_add(1, RELAXED); 
    while(now_serving.load(SEQ_CST) != my_num) {}
}

void Ticket_Lock::unlock() {
    now_serving.fetch_add(1, SEQ_CST);
}


MSC_Lock::MSC_Lock() {
    tail.store(nullptr, SEQ_CST);
}

void MSC_Lock::lock() {
    priv_lock(&myNode);
}
void MSC_Lock::unlock() {
    priv_unlock(&myNode);
}

void MSC_Lock::priv_lock(node* myNode) {
    node* oldTail = tail.load(SEQ_CST);
    myNode->next.store(nullptr, SEQ_CST);
    while(!tail.compare_exchange_strong(oldTail,myNode,SEQ_CST)) {
        //oldTail is updated to latest tail value
    }

    if (oldTail != nullptr) {
        myNode->wait.store(true, SEQ_CST);
        oldTail->next.store(myNode,SEQ_CST);
        while (myNode->wait.load(SEQ_CST)) {}
    }    
}

void MSC_Lock::priv_unlock(node* myNode) {
    node* m = myNode;
    if(tail.compare_exchange_strong(m,nullptr,SEQ_CST)){//No one waiting, freed lock
    }else{//give lock to next thread
        node* next_node = myNode->next.load(SEQ_CST);
        while(next_node == nullptr) {
            next_node =myNode->next.load(SEQ_CST);
        }
        next_node->wait.store(false,SEQ_CST);
    }    
}

void Orig_Lock::lock() {
    lk.lock();
}

void Orig_Lock::unlock() {
    lk.unlock();
}



Sense_Reversal_Barrier::Sense_Reversal_Barrier(int num_threads) {
    cnt.store(0, SEQ_CST);
    sense.store(0, SEQ_CST);
    numThreads = num_threads;
}

void Sense_Reversal_Barrier::arrive_and_wait() {
    thread_local bool my_sense = 0;
    my_sense = !my_sense;
    int cnt_cpy = cnt.fetch_add(1,SEQ_CST);
    
    if(cnt_cpy == numThreads-1) {//last one is here
        cnt.store(0,SEQ_CST);
        sense.store(my_sense,SEQ_CST);
    }else{
        while(sense.load(SEQ_CST) !=my_sense) {}
    }
}

Orig_Barrier::Orig_Barrier(int num_threads) {
    bar = new barrier(num_threads); 
}

void Orig_Barrier::arrive_and_wait() {
    bar->arrive_and_wait();    
}
