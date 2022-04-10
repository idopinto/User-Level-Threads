#define TEST1


//
//    if (thread_iter == thread_map.end()){   // thread doesn't exist
//        fprintf(stderr,LIB_ERR,NON_EXIST_THREAD);
//        block_timer(SIG_UNBLOCK);
//        return FAILURE;
//    }
//    switch (thread_iter->second->get_state()) {
//        case READY:
//            remove_thread_in_ready_list_by_tid(tid);
//            thread_map.erase(thread_iter);
//            break;
//        case BLOCK:
//            delete thread_iter->second;
////            auto blocked_thread = thread_map.find(tid);
////            delete blocked_thread->second;
//            thread_map.erase(thread_map.find(tid));
//            break;
//        case (SLEEPING | SLEEPING_AND_BLOCKED):
////            auto thread = thread_map.find(tid);
//            delete thread_iter->second;
//            thread_map.erase(thread_map.find(tid));
//            for(auto it = sleeping_threads.begin();it != sleeping_threads.end();it++){
//                if((*it)->get_tid() == tid){
//                    sleeping_threads.erase(it);
//                    break;
//                }
//            }
//            break;
//        case RUNNING:
//            thread_map.erase(thread_iter);
//            reset_virtual_timer_for_this_process();
//            global_quantum_counter++;
//            delete running_thread;
//            running_thread = nullptr;
//            if(ready_list.empty()){ uthread_terminate(0);}
//            switch_threads();
//            break;
//    }
#ifdef TEST1

/**********************************************
 * Test 1: correct threads ids
 *
 **********************************************/
#include <cstdio>
#include <cstdlib>
#include "uthreads.h"
#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"
void halt()
{
    while (true)
    {}
}

void wait_next_quantum()
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) == quantum)
    {}
    return;
}

void thread1()
{
    uthread_block(uthread_get_tid());
}

void thread2()
{
    halt();
}

void error()
{
    printf(RED "ERROR - wrong id returned\n" RESET);
    exit(1);
}

int main()
{
    printf(GRN "Test 1:    " RESET);
    fflush(stdout);

    int q[2] = {10, 20};
    uthread_init(2);
    if (uthread_spawn(thread1) != 1)
        error();
    if (uthread_spawn(thread2) != 2)
        error();
    if (uthread_spawn(thread2) != 3)
        error();
    if (uthread_spawn(thread1) != 4)
        error();
    if (uthread_spawn(thread2) != 5)
        error();
    if (uthread_spawn(thread1) != 6)
        error();

    uthread_terminate(5);
    if (uthread_spawn(thread1) != 5)
        error();

    wait_next_quantum();
    wait_next_quantum();

    uthread_terminate(5);
    if (uthread_spawn(thread1) != 5)
        error();

    uthread_terminate(2);
    if (uthread_spawn(thread2) != 2)
        error();

    uthread_terminate(3);
    uthread_terminate(4);
    if (uthread_spawn(thread2) != 3)
        error();
    if (uthread_spawn(thread1) != 4)
        error();

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
#endif

#ifdef  TEST2
/**********************************************
 * Test 5: sync/block/resume
 *
 **********************************************/

#include <cstdio>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 4
#define RUN 0
#define DONE 1
char thread_status[NUM_THREADS];


void halt()
{
    while (true)
    {}
}

int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void thread()
{
    //uthread_sync(next_thread());

    //uthread_sync(next_thread());

    uthread_block(uthread_get_tid());

    for (int i = 0; i < 50; i++)
    {
        uthread_resume(next_thread());
    }

    thread_status[uthread_get_tid()] = DONE;

    halt();
}

bool all_done()
{
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
    }
    return res;
}

int main()
{
    printf(GRN "Test 5:    " RESET);
    fflush(stdout);

    int q[2] = {10, 20};
    uthread_init(2);
    uthread_spawn(thread);
    uthread_spawn(thread);
    uthread_spawn(thread);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

    while (!all_done())
    {
        uthread_resume(1);
    }

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
#endif

#ifdef TEST3
/**********************************************
 * Test 42: each thread has its own unique stack
 *
 **********************************************/

#include <cstdio>
#include <cstdlib>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 7
#define RUN 0
#define DONE 1

char thread_status[NUM_THREADS];

int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void wait_next_quantum()
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) == quantum)
    {}
    return;
}

