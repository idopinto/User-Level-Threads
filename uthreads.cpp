//
// Created by idopinto12 on 29/03/2022.
//

#include <cstdio>
#include "uthreads.h"
#include <queue>
#include <csetjmp>
#include <csignal>
#include <map>
#include <unordered_map>
#include <set>
#include <iostream>
#include <list>
#include <signal.h>
#include <sys/time.h>
#include <cstring>
#include <algorithm>

typedef unsigned long address_t;
sigjmp_buf env[MAX_THREAD_NUM];
#ifdef __x86_64__

/* code for 64 bit Intel arch */
typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}
#endif

/*~~~Constants~~~~*/
#define READY 1
#define BLOCK 2
#define RUNNING 3
#define SLEEPING 4
#define BLOCKED 5
#define SLEEPING_AND_BLOCKED 5
#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD_TID 0
#define SECOND 1000000
#define TERMINATED 0


/*~~~~~~~~~~~~~~~~*/

class Thread{
private:
    int tid = 0;
    char* stack{};
    int state;
//    int quantum_counter = 1;
    int quantum_counter;//TODO
    thread_entry_point  entry_point{};
    int sleeping_period  = 0;
//    bool is_sleeping = false;

    /*ctor*/
public:
    sigjmp_buf t_env{};

    Thread(int tid,thread_entry_point entry){
        if(tid != 0){
            this->tid = tid;
            this->stack = new char[STACK_SIZE];
            this->state = READY;
            this->quantum_counter =0;
            this->entry_point =entry;
            address_t sp = (address_t) this->stack + STACK_SIZE - sizeof(address_t);
            auto pc = (address_t) this->entry_point;
            sigsetjmp(t_env, 1);
            (t_env->__jmpbuf)[JB_SP] = translate_address(sp);
            (t_env->__jmpbuf)[JB_PC] = translate_address(pc);
            sigemptyset(&t_env->__saved_mask);
        }
    };

    Thread(){
        this->state = RUNNING;
        this->quantum_counter=1;//TODO

        if(tid == MAIN_THREAD_TID){
            sigsetjmp(t_env, 1);
            sigemptyset(&t_env->__saved_mask);
        }
    }

    // TODO Rule of three
    ~Thread(){
        delete[] this->stack;
    }

    const int& get_tid() const{
        return this->tid;
    }

    const int& get_state() const{
        return this->state;
    }

    const int& get_quantums() const{
        return this->quantum_counter;
    }

    const int& get_sleeping_period() const{
        return this->sleeping_period;
    }


    void inc_quantum() {
        ++this->quantum_counter;
    }
    void set_state(int s){
        this->state = s;
    }

//    void set_if_sleeping(bool f){
//        this->is_sleeping = f;
//    }

    void set_sleeping_period(int num){
        this->sleeping_period = num;
    }
    void dec_sleeping_period(){
        --this->sleeping_period;
    }

//    void print_thread() const{
//        std::cout << "tid: " << this->tid<<"\nState: "<<this->state<<"\nSP: "
//                    << &this->stack <<"\nPC: "<<&this->entry_point<<"\n"<<std::endl;
//    }
};

/*~~~Data structures~~~~*/
std::list<Thread*> ready_list;
std::map<int,Thread*> thread_map;
std::unordered_map<int,Thread*> blocked_map;
std::list<Thread*> sleeping_threads;
/*~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~Global variables~~~~*/
int quantum_usec;
int global_quantum_counter = 1;
Thread *running_thread;

struct sigaction sa = {nullptr};
struct itimerval timer;
sigset_t set;
/*~~~~~~~~~~~~~~~~~~~~~~*/
void install_timer_handler(void);
void switch_threads();

/*~~~~~~ Helper functions ~~~~~~*/

//int uthread_print(void){
//    printf("##------------------------------------\n");
//    printf("---- Thread map ----\n");

//    for (auto &pair: thread_map) {
//        pair.second->print_thread();
//    }
//    printf("---- Ready list ----\n");
//    for (auto &e: ready_list) {
//        e->print_thread();
//    }
//    printf("##------------------------------------\n");
//    return SUCCESS;
//}

