//
// Created by idopinto12 on 29/03/2022.
//

#include <cstdio>
#include "uthreads.h"
#include <csetjmp>
#include <map>
#include <set>
#include <list>
#include <signal.h>
#include <sys/time.h>
#include <cstring>
#include <algorithm>

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
#define SLEEPING_AND_BLOCKED 5
#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD_TID 0
#define SECOND 1000000
#define RET_VAL_FOR_LONG 1
/*~~~~~~ERROR MESSAGE~~~~~~~*/
#define SYS_ERR "system error: %s\n"
#define SIG_ACTION_ERR "sig_action failed"
#define PROCMASK_ERR "sigprocmask failed"
#define ITIMER_ERR "setitimer failed"
#define BAD_ALLOC_ERR "allocation failed"

#define LIB_ERR "thread library error: %s\n"
#define INVALID_TID "the thread id is invalid (it needs to be  between 0 to 99)"
#define MAX_THREAD_NUM_REACHED "no more threads can be spawned"

#define NON_EXIST_THREAD "the thread doesn't exist"
#define INVALID_BLOCK_TO_MAIN "it's illegal to block the main thread"
#define NON_POSITIVE_QUANTUM_USECS "quantum_usecs must be positive"

#define INVALID_ENTRY "invalid entry point"

#define SIG_EMPTY_SET_ERR "sigemptyset failed" // set errno to indicate the error

/*This class represents thread object*/
class Thread{
private:
    int tid = 0;
    char* stack{};
    int state;
    int quantum_counter =1;
    thread_entry_point  entry_point{};
    int sleeping_period  = 0;

public:
    sigjmp_buf t_env{};
    /**
     *  C'tor
     * @param tid - thread unique id
     * @param entry -pc address
     */
    Thread(int tid,thread_entry_point entry){
        if(tid != MAIN_THREAD_TID){
            this->tid = tid;

            try{
                this->stack = new char[STACK_SIZE];
            }catch(std::bad_alloc&){
                fprintf(stderr,SYS_ERR,BAD_ALLOC_ERR);
                exit(1);
            }
            this->state = READY;
            this->quantum_counter = 0;
            this->entry_point = entry;
            address_t sp = (address_t) this->stack + STACK_SIZE - sizeof(address_t);
            auto pc = (address_t) this->entry_point;
            sigsetjmp(t_env, RET_VAL_FOR_LONG);
            (t_env->__jmpbuf)[JB_SP] = translate_address(sp);
            (t_env->__jmpbuf)[JB_PC] = translate_address(pc);
            if(sigemptyset(&t_env->__saved_mask)){
                fprintf(stderr,SYS_ERR,SIG_EMPTY_SET_ERR);
                exit(1);
            }
        }
    };

    /**
     * Default C'tor
     */
    Thread(){
        if(tid == MAIN_THREAD_TID){
            this->state = RUNNING;
            sigsetjmp(t_env, RET_VAL_FOR_LONG);

            if(sigemptyset(&t_env->__saved_mask)){
                fprintf(stderr,SYS_ERR,SIG_EMPTY_SET_ERR);
            }
        }
    }

    /**
     * D'tor
     */
    ~Thread(){
        delete[] this->stack;
    }
    /*~~~~~~~~~~~ Getters ~~~~~~~~~~~*/
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
    /*~~~~~~~~~~~ Setters ~~~~~~~~~~~*/

    void inc_quantum() {
        ++this->quantum_counter;
    }
    void set_state(int s){
        this->state = s;
    }

    void set_sleeping_period(int num){
        this->sleeping_period = num;
    }
    void dec_sleeping_period(){
        --this->sleeping_period;
    }
};

/*~~~Data structures~~~~*/
std::list<Thread*> ready_list;
std::map<int,Thread*> thread_map;
std::list<Thread*> sleeping_threads;
/*~~~Global variables~~~~*/
int program_quantum_num;
int global_quantum_counter = 1;
Thread *running_thread;
struct sigaction sa = {nullptr};
struct itimerval timer;
sigset_t set;
/*~~~~~~ Signatures ~~~~~~~*/
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

/**
 * @return the current minimal available tid
 */
int get_thread_id_for_spawn(){
    int check = 0;
    for (auto &pair: thread_map) {
        if(check != pair.first){return check;}
        check++;
    }
    return (int)thread_map.size();
}

/**
 *  remove thread in ready list by tid
 *  @param tid thread id
 */
void remove_thread_in_ready_list_by_tid(int tid) {
    for (auto it = ready_list.begin(); it != ready_list.end() ; it++) {
        if((*it)->get_tid() == tid){
            delete *it;
            ready_list.erase(it);
            return;
        }
    }
}

/**
 * @param flag indicator whether to block or unblock the signal mask set
 */