void run_test()
{
    int tid = uthread_get_tid();
    int arr[10];

    for (int i = 0; i < 10; i++)
    {
        arr[i] = i * tid;
    }

    //uthread_sync(next_thread());

    for (int i = 0; i < 10; i++)
    {
        if (arr[i] != i * tid)
        {
            printf(RED "ERROR - stack values changed\n" RESET);
            exit(1);
        }
    }

    int b1 = tid * 314, b2 = tid * 141;

    // let switching be invoked by the timer
    wait_next_quantum();


    if ((b1 != tid * 314) || (b2 != tid * 141) || (tid != uthread_get_tid()))
    {
        printf(RED "ERROR - stack values changed\n" RESET);
        exit(1);
    }

    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

bool all_done()
{
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
    }
    return res;
}


int main()
{
    printf(GRN "Test 42:   " RESET);
    fflush(stdout);

    int q[2] = {10, 20};
    uthread_init(2);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);
    uthread_spawn(run_test);

    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }


    while (!all_done())
    {}

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
#endif

#ifdef TEST4
/**********************************************
 * Test 429: Compute the N'th catalan number using
 *           multiple threads.
 *           (+random calls to uthreads functions)
 *
 **********************************************/

/*
 * recall that the n'th catalan number is
 *
 *              n
 *           _______
 *           |     |    n+k
 *  C_n  =   |     |   -----
 *           |     |     k
 *             k=2
 */


#include <cstdio>
#include <cstdlib>
#include <time.h>
#include "uthreads.h"


#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define N 15
#define CATALAN_N 9694845
#define NUM_THREADS 6

#define RUN 0
#define DONE 1

typedef unsigned long ulong;

char thread_status[NUM_THREADS];

ulong partial_calc[NUM_THREADS];


void halt()
{
    while (true)
    {}
}

int rand_tid()
{
    return rand() % NUM_THREADS;
}

// random and legal call to one or more of block/sync/resume
void random_uthreads_call()
{
    int tid = rand_tid();
    if (rand() % 2)
    {
        if (tid != 0)
        {
            uthread_block(tid);
        }
    }
    else
    {
//        if (tid != uthread_get_tid())
//        {
//            uthread_sync(tid);
//        }
    }
    uthread_resume(rand_tid());
}

ulong partial_numerator_calculator(ulong start, ulong step)
{
    int tid;
    ulong res = 1;
    for (ulong k = start; k <= N; k += step)
    {
        random_uthreads_call();

        res *= (N + k);

        random_uthreads_call();
    }
    return res;
}

ulong partial_denominator_calculator(ulong start, ulong step)
{
    int tid;
    ulong res = 1;
    for (ulong k = start; k <= N; k += step)
    {
        random_uthreads_call();

        res *= k;

        random_uthreads_call();
    }
    return res;
}

void summon_calculator(bool numerator, ulong start, ulong step)
{
    if (numerator)
    {
        partial_calc[uthread_get_tid()] = partial_numerator_calculator(start, step);
    }
    else
    {
        partial_calc[uthread_get_tid()] = partial_denominator_calculator(start, step);
    }
    thread_status[uthread_get_tid()] = DONE;
    halt();
}

void thread1()
{
    summon_calculator(true, 2, 3);
}

void thread2()
{
    summon_calculator(true, 3, 3);
}

void thread3()
{
    summon_calculator(true, 4, 3);
}

void thread4()
{
    summon_calculator(false, 2, 2);
}

void thread5()
{
    summon_calculator(false, 3, 2);
}


bool all_done()
{
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
    }
    return res;
}


int main()
{
    printf(GRN "Test 429:  " RESET);
    fflush(stdout);

    srand(time(NULL));

    // threads 1,2,3 - numerator
    // threads 4,5 - denominator

    int q[2] = {10, 20};
    uthread_init(2);

    uthread_spawn(thread1);
    uthread_spawn(thread2);
    uthread_spawn(thread3);
    uthread_spawn(thread4);
    uthread_spawn(thread5);

    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

    int tid = 0;
    while (!all_done())
    {
        // sequentially resume all threads, as they are blocking each other like madmen
        uthread_resume(tid);
        tid = (tid + 1) % NUM_THREADS;
    }

    ulong res = (partial_calc[1] * partial_calc[2] * partial_calc[3]) /
                (partial_calc[4] * partial_calc[5]);

    if (res == CATALAN_N)
    {
        printf(GRN "SUCCESS\n" RESET);
    }
    else
    {
        printf(RED "ERROR - failed to correctly calculate catalan number\n" RESET);
    }
    uthread_terminate(0);

}
#endif

