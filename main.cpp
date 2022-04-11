/**
 * TEST1-6 : jona passed
 * test1-7 : drive passed
 * test8 : yuval arbel passed
 */

#define test8

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

#ifdef  test1
/*
 * test1.cpp
 *
 *	test suspends and resume
 *
 *  Created on: Apr 6, 2015
 *      Author: roigreenberg
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include "uthreads.h"
#include <iostream>

using namespace std;


void f (void)
{
    int i = 1;
    int j = 0;
    while(1)
    {
        if (i == uthread_get_quantums(uthread_get_tid()))
        {
            cout << "f" << "  q:  " << i << endl;
            if (i == 3 && j == 0)
            {
                j++;
                cout << "          f suspend by f" << endl;
                uthread_block(uthread_get_tid());
            }
            if (i == 6 && j == 1)
            {
                j++;
                cout << "          g resume by f" << endl;
                uthread_resume(2);
            }
            if (i == 8 && j == 2)
            {
                j++;
                cout << "          **f end**" << endl;
                uthread_terminate(uthread_get_tid());
                return;
            }
            i++;
        }
    }
}

void g (void)
{
    int i = 1;
    int j = 0;
    while(1)
    {
        if (i == uthread_get_quantums(uthread_get_tid()))
        {
            cout << "g" << "  q:  " << i << endl;
            if (i == 11 && j == 0)
            {
                j++;
                cout << "          **g end**" << endl;
                uthread_terminate(uthread_get_tid());
                return;
            }
            i++;
        }
    }
}


int main(void)
{
    int usecs = 10;
    if (uthread_init(usecs) == -1)
    {
        return 0;
    }

    int i = 1;
    int j = 0;

    while(1)
    {
        //int a = uthread_get_quantums(uthread_get_tid());
        //cout<<"quantums of thread number " << uthread_get_tid()<<" is " <<a<<std::endl;
        if (i == uthread_get_quantums(uthread_get_tid()))
        {
            cout << "m" << "  q:  " << i << endl;
            if (i == 3 && j == 0)
            {
                j++;
                cout << "          spawn f at (1) " << uthread_spawn(f) << endl;
                cout << "          spawn g at (2) " << uthread_spawn(g) << endl;
            }

            if (i == 6 && j == 1)
            {
                j++;
                cout << "          g suspend by main" << endl;
                uthread_block(2);
                cout << "          g suspend again by main" << endl;
                uthread_block(2);
            }
            if (i == 9 && j == 2)
            {
                j++;
                cout << "          f resume by main" << endl;
                uthread_resume(1);
                cout << "          f resume again by main" << endl;
                uthread_resume(1);
            }
            if (i == 13 && j == 3)
            {
                j++;
                cout << "          spawn f at (1) " << uthread_spawn(f) << endl;
                cout << "          f suspend by main" << endl;
                uthread_block(1);
            }
            if (i == 17 && j == 4)
            {
                j++;
                cout << "          spawn g at (2) " << uthread_spawn(g) << endl;
                cout << "          f terminate by main" << endl;
                uthread_terminate(1);
                cout << "          spawn f at (1) " << uthread_spawn(f) << endl;
                cout << "          f suspend by main" << endl;
                uthread_block(1);
            }
            if (i == 20 && j == 5)
            {
                j++;
                //cout << "i: " << i << endl;
                cout << "          ******end******" << endl;
                cout << "total quantums:  " << uthread_get_total_quantums() << endl;
                uthread_terminate(0);
                return 0;
            }
            i++;
        }
    }
    cout << "end" << endl;
    return 0;
}

#endif

#ifdef  test2
/*
 * test2.cpp
 *
 *  Created on: Apr 7, 2015
 *      Author: roigreenberg
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include "uthreads.h"
//#include "libuthread.a"
#include <iostream>

using namespace std;

void f (void)
{
    while(1);
}

int main(void)
{
	int q[2] = {10, 20};
	if (uthread_init(2) == -1)
    {
        return 0;
    }
    for (int i = 0; i < 100; i++)
        cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;

    uthread_terminate(5);

    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;

    uthread_terminate(15);
    uthread_terminate(25);
    uthread_terminate(35);
    uthread_terminate(45);
    uthread_terminate(55);
    uthread_terminate(65);
    uthread_terminate(75);
    uthread_terminate(85);

    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;
    cout << uthread_spawn(f) << endl;



    uthread_terminate(0);
    return 0;
}

#endif

#ifdef  test3
/*
 * test3.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: roigreenberg
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include "uthreads.h"
//#include "libuthread.a"
#include <iostream>

using namespace std;

void f(void){}

int main(void)
{
	int usecs = 10;
	if (uthread_init(usecs) == -1)
	{
        return 0;
    }


    uthread_terminate(-1);
    uthread_block(-1);
    uthread_resume(-1);
    uthread_get_quantums(-1);

    uthread_terminate(1);
    uthread_block(1);
    uthread_resume(1);
    uthread_get_quantums(1);

    uthread_block(0);

    uthread_spawn(f);
    uthread_terminate(1);

    uthread_terminate(1);
    uthread_block(1);
    uthread_resume(1);
    uthread_get_quantums(1);

    int w = -10;
	uthread_init(w);
	int m = 0;
    uthread_init(m);


    uthread_terminate(0);
    return 0;
}
#endif

#ifdef test4
/*
 * test4.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: roigreenberg
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include "uthreads.h"
//#include "libuthread.a"
#include <iostream>

using namespace std;

void f (void)
{
    while(1)
    {
//		cout << "f" << endl;
        uthread_block(1);
    }
}

void g (void)
{
    while(1)
    {
//		cout << "g" << endl;
        uthread_block(2);
    }
}

void h (void)
{
    while(1)
    {
//		cout << "h" << endl;
        uthread_block(3);
    }
}

int main(void)
{
	int q = 10;
	if (uthread_init(q) == -1)
    {
        return 0;
    }

    uthread_spawn(f);
    uthread_spawn(g);
    uthread_spawn(h);

    while(uthread_get_total_quantums() < 10)
    {
        uthread_resume(1);
        uthread_resume(2);
        uthread_resume(3);
    }

    cout << uthread_get_quantums(0) << " + " << endl;
    cout << uthread_get_quantums(1) << " + " << endl;
    cout << uthread_get_quantums(2) << " + " << endl;
    cout << uthread_get_quantums(3) << endl;
    cout << " = " << uthread_get_total_quantums() << endl;

    uthread_block(2);

    while(uthread_get_total_quantums() < 20)
    {
        uthread_resume(1);
        uthread_resume(3);
    }

    cout << uthread_get_quantums(0) << " + " << endl;
    cout << uthread_get_quantums(1) << " + " << endl;
    cout << uthread_get_quantums(2) << " + " << endl;
    cout << uthread_get_quantums(3) << endl;
    cout << " = " << uthread_get_total_quantums() << endl;

    uthread_resume(2);

    while(uthread_get_total_quantums() < 30)
    {
        uthread_resume(1);
        uthread_resume(2);
        uthread_resume(3);
    }

    cout << uthread_get_quantums(0) << " + " << endl;
    cout << uthread_get_quantums(1) << " + " << endl;
    cout << uthread_get_quantums(2) << " + " << endl;
    cout << uthread_get_quantums(3) << endl;
    cout << " = " << uthread_get_total_quantums() << endl;


    uthread_terminate(0);
    return 0;
}

#endif

#ifdef test5
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

	int q = 10;
	if (uthread_init(q) == -1)
	{
		error();
	}
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

#ifdef test6
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

	int q = 10;
	if (uthread_init(q) == -1)
	{
		return 0;
	}

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

#ifdef test7
//
// Created by eliasital
//

/**
 * boolean print (global var) :
 *     prints to screen if true, otherwise runs, but without any prints.
 *
 * printCount (function local variable) :
 *     Functions runs and writes 'in f(0)', 'in f(1)' and so on.
 *     The number in the brackets is the printCount var.
 *     A condition of 'if (printCount == x) then ...'
 *     states what will the function do when it will reach its 'x' cycle.
 *
 * Tests :
 *     1    Spawn functions
 *     2    Non-Main function terminates it self
 *     3    Non-Main function blocks Non-Main function
 *     4    Non-Main function goes to sleep
 *     5    Main terminates functions
 *     6    Main blocks function
 *     7    Main unblocks function
 *     8    Main removes blocked function
 *     9    Main terminates with functions that are alive
 *     10   Sanity check - Main blocks sleeping function (OK!)
 *          Sanity check - Main unblocks sleeping function (ERR!)
 *          Sanity check - Main blocks non-existent function (ERR!)
 *          Sanity check - Main asks to sleep (ERR!)
 *          Sanity check - Main asks to block himself (ERR!)
 *          Sanity check - Main asks to unblock sleeping function (ERR!)
 */