void block_timer(int how){
    if(sigprocmask(how,&set, nullptr)){
        fprintf(stderr,SYS_ERR, PROCMASK_ERR);
        exit(1);
    }
}

//void default_timer_handler(void){
//    // Install timer_handler as the signal handler for SIGVTALRM.
//    memset(&sa, 0, sizeof(sa));
//    sa.sa_handler = SIG_IGN;
//    sa.sa_flags = SA_SIGINFO;
//    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
//    {
//        printf("sigaction error.");
//    }
//    block_timer(false);
//}

/**
 * initialize the alarm clock timer to expire after quentum time
 */
void timer_configuration(){
    timer.it_value.tv_sec = program_quantum_num / SECOND;        // first time interval, seconds part
    timer.it_value.tv_usec = program_quantum_num % SECOND;        // first time interval, microseconds part
    timer.it_interval.tv_sec = program_quantum_num / SECOND;    // following time intervals, seconds part
    timer.it_interval.tv_usec = program_quantum_num % SECOND;    // following time intervals, microseconds part
}

/**
 * reset a virtual timer. It counts down whenever this process is executing.
 */
void reset_virtual_timer_for_this_process(){
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        fprintf(stderr,SYS_ERR,ITIMER_ERR);
        exit(1);
    }
}

/**
 * every quantum this function is call, iterates over the sleeping list and decrease their sleeping period time.
 * and wakes threads up if needed.
 * @return 0 for success and -1 for failure
 */
int update_sleeping_list(){
//    if(sleeping_threads.empty()){return SUCCESS;}
    for (auto it = sleeping_threads.begin(); it != sleeping_threads.end() ; ++it) {
        auto cur = *it;
        cur->dec_sleeping_period();
        // if the current thread needs to wake up
        if(cur->get_sleeping_period() == 0){
            if(cur->get_state() == SLEEPING){
                cur->set_state(READY);
                ready_list.push_back(cur);
            }
            else{ // otherwise the thread is both blocked and sleeping, and now should be only blocked
                cur->set_state(BLOCK);
            }
            it = sleeping_threads.erase(it);
        }
    }
    return SUCCESS;
}


/**
 * signal handler for the SIGVTALR.
 * executed only when the quantum expire.
 * @param sig - the signal which woke this function
 */
void timer_handler(int sig){
    block_timer(SIG_BLOCK); // so we won't get signal during the execution of this function
    global_quantum_counter++; // TODO maybe in switch_threads function
//    update_sleeping_list();
    /* set the running thread to be in the end of the ready list*/
    running_thread->set_state(READY);
    ready_list.push_back(running_thread);
    switch_threads();
}

/**
 * This function can be called when quantum time is expired, thread is terminated or blocked.
 */
void switch_threads() {

    /* if the running thread isn't terminated */
    if(running_thread != nullptr)
    {
        int ret_val = sigsetjmp(running_thread->t_env, RET_VAL_FOR_LONG);
        if(ret_val == RET_VAL_FOR_LONG){
            block_timer(SIG_UNBLOCK);
            return;
        }
    }
    update_sleeping_list();

    // pop the first thread from the ready list and set it to be the running thread and increase his quantum by one
    running_thread = ready_list.front();
    running_thread->set_state(RUNNING);
    running_thread->inc_quantum();
    ready_list.pop_front();
    block_timer(SIG_UNBLOCK);
//    uthread_print_as_list();

    // jump to the next thread
    siglongjmp(running_thread->t_env,RET_VAL_FOR_LONG);

}

/**
 * Install timer_handler as the signal handler for SIGVTALRM.
 */
void install_timer_handler(){
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        fprintf(stderr,SYS_ERR,SIG_ACTION_ERR);
        exit(1);
    }

}
/**
 * checks if tid between 0 to 100,prints error if needed.
 * assumption: SIGVTALRM is blocked when calling
 * @param tid
 * @return 0 for success -1 othewrwise
 */
int tid_validation(int tid){
    if(tid >= MAX_THREAD_NUM || tid < 0 ){
        fprintf(stderr,LIB_ERR,INVALID_TID);
        block_timer(SIG_UNBLOCK);
        return FAILURE;
    }
    return SUCCESS;
}
/**
 * free all the stacks allocated for each thread (except for main) and clear all data structures
 */
void terminate_all_threads() {
    for (auto p : thread_map) {
        delete p.second;
        p.second = nullptr;
    }
    thread_map.clear();
    ready_list.clear();
    sleeping_threads.clear();
}