#ifdef TEST5
/**********************************************
 * Test 1430: sort array using multiple threads
 *
 **********************************************/



/*
 *          main              (merge N-size array)
 *         /    \
 *        /      \
 *       /        \
 *      t1        t2          (merge N/2-size array)
 *     / \        / \
 *    /   \      /   \
 *  t3    t4    t5    t6      (sort N/4-size array)
 *
 * DISCLAIMER:
 * This sorting application is very limited for this specific test,
 * and is not recommended to be used for real sorting purposes
 *
 */




#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <time.h>
#include "uthreads.h"

#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"

#define NUM_THREADS 7
#define ARRAY_SIZE 8192
#define N ARRAY_SIZE
#define RUN 0
#define DONE 1

char thread_status[NUM_THREADS];
int array[ARRAY_SIZE];

int place_holder0[N], place_holder1[N / 2], place_holder2[N / 2];


void printArray();


int rand_tid()
{
    return rand() % NUM_THREADS;
}

void wait_next_quantum()
{
    int quantum = uthread_get_quantums(uthread_get_tid());
    while (uthread_get_quantums(uthread_get_tid()) == quantum)
    {}
    return;
}

// sort array[start..end-1]
void sort(int start, int end)
{
    wait_next_quantum();

    for (int i = start; i < end; i++)
    {
        for (int j = i + 1; j < end; j++)
        {
            if (array[i] > array[j])
            {
                int temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
        if (i == (end + start) / 2)
        {
            wait_next_quantum();
        }
    }

}


/*
 * merges array[start..end/2-1] and array[end/2..end-1]
 */
void merge(int start, int end, int* place_holder)
{

    int start1 = start, end1 = (end + start) / 2;
    int start2 = end1, end2 = end;
    int i = start1, j = start2;
    int k = 0;

    while (i < end1 && j < end2)
    {
        if (array[i] < array[j])
        {
            place_holder[k++] = array[i++];
        }
        else
        {
            place_holder[k++] = array[j++];
        }
    }

    wait_next_quantum();

    while (i < end1)
    {
        place_holder[k++] = array[i++];
    }
    while (j < end2)
    {
        place_holder[k++] = array[j++];
    }

    wait_next_quantum();

    // copy from place_holder to original array
    memcpy(array + start1, place_holder, sizeof(int) * (end2 - start1));

}

void sorting_thread(int start, int end)
{
    sort(start, end);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void thread3()
{
    sorting_thread(0, N / 4);
}

void thread4()
{
    sorting_thread(N / 4, N / 2);
}

void thread5()
{
    sorting_thread(N / 2, 3 * N / 4);
}

void thread6()
{
    sorting_thread(3 * N / 4, N);
}


void thread1()
{
    int t3 = uthread_spawn(thread3);
    int t4 = uthread_spawn(thread4);

    if (t3 == -1 || t4 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t3] == RUN || thread_status[t4] == RUN)
    {}

    merge(0, N / 2, place_holder1);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void thread2()
{
    int t5 = uthread_spawn(thread5);
    int t6 = uthread_spawn(thread6);

    if (t5 == -1 || t6 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t5] == RUN || thread_status[t6] == RUN)
    {}

    merge(N / 2, N, place_holder2);
    thread_status[uthread_get_tid()] = DONE;
    uthread_terminate(uthread_get_tid());
}

void printArray()
{
    for (int i = 0; i < N; i++)
    {
        printf("%d, ", array[i]);
    }
    printf("\n");
}


int main()
{
    printf(GRN "Test 1430: " RESET);
    fflush(stdout);

    srand(time(NULL));

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        array[i] = rand() % 4862;
    }


    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

	int q[2] = {10, 20};
	uthread_init(2);

    int t1 = uthread_spawn(thread1);
    int t2 = uthread_spawn(thread2);
    if (t1 == -1 || t2 == -1)
    {
        printf(RED "ERROR - thread spawn failed\n" RESET);
        uthread_terminate(0);
    }

    while (thread_status[t1] == RUN || thread_status[t2] == RUN)
    {}

    merge(0, N, place_holder0);

    // check that array is sorted
    for (int i = 0; i < N - 1; i++)
    {
        if (array[i] > array[i + 1])
        {
            printf(RED "ERROR - failed to sort the array\n" RESET);
            uthread_terminate(0);
        }
    }

    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);

}
#endif