int uthread_print_as_list(void){
    printf("Thread map : ");
    printf("[ ");
    if(!thread_map.empty()){
        for (auto &pair: thread_map) {
            printf("%d,",pair.first);
        }
    }
    printf(" ]\n");
    printf("Ready list : ");
    printf("[ ");

    if (!ready_list.empty()){
        for (auto &e: ready_list) {
            printf("%d,",e->get_tid());
        }
    }
    printf(" ]\n");

    printf("Sleeping list : ");
    printf("[ ");

    if (!sleeping_threads.empty()){
        for (auto &e: sleeping_threads) {
            printf("%d,",e->get_tid());
        }
    }
    printf(" ]\n");

    if(running_thread != nullptr){
        printf("Currently running thread: %d\n",running_thread->get_tid());
    }
    else{
        printf("Currently running thread: NULL\n");

    }
    return SUCCESS;
}

int get_thread_id_for_spawn(){
    int check = 0;
    for (auto &pair: thread_map) {
        if(check != pair.first){return check;}
        check++;
    }
    return (int)thread_map.size();
}

void remove_thread_in_ready_list_by_tid(int tid) {

    auto start = ready_list.begin();
    auto end = ready_list.end();
    for (auto it = start; it != end ; it++) {
        if((*it)->get_tid() == tid){
            delete *it;
            ready_list.erase(it);
            return;
        }
    }
}

void block_timer(bool flag){
    if(flag){
        if(sigprocmask(SIG_BLOCK,&set, nullptr)){
            printf("error");
        }
    }
    else{
        if(sigprocmask(SIG_UNBLOCK,&set, nullptr)){
            printf("error");
        }
    }
}

void default_timer_handler(void){
    // Install timer_handler as the signal handler for SIGVTALRM.
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_SIGINFO;
//    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error.");
    }
    block_timer(false);
}

void timer_configuration(){
    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = quantum_usec / SECOND;        // first time interval, seconds part
    timer.it_value.tv_usec = quantum_usec % SECOND;        // first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = quantum_usec / SECOND;    // following time intervals, seconds part
    timer.it_interval.tv_usec = quantum_usec % SECOND;    // following time intervals, microseconds part
}

void start_virtual_timer_for_this_process(void){
    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        printf("setitimer error.");
    }
}

int update_sleeping_list(){
    for (auto it = sleeping_threads.begin(); it != sleeping_threads.end() ; ++it) {
        (*it)->dec_sleeping_period();
        if((*it)->get_sleeping_period() == 0){
            if((*it)->get_state() == SLEEPING){
                (*it)->set_state(READY);
                ready_list.push_back(*it);
            }
            else{
                (*it)->set_state(BLOCK);
            }
            it = sleeping_threads.erase(it);
        }
    }
    return SUCCESS;
}

void timer_handler(int sig){
    block_timer(true);
    global_quantum_counter++;
//    running_thread->inc_quantum();
//    printf("***** One quantum has passed. now: %d *****\n",uthread_get_total_quantums());
    update_sleeping_list();
    running_thread->set_state(READY);
    ready_list.push_back(running_thread);
//    int ret_val = sigsetjmp(running_thread->t_env,1);
//    if(ret_val == 1){
//        block_timer(false);
//        return;
//    }
//    // push to ready list - only when quantum ends
//    if((running_thread != nullptr)&& (sig != 1)){
//        running_thread->set_state(READY);
//        ready_list.push_back(running_thread);
//    }
    switch_threads();
}

void install_timer_handler(void){
    // Install timer_handler as the signal handler for SIGVTALRM.
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error.");
    }

}

void terminate_all_threads() {
    for (auto p : thread_map) {
        delete p.second;
        p.second = nullptr;
    }
    thread_map.clear();
    ready_list.clear();
    sleeping_threads.clear();
}