#include <array>
#include <iostream>
#include "uthreads.h"

#define QUANT 1000000
#define PRINTER 100000000

bool print = true;

using namespace std;

/**
 * A Simple function - runs and terminates itself in the end.
 */
void f()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in f(" << printCount << ")" << endl;}
            if (printCount == 7)
            {
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "f terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}

/**
 * A Simple function - runs and terminates itself in the end.
 */
void g()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in g(" << printCount << ")" << endl;}
            if (printCount == 13)
            {
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "g terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_h()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in h(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_i()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in i(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_j()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in j(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

///**
// * A simple function that sends it self to sleep.
// */
//void sleeper()
//{
//    int printCount = 0;
//    for (int i = 0; ; i++)
//    {
//        if (i%PRINTER == 0)
//        {
//            if (print) {cout << "in sleeper(" << printCount << ")" << endl;}
//
//            if (printCount == 15)
//            {
//                if (print) {cout << "sleeper went to sleep for 50 quants!" << endl;}
//                uthread_sleep(50);
//                if (print) {cout << "sleeper is awake again!" << endl;}
//            }
//
//            if (printCount == 57)
//            {
//                break;
//            }
//            printCount++;
//        }
//    }
//
//    if (print) {cout << "sleeper terminates" << endl;}
//    int me = uthread_get_tid();
//    uthread_terminate(me);
//}

/**
 * Blocks some thread and then unblocks its.
 * Blocks it self.
 */
void blocker()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in blocker(" << printCount << ")" << endl;}

            if (printCount == 3)
            {
                if (print) {cout << "blocker blocks f (tid = 1)!" << endl;}
                uthread_block(1);
            }

            if (printCount == 11)
            {
                if (print) {cout << "blocker unblocks f (tid = 1)!" << endl;}
                uthread_resume(1);
            }

            if (printCount == 40)
            {
                if (print) {cout << "blocker blocks himself!" << endl;}
                uthread_block(uthread_get_tid());
                if (print) {cout << "blocker was resumed!" << endl;}
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "blocker terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}


int main()
{

	int q = 10;
	int init = uthread_init(q);

    if (print) {cout << "MAIN INIT : " << init << endl << endl;}

    int fid = uthread_spawn(&f);
    if (print) {cout << "MAIN SPAWNS f : " << fid << endl;}

    int gid = uthread_spawn(&g);
    if (print) {cout << "MAIN SPAWNS g : " << gid << endl;}

    int hid = uthread_spawn(&neverEnding_h);
    if (print) {cout << "MAIN SPAWNS h : " << hid << endl;}

    int iid = uthread_spawn(&neverEnding_i);
    if (print) {cout << "MAIN SPAWNS i : " << iid << endl;}

    int jid = uthread_spawn(&neverEnding_j);
    if (print) {cout << "MAIN SPAWNS j : " << jid << endl;}

//    int sleeperid = uthread_spawn(&sleeper, 1);
//    if (print) {cout << "MAIN SPAWNS sleeper : " << sleeperid << endl;}

    int blockerid = uthread_spawn(&blocker);
    if (print) {cout << "MAIN SPAWNS blockerid : " << blockerid << endl;}

    if (print) {cout << endl << "MAIN RUNS!" << endl;}

    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "IN MAIN(" << printCount << ")" << endl;}

            if (printCount == 20)
            {
//                if (print) {cout << "GOOD-ERR Main blocks sleeping sleeper!" << endl;}
//                uthread_block(sleeperid);
//                if (print) {cout << "ERR Main unblocks sleeping sleeper!" << endl;}
//                uthread_resume(sleeperid);
                if (print) {cout << "ERR Main blocks non-existent 99!" << endl;}
                uthread_block(99);
                if (print) {cout << "ERR Main unblocks non-existent 101!" << endl;}
                uthread_block(101);
                if (print) {cout << "ERR Main asks to block himself!" << endl;}
                uthread_block(uthread_get_tid());
            }

            if (printCount == 30)
            {
                if (print) {cout << "Main removes h,i!" << endl;}
                uthread_terminate(hid);
                uthread_terminate(iid);

//                if (print) {cout << "ERR Main unblocks sleeping sleeper!" << endl;}
//                uthread_resume(sleeperid);
            }


            if (printCount == 45)
            {
                if (print) {cout << "Main unblocks blocker (which blocked himself)!" << endl;}
                uthread_resume(blockerid);
            }

            if (printCount == 52)
            {
                if (print) {cout << "Main blocks blocker!" << endl;}
                uthread_block(blockerid);
            }

            if (printCount == 58)
            {
                if (print) {cout << "Main removes blocker!" << endl;}
                uthread_terminate(blockerid);
            }

            if (printCount == 65)
            {
                break;
            }
            printCount++;
        }
    }

    if (print) {cout << "MAIN TERMINATES WILE j IS ALIVE" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}
#endif

#ifdef test8
#include <iostream>
#include "uthreads.h"
#include <unordered_map>

int a;

void wait_one_quantum ()
{
  a = a + 1;
  for (int j = 0; j < 8000000; ++j)
    {
      a = 1;
    }
}

void wait_for_test_end() {
  for (int i = 0; i < 200; ++i) {
      wait_one_quantum();
    }
}

void empty_func(){
  while (true) {}
}


void thread1(){
  for (int i = 0; i < 5; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-1: round-" << i << ", quantums-" << uthread_get_quantums(1) <<
                std::endl;
    }
  uthread_terminate (1);
}

void thread2(){
  for (int i = 0; i < 5; ++i)
    {
      wait_one_quantum();
      std::cout << "thread-2: round-" << i << ", quantums-" << uthread_get_quantums(2) <<
                std::endl;
    }
  uthread_terminate (2);
}

void thread3(){
  for (int i = 0; i < 5; ++i)
    {
      wait_one_quantum();
      wait_one_quantum();
      std::cout << "thread-3: round-" << i << ", quantums-" << uthread_get_quantums(3) <<
                std::endl;
    }
  uthread_terminate (3);
}

void thread4(){
  for (int i = 0; i < 5; ++i)
    {
      wait_one_quantum();
      wait_one_quantum();
      std::cout << "thread-4: round-" << i << ", quantums-" << uthread_get_quantums(4) <<
                std::endl;
    }
  uthread_terminate (4);
}

void thread5(){
  for (int i = 0; i < 5; ++i)
    {
      wait_one_quantum();
      std::cout << "thread-5: round-" << i << ", quantums-" << uthread_get_quantums(5) <<
                std::endl;
    }
  uthread_terminate (5);
}

/**
 * method to check the correct operation of switching between 2 threads.
 * each thread will print a message 5 times.
 */
void check_two_threads_running(){
  std::cout << "thread 1 and 2 will print 'thread-i' 5 times each" <<
            std::endl;
  int id1 = uthread_spawn (&thread1);
  int id2 = uthread_spawn (&thread2);
  if (id1!=1 || id2!=2){
      std::cout << "threads ids are not 1 and 2, but instead: " << id1
                << " and " << id2 << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "threads 1 and 2 finished successfully" << std::endl;
}

void thread1_block(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-1: round-" << i << ", quantums-" << uthread_get_quantums(1) <<
                std::endl;
      if (i == 3){
          std::cout << "blocking thread 2!!!" << std::endl;
          uthread_block (2);
        }
    }
  std::cout << "resuming thread 2!!!" << std::endl;
  uthread_resume (2);
  uthread_terminate (1);
}

void block_threads_test(){
  std::cout << "thread 1 will count to 8 and thread 2 to 5." << std::endl;
  int id1 = uthread_spawn (&thread1_block);
  int id2 = uthread_spawn (&thread2);
  if (id1!=1 || id2!=2){
      std::cout << "threads ids are not 1 and 2, but instead: " << id1
                << " and " << id2 << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "threads 1 and 2 finished successfully" << std::endl;
}

void thread1_sleep(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-1: round-" << i << ", quantums-" << uthread_get_quantums(1) <<
                std::endl;
      if (i == 3){
          std::cout << "putting thread 1 to sleep for 5 quantums." << std::endl;
          uthread_sleep(5);
        }
    }
  uthread_terminate (1);
}

void thread2_sleep(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum();
      std::cout << "thread-2: round-" << i << ", quantums-" << uthread_get_quantums(2) <<
                std::endl;
    }
  uthread_terminate (2);
}

