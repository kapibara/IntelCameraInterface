
#include <iostream>
#include <queue>
#include <QThread>

#define DEBUG 1

#include "syncqueue.h"

typedef SyncQueue<int> ISyncQueue;

class Consumer: public QThread
{
public:
    Consumer(ISyncQueue &queueRef, int N): q_(queueRef){
        N_ = N;
    }

    void run(){
        int i=0;
        while(i < N_){
            q_.pop();
            sleep(1);
            i++;
        }

        std::cout << "ID: " << this->currentThreadId() << " finishing..." << std::endl;
    }

private:
    ISyncQueue &q_;
    int N_;
};

class Producer: public QThread
{
public:
    Producer(ISyncQueue &queueRef, int N): q_(queueRef){
        N_ = N;
    }

    void run(){
        int i=0;
        while(i < N_){
            q_.push(1);
            sleep(0.1);
            i++;
        }

        std::cout << "ID: " << this->currentThreadId() << " finishing..." << std::endl;
    }

private:
    ISyncQueue &q_;
    int N_;
};

class ComplexObject{
public:
    ComplexObject(int i){
        i_ = i;
    }

    ComplexObject(const ComplexObject &ref){
        std::cout << "copy constructor: " << ref.i_ << std::endl;
        i_ = ref.i_;
    }

    ComplexObject &operator=(ComplexObject &ref){
        std::cout << "operator=: " << ref.i_ << std::endl;
        i_ = ref.i_;
        return *this;
    }

private:
    int i_;
};

int main(int argc, char **argv){

    std::queue<ComplexObject> q;
    ComplexObject co(1);
    ComplexObject co_out(0);

    std::cout << "push an object: " << std::endl;

    q.push(co);

    std::cout << "get the object back: " << std::endl;

    co_out = q.front();

    q.pop();

    std::cout << "finishing: " << std::endl;

    exit(0);


    ISyncQueue queue(5);
    Producer p(queue,11);
    Consumer c1(queue,6),c2(queue,6);

    c1.start();
    p.start();
    c2.start();

    while(!(c1.isFinished() & c2.isFinished() & p.isFinished())){
        QThread::currentThread()->sleep(1);
        std::cout << "sleeping: " <<!(c1.isFinished() & c2.isFinished() & p.isFinished()) << std::endl;
    }

    std::cout << "queue size: " << queue.size() << std::endl;
}