void switch_threads() {

    if(running_thread != nullptr)
    {
        int ret_val = sigsetjmp(running_thread->t_env,1);
        if(ret_val == 1){
            block_timer(false);
            return;
        }
    }

    // push to ready list - only when quantum ends
//    if((running_thread != nullptr) && (running_thread->get_state() != BLOCK )){
//        running_thread->set_state(READY);
//        ready_list.push_back(running_thread);
//    }
//    running_thread->inc_quantum();
    running_thread = ready_list.front();
    running_thread->set_state(RUNNING);
    running_thread->inc_quantum();
//    printf("***** One quantum has passed. now: %d *****\n",uthread_get_total_quantums());
//    printf("***** quantum of the running thread . now: %d *****\n",running_thread->get_quantums());
//    if(running_thread->get_quantums() == 0){
//        running_thread->inc_quantum();//TODO WRONG WRONGG
//    }
    ready_list.pop_front();
    block_timer(false);
//    uthread_print_as_list();
    siglongjmp(running_thread->t_env,1);

}

/*~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~ UThread library ~~~~~~*/

int uthread_init(int quantum_usecs){

    if(quantum_usecs <= 0){
        printf("thread library error: invalid quantum_usecs input\n" );
        return FAILURE;
    }

    quantum_usec = quantum_usecs;
    running_thread = new Thread(); // validity
    thread_map.insert({0,running_thread});

    sigemptyset(&set);
    sigaddset(&set,SIGVTALRM);

    install_timer_handler();
    timer_configuration();
    start_virtual_timer_for_this_process();
//    uthread_print_as_list();
    return SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point){
    if(entry_point == nullptr){
        std::cerr<< "ERROR" << std::endl;
    }
    // block timer TODO allocation validity
    block_timer(true);
    if ((int)(thread_map).size() == MAX_THREAD_NUM){
        printf("ERROR");
        return FAILURE;
    }
    Thread* thread = new Thread(get_thread_id_for_spawn(),entry_point);
    ready_list.push_back(thread);
    thread_map.insert({thread->get_tid(),thread});
//    printf("Spawned thread: %d\n",thread->get_tid());
//    uthread_print_as_list();

    // unblock timer
    block_timer(false);
    return thread->get_tid();
}

int uthread_terminate(int tid){
    block_timer(true);
    if(tid>=MAX_THREAD_NUM ||tid<0 ){
        printf("thread library error: invalid input:TERMINATE\n");
        block_timer(false);
        return FAILURE;

    }
    if (tid == MAIN_THREAD_TID){
        terminate_all_threads();
//        std::cout<<"~~~~~~~~~~ Terminating main thread and exist the program... :) ~~~~~~~~~~" <<std::endl;
        exit(0);
    }
    auto thread_iter = thread_map.find(tid);

    if(running_thread->get_tid() != tid){
        if (thread_iter == thread_map.end()){   // thread doesn't exist
            printf("thread library error: thread doesn't exist:TERMINATE\n");
            block_timer(false);
            return FAILURE;
        }
        auto state = thread_iter->second->get_state();
        if (state == READY){
            // release resources
            remove_thread_in_ready_list_by_tid(tid);

            thread_map.erase(thread_iter);
//            printf("***** Terminated thread (READY): %d ***** \n",tid);
//            uthread_print_as_list();
        }
        if (state == BLOCK){
            // thread is blocked. terminate him.
            auto blocked_thread = thread_map.find(tid);
            delete blocked_thread->second;
            thread_map.erase(thread_map.find(tid));
//            printf("***** Terminated thread (BLOCKED): %d ***** \n",tid);
//            uthread_print_as_list();
        }
        else if ((state == SLEEPING) || (state == SLEEPING_AND_BLOCKED)){
            auto thread = thread_map.find(tid);
            delete &thread;
            thread_map.erase(thread_map.find(tid));

            for(auto it = sleeping_threads.begin();it != sleeping_threads.end();it++){
                if((*it)->get_tid() == tid){
                    sleeping_threads.erase(it);
                    break;
                }
            }
        }

    }
    else{
        // TODO terminate the running thread
//        printf("***** Terminated thread (RUNNING): %d *****\n ",tid);
        thread_map.erase(thread_iter);
        start_virtual_timer_for_this_process();
        global_quantum_counter++;
//        printf("***** Abruptly quantum has passed. now: %d *****\n",uthread_get_total_quantums());
//        running_thread->inc_quantum();
        delete running_thread;
        running_thread = nullptr;
        if(ready_list.empty()){ uthread_terminate(0);}
//        printf("***** Termination completed *****\n");
        switch_threads();
//        timer_handler(0);
    }
    block_timer(false);
    return SUCCESS;
}