/**
 * method to test the sleep for threads.
 */
void sleep_threads_test(){
  std::cout << "thread 1 and 2 will print 'thread-i' 8 times each" <<
            std::endl;
  int id1 = uthread_spawn (&thread1_sleep);
  int id2 = uthread_spawn (&thread2_sleep);
  if (id1!=1 || id2!=2){
      std::cout << "threads ids are not 1 and 2, but instead: " << id1
                << " and " << id2 << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "threads 1 and 2 finished successfully" << std::endl;
}

void thread1_sleep_block_before_wakeup(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-1: round-" << i << ", quantums-" << uthread_get_quantums(1) << std::endl;
      if (i == 3){
          std::cout << "putting thread 1 to sleep for 20 quantums" <<
          std::endl;
          int before = uthread_get_total_quantums();
          uthread_sleep(20);
          std::cout << "thread 1 is awake, it was out for " << uthread_get_total_quantums() - before << " quantums" << std::endl;
        }
    }
  uthread_terminate (1);
}

/**
 * resuming thread 1 before it wakes up from sleep
 */
void thread2_sleep_block_before_wakeup(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-2: round-" << i << ", quantums-" << uthread_get_quantums(1) << std::endl;
      if (i == 4){
          std::cout << "blocking thread 1" << std::endl;
          uthread_block(1);
        }
    }
  std::cout << "resuming thread 1" << std::endl;
  uthread_resume (1);
  uthread_terminate (2);
}