/*~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~ UThread library ~~~~~~*/
/**
 * @brief initializes the thread library.
 *
 * You may assume that this function is called before any other thread library function, and that it is called
 * exactly once.
 * The input to the function is the length of a program_quantum_num in micro-seconds.
 * It is an error to call this function with non-positive quantum_usecs.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs){

    if(quantum_usecs <= 0){
        fprintf(stderr,LIB_ERR,NON_POSITIVE_QUANTUM_USECS);
        return FAILURE;
    }

    program_quantum_num = quantum_usecs;

    // initialize the main thread to be the running thread and insert it to map
    try{
        running_thread = new Thread();
    } catch (std::bad_alloc&) {
        fprintf(stderr,SYS_ERR,BAD_ALLOC_ERR);
        exit(1);
    }
    thread_map.insert({MAIN_THREAD_TID,running_thread});
    // initialize empty signal mask set and add SIGVTALRM signal
    // so this signal can be blocked or unblocked using sigprocmask for our use in ease
    sigemptyset(&set);
    sigaddset(&set,SIGVTALRM);

    // initialize the signal handler for the alarm timer,
    // configure the interval length (with quantum) and let the virtual timer start ticking
    install_timer_handler();
    timer_configuration();
    reset_virtual_timer_for_this_process();
//    uthread_print_as_list();
    return SUCCESS;
}
/**
 * @brief Creates a new thread, whose entry point is the function entry_point with the signature
 * void entry_point(void).
 *
 * The thread is added to the end of the READY threads list.
 * The uthread_spawn function should fail if it would cause the number of concurrent threads to exceed the
 * limit (MAX_THREAD_NUM).
 * Each thread should be allocated with a stack of size STACK_SIZE bytes.
 *
 * @return On success, return the ID of the created thread. On failure, return -1.
*/
int uthread_spawn(thread_entry_point entry_point){
    block_timer(SIG_BLOCK);

    if(!entry_point){
        fprintf(stderr,LIB_ERR,INVALID_ENTRY);
    }

    if ((int)(thread_map).size() == MAX_THREAD_NUM){
        fprintf(stderr,LIB_ERR,MAX_THREAD_NUM_REACHED);
        return FAILURE;
    }

    Thread* thread = new Thread(get_thread_id_for_spawn(), entry_point);
    if(thread == nullptr){
        fprintf(stderr,SYS_ERR,BAD_ALLOC_ERR);
        exit(1);
    }

    ready_list.push_back(thread);
    thread_map.insert({thread->get_tid(),thread});
//    printf("Spawned thread: %d\n",thread->get_tid());
//    uthread_print_as_list();
    block_timer(SIG_UNBLOCK);
    return thread->get_tid();
}

void remove_from_sleeping_threads(int tid){
    for(auto it = sleeping_threads.begin();it != sleeping_threads.end();it++){
        if((*it)->get_tid() == tid){
            sleeping_threads.erase(it);
            break;
        }
    }
}
/**
 * @brief Terminates the thread with ID tid and deletes it from all relevant control structures.
 *
 * All the resources allocated by the library for this thread should be released. If no thread with ID tid exists it
 * is considered an error. Terminating the main thread (tid == 0) will result in the termination of the entire
 * process using exit(0) (after releasing the assigned library memory).
 *
 * @return The function returns 0 if the thread was successfully terminated and -1 otherwise. If a thread terminates
 * itself or the main thread is terminated, the function does not return.
*/
int uthread_terminate(int tid){
    block_timer(SIG_BLOCK);
    if(tid_validation(tid)){return FAILURE;}

    if (tid == MAIN_THREAD_TID){
        terminate_all_threads();
        exit(0);
    }
    auto thread_iter = thread_map.find(tid);
    if (thread_iter == thread_map.end()){   // thread doesn't exist
        fprintf(stderr,LIB_ERR,NON_EXIST_THREAD);
        block_timer(SIG_UNBLOCK);
        return FAILURE;
    }
    switch (thread_iter->second->get_state()) {
        case READY:
            remove_thread_in_ready_list_by_tid(tid);
            thread_map.erase(thread_iter);
            break;
        case BLOCK:
            delete thread_iter->second;
            thread_iter->second = nullptr;
            thread_map.erase(thread_iter);//find
            break;
        case (SLEEPING | SLEEPING_AND_BLOCKED):
            delete thread_iter->second;
            thread_iter->second = nullptr;
            thread_map.erase(thread_iter); // maybe use find
            remove_from_sleeping_threads(tid);
            break;
        case RUNNING:
            thread_map.erase(thread_iter);
            reset_virtual_timer_for_this_process();
            global_quantum_counter++;
            delete running_thread;
            running_thread = nullptr;
            if(ready_list.empty()){ uthread_terminate(0);}
            switch_threads();
            break;
    }
    block_timer(SIG_UNBLOCK);
    return SUCCESS;
}