int uthread_block(int tid){
    block_timer(true);

//    if(tid <= 0){
//        printf("thread library error: invalid input\n");
//        block_timer(false);
//        return FAILURE;
//    }
    if(tid>=MAX_THREAD_NUM ||tid<=0){
        printf("thread library error: invalid input:BLOCK\n");
        block_timer(false);
        return FAILURE;

    }

    auto thread_iter = thread_map.find(tid);
    if (thread_iter == thread_map.end()){   // thread doesn't exist
        printf("thread library error: thread doesn't exist:BLOCK\n");
        block_timer(false);
        return FAILURE;
    }

    if(thread_iter->second->get_state() == RUNNING){
        // if a thread blocks itself
        thread_iter->second->set_state(BLOCK);
//        printf("blocked thread number: %d\n",thread_iter->second->get_tid());
//        timer_handler(1);
        global_quantum_counter++;
//        running_thread->inc_quantum();
        start_virtual_timer_for_this_process();
//        printf("***** Abruptly quantum has passed. now: %d *****\n",uthread_get_total_quantums());

        switch_threads();
    }
    else if(thread_iter->second->get_state() == READY){
        // if a thread blocks another thread in ready list
        for(auto it = ready_list.begin();it != ready_list.end();it++){
            if((*it)->get_tid() == tid){
                ready_list.erase(it);
                break;
            }
        }
        thread_iter->second->set_state(BLOCK);
    }
    else if(thread_iter->second->get_state() == SLEEPING){
        thread_iter->second->set_state(SLEEPING_AND_BLOCKED);
    }
    // else if the the thread in block already there is no effect
    block_timer(false);
    return SUCCESS;
}

int uthread_resume(int tid){
    block_timer(true);

    if(tid <0){//changed from <= to <
        printf("thread library error: invalid input\n");
        block_timer(false);
        return FAILURE;
    }
    auto blocked_iter = thread_map.find(tid);
    if (blocked_iter == thread_map.end()){   // thread doesn't exist
        printf("thread library error: thread doesn't exist:RESUME\n");
        return FAILURE;
    }
    if(blocked_iter->second->get_state() == SLEEPING_AND_BLOCKED){
        blocked_iter->second->set_state(SLEEPING);
    }
    if(blocked_iter->second->get_state() == BLOCK){
        blocked_iter->second->set_state(READY);
        ready_list.push_back(blocked_iter->second);
    }
    block_timer(false);
    return SUCCESS;
}

int uthread_sleep(int num_quantums){
    block_timer(true);
    if (num_quantums <= 0){
        return FAILURE;
    }
    running_thread->set_sleeping_period(num_quantums);
    running_thread->set_state(SLEEPING);
    sleeping_threads.push_back(running_thread);
    global_quantum_counter++;
//    running_thread->inc_quantum();
    start_virtual_timer_for_this_process();
//    printf("***** Abruptly quantum has passed. now: %d *****\n",uthread_get_total_quantums());
    switch_threads();
    return SUCCESS;
}

int uthread_get_tid(){
    return running_thread->get_tid();
}

int uthread_get_total_quantums(){
    return global_quantum_counter;
}

int uthread_get_quantums(int tid){
    // TODO Validation
    block_timer(true);
    if(tid>=MAX_THREAD_NUM ||tid<0 ){
        printf("thread library error: invalid input\n");
        block_timer(false);
        return FAILURE;

    }
    auto thread_iter = thread_map.find(tid);
    if (thread_iter == thread_map.end()) {   // thread doesn't exist
        printf("thread library error: thread doesn't exist:GET\n");
        block_timer(false);
        return FAILURE;
    }

    block_timer(false);
//    auto thread_iter = thread_map.find(tid);
//    if (thread_iter == thread_map.end()){   // thread doesn't exist
//        printf("thread library error: thread doesn't exist:GET\n");
//        printf("%d\n",tid);
////        block_timer(false);
//        return FAILURE;
//    }
//    if(thread_iter->second->get_quantums() == 0  && thread_iter->second->get_state()==RUNNING ){
//        running_thread->inc_quantum();//TODO WRONG WRONGG
//    }
    return thread_map[tid]->get_quantums();
}
/*~~~~~~~~~~~~~~~~~~~~~~*/