/**
 * test blocking threads while they are asleep, resuming before thread wakes up
 */
void test_block_sleeping_before_wakeup(){
  std::cout << "thread 1 and 2 will print 'thread-i' 8 times each" << std::endl;
  std::cout << "thread 1 should be blocked and resumed before waking up from its sleep" << std::endl;
  int id1 = uthread_spawn (&thread1_sleep_block_before_wakeup);
  int id2 = uthread_spawn (&thread2_sleep_block_before_wakeup);
  if (id1!=1 || id2!=2){
      std::cout << "threads ids are not 1 and 2, but instead: " << id1
                << " and " << id2 << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "threads 1 and 2 finished successfully" << std::endl;
}

void thread1_sleep_block_after_wakeup(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-1: round-" << i << ", quantums-" << uthread_get_quantums(1) << std::endl;
      if (i == 3){
          std::cout << "putting thread 1 to sleep for 10 quantums" << std::endl;
          int before = uthread_get_total_quantums();
          uthread_sleep(10);
          std::cout << "thread 1 is awake, it was out for " << uthread_get_total_quantums() - before << " quantums" << std::endl;
        }
    }
  uthread_terminate (1);
}

/**
 * resuming thread 1 after it has woken up
 */
void thread2_sleep_block_after_wakeup(){
  for (int i = 0; i < 8; ++i)
    {
      wait_one_quantum ();
      std::cout << "thread-2: round-" << i << ", quantum-" << uthread_get_quantums(1) <<
                std::endl;
      if (i == 4){
          std::cout << "blocking thread 1" << std::endl;
          uthread_block(1);
        }
    }
  uthread_sleep (10); //sleeping some more to make sure thread 1 will finish sleep before resuming
  std::cout << "resuming thread 1" << std::endl;
  uthread_resume (1);
  uthread_terminate (2);
}