/**
 * @brief Blocks the thread with ID tid. The thread may be resumed later using uthread_resume.
 *
 * If no thread with ID tid exists it is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision should be made. Blocking a thread in
 * BLOCKED state has no effect and is not considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_block(int tid){
    block_timer(SIG_BLOCK);
    if(tid_validation(tid)){return FAILURE;}
    if (tid == MAIN_THREAD_TID){
        fprintf(stderr,LIB_ERR,INVALID_BLOCK_TO_MAIN);
        return FAILURE;
    }
    auto thread_iter = thread_map.find(tid);
    if (thread_iter == thread_map.end()){   // thread doesn't exist
        fprintf(stderr,LIB_ERR,NON_EXIST_THREAD);
        block_timer(SIG_UNBLOCK);
        return FAILURE;
    }
    switch (thread_iter->second->get_state()) {
        case RUNNING:
            thread_iter->second->set_state(BLOCK);
            global_quantum_counter++;
            reset_virtual_timer_for_this_process();
            switch_threads();
            break;
        case READY:
            for(auto it = ready_list.begin();it != ready_list.end();it++){
                if((*it)->get_tid() == tid){
                    ready_list.erase(it);
                    break;
                }
            }
            thread_iter->second->set_state(BLOCK);
            break;
        case SLEEPING:
            thread_iter->second->set_state(SLEEPING_AND_BLOCKED);
            break;

    }
    // else if the the thread in block already there is no effect
    block_timer(SIG_UNBLOCK);
    return SUCCESS;
}

/**
 * @brief Resumes a blocked thread with ID tid and moves it to the READY state.
 *
 * Resuming a thread in a RUNNING or READY state has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid){
    block_timer(SIG_BLOCK);
    if(tid_validation(tid)){return FAILURE;}

    auto blocked_iter = thread_map.find(tid);
    if (blocked_iter == thread_map.end()){   // thread doesn't exist
        fprintf(stderr,LIB_ERR,NON_EXIST_THREAD);
        block_timer(SIG_UNBLOCK);
        return FAILURE;
    }

    if(blocked_iter->second->get_state() == SLEEPING_AND_BLOCKED){
        blocked_iter->second->set_state(SLEEPING);
    }
    if(blocked_iter->second->get_state() == BLOCK){
        blocked_iter->second->set_state(READY);
        ready_list.push_back(blocked_iter->second);
    }
    block_timer(SIG_UNBLOCK);
    return SUCCESS;
}

/**
 * @brief Blocks the RUNNING thread for num_quantums quantums.
 *
 * Immediately after the RUNNING thread transitions to the BLOCKED state a scheduling decision should be made.
 * After the sleeping time is over, the thread should go back to the end of the READY threads list.
 * The number of quantums refers to the number of times a new program_quantum_num starts, regardless of the reason. Specifically,
 * the program_quantum_num of the thread which has made the call to uthread_sleep isn’t counted.
 * It is considered an error if the main thread (tid==0) calls this function.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_sleep(int num_quantums){
    block_timer(SIG_BLOCK);
    if (num_quantums <= 0){
        fprintf(stderr,LIB_ERR,NON_POSITIVE_QUANTUM_USECS);
        return FAILURE;
    }
    running_thread->set_sleeping_period(num_quantums);
    running_thread->set_state(SLEEPING);
    sleeping_threads.push_back(running_thread);
    global_quantum_counter++;
    reset_virtual_timer_for_this_process();
    switch_threads();
    return SUCCESS;
}

/**
 * @brief Returns the thread ID of the calling thread.
 * @return The ID of the calling thread.
*/
int uthread_get_tid(){
    if(running_thread == nullptr){return FAILURE;}
    return running_thread->get_tid();
}


/**
 * @brief Returns the total number of quantums since the library was initialized, including the current program_quantum_num.
 *
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new program_quantum_num starts, regardless of the reason, this number should be increased by 1.
 *
 * @return The total number of quantums.
*/
int uthread_get_total_quantums(){
    return global_quantum_counter;
}


/**
 * @brief Returns the number of quantums the thread with ID tid was in RUNNING state.
 *
 * On the first time a thread runs, the function should return 1. Every additional program_quantum_num that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state when this function is called, include
 * also the current program_quantum_num). If no thread with ID tid exists it is considered an error.
 *
 * @return On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid){
    block_timer(SIG_BLOCK);
    if(tid_validation(tid)){return FAILURE;}

    auto thread_iter = thread_map.find(tid);
    if (thread_iter == thread_map.end()){   // thread doesn't exist
        fprintf(stderr,LIB_ERR,NON_EXIST_THREAD);
        block_timer(SIG_UNBLOCK);
        return FAILURE;
    }
    block_timer(SIG_UNBLOCK);
    return thread_map[tid]->get_quantums();
}