#ifdef  TEST6
/**********************************************
 * Test 132: thread's signal mask is saved between switches (not including VTALRM)
 *
 * steps:
 * create three global sets of different signals (not including VTALRM) - set1, set2, set3
 * spawn threads 1,2,3
 * each thread K blocks setK and infinitely checks that his sigmask is setK
 *
 **********************************************/



#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "uthreads.h"


#define GRN "\e[32m"
#define RED "\x1B[31m"
#define RESET "\x1B[0m"


#define NUM_THREADS 4

#define RUN 0
#define DONE 1

sigset_t set1, set2, set3;

char thread_status[NUM_THREADS];

void halt()
{
    while (true)
    {}
}


int next_thread()
{
    return (uthread_get_tid() + 1) % NUM_THREADS;
}

void check_sig_mask(const sigset_t& expected)
{
    for (unsigned int i = 0; i <= 20; i++)
    {
        sigset_t actual;
        sigprocmask(0, NULL, &actual);
        if (memcmp(&expected, &actual, sizeof(sigset_t)) != 0)
        {
            printf(RED "ERROR - sigmask changed\n" RESET);
            exit(1);
        }

        // in the first 10 iterations let the thread stop because of sync / block.
        // in later iterations it will stop because of the timer
        if (i < 5)
        {
            //uthread_sync(next_thread());
        }
        else if (i < 10)
        {
            uthread_block(uthread_get_tid());
        }
        else
        {
            int quantum = uthread_get_quantums(uthread_get_tid());
            while (uthread_get_quantums(uthread_get_tid()) == quantum)
            {}
        }
    }
    thread_status[uthread_get_tid()] = DONE;
    halt();
}

void thread1()
{
    sigprocmask(SIG_BLOCK, &set1, NULL);
    check_sig_mask(set1);
}

void thread2()
{
    sigprocmask(SIG_BLOCK, &set2, NULL);
    check_sig_mask(set2);
}

void thread3()
{
    sigprocmask(SIG_BLOCK, &set3, NULL);
    check_sig_mask(set3);
}

bool all_done()
{
    bool res = true;
    for (int i = 1; i < NUM_THREADS; i++)
    {
        res = res && (thread_status[i] == DONE);
    }
    return res;
}

int main()
{
    printf(GRN "Test 132:  " RESET);
    fflush(stdout);

    sigemptyset(&set1);
    sigemptyset(&set2);
    sigemptyset(&set3);

    sigaddset(&set1, SIGBUS);
    sigaddset(&set1, SIGTERM);
    sigaddset(&set1, SIGRTMAX);
    sigaddset(&set1, SIGABRT);

    sigaddset(&set2, SIGUSR1);
    sigaddset(&set2, SIGSEGV);
    sigaddset(&set2, SIGUSR2);
    sigaddset(&set2, SIGPIPE);

    sigaddset(&set3, SIGTSTP);
    sigaddset(&set3, SIGTTIN);
    sigaddset(&set3, SIGTTOU);
    sigaddset(&set3, SIGBUS);

	int q[2] = {10, 20};
	uthread_init(2);

    for (int i = 1; i < NUM_THREADS; i++)
    {
        thread_status[i] = RUN;
    }

    int t1 = uthread_spawn(thread1);
    int t2 = uthread_spawn(thread2);
    int t3 = uthread_spawn(thread3);

    if (t1 == -1 || t2 == -1 || t3 == -1)
    {
        printf(RED "ERROR - threads spawning failed\n" RESET);
        exit(1);
    }


    int tid = 0;
    while (!all_done())
    {
        // resume all threads, as each one of them is blocking himself
        uthread_resume(tid);
        tid = (tid + 1) % NUM_THREADS;
    }


    printf(GRN "SUCCESS\n" RESET);
    uthread_terminate(0);
}
#endif