/**
 * test blocking threads while they are asleep, resuming after thread wakes up
 */
void test_block_sleeping_after_wakeup(){
  std::cout << "thread 1 and 2 will print 'thread-i' 8 times each" << std::endl;
  std::cout << "thread 1 should be blocked while sleeping, and resumed when is awake" << std::endl;
  int id1 = uthread_spawn (&thread1_sleep_block_after_wakeup);
  int id2 = uthread_spawn (&thread2_sleep_block_after_wakeup);
  if (id1!=1 || id2!=2){
      std::cout << "threads ids are not 1 and 2, but instead: " << id1
                << " and " << id2 << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "threads 1 and 2 finished successfully" << std::endl;
}

void sleep_100(){
  uthread_sleep (500);
  while (true){}
}

void test_100_threads(){
  for (int i = 0; i < 100; ++i)
    {
      int id = uthread_spawn (&sleep_100);
      if (i==99){
          if (id!=-1){
              std::cout << "thread id is not -1" << std::endl;
              exit(1);
            }
          else {
              std::cout << "max limit reached - this is good!" << std::endl;
            }
        }
      else if (id!=i+1){
          std::cout << "wrong id, expected: "<< i+1<<", got: "<< id <<
                    std::endl;
          exit(1);
        }
    }
  wait_for_test_end();
  for (int i = 1; i < 100; ++i)
    {
      uthread_terminate (i);
    }

  std::cout << "finished spawning and terminating 100 threads successfully."
            << std::endl;
}

/**
 * test to check ids are allocated correctly
 */
void test_thread_id_management(){
  std::cout << "checking allocation of ids." << std::endl;
  int id1 = uthread_spawn (&empty_func);
  int id2 = uthread_spawn (&empty_func);
  int id3 = uthread_spawn (&empty_func);
  if (id1!=1 || id2!=2 || id3!=3){
      std::cout << "initial threads ids are not correct" << std::endl;
      exit(1);
    }

  uthread_terminate(2);
  int new_id = uthread_spawn (&empty_func);
  if (new_id!=2){
      std::cout << "terminated thread 2 and created a new one instead, it did not receive id=2" << std::endl;
      exit(1);
    }

  uthread_terminate(3);
  new_id = uthread_spawn (&empty_func);
  if (new_id!=3){
      std::cout << "terminated thread 3 and created a new one instead, it did not receive id=3" << std::endl;
      exit(1);
    }

  uthread_terminate (1);
  uthread_terminate (2);
  uthread_terminate (3);
  std::cout << "Success" << std::endl;


}

/**
 * testing switching between 5 different threads.
 */
void test_5_threads(){
  std::cout << "Spawning 5 threads, each thread will print thread-i 5 times" <<
            std::endl;
  int id1 = uthread_spawn (&thread1);
  int id2 = uthread_spawn (&thread2);
  int id3 = uthread_spawn (&thread3);
  int id4 = uthread_spawn (&thread4);
  int id5 = uthread_spawn (&thread5);
  if (id1!=1 || id2!=2 || id3!=3 || id4!=4 || id5!=5){
      std::cout << "threads ids are not correct" << std::endl;
      exit(1);
    }
  wait_for_test_end();
  std::cout << "Success" << std::endl;
}


int main ()
{
  //init uthread class
  uthread_init (1);

  std::cout << std::endl << "Test 1" << std::endl;
  check_two_threads_running();

  std::cout << std::endl << "Test 2" << std::endl;
  block_threads_test();

  std::cout << std::endl << "Test 3" << std::endl;
  sleep_threads_test();

  std::cout << std::endl << "Test 4" << std::endl;
  test_block_sleeping_before_wakeup();

  std::cout << std::endl << "Test 5" << std::endl;
  test_block_sleeping_after_wakeup();

  std::cout << std::endl << "Test 6" << std::endl;
  test_thread_id_management();

  std::cout << std::endl << "Test 7" << std::endl;
  test_5_threads();

  std::cout << std::endl << "Test 8" << std::endl;
  test_100_threads();

  uthread_terminate (0);
  std::cout << "ERR: this message should not be printed!!!!!!!" << std::endl;

  return 0;
}

#endif

