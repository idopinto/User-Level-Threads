## User-Level Threads Library (uthreads)
This is a library for creating and managing user-level threads in C.

It was created as part of the Hebrew University OS course,

and it allows users to create threads, manage their execution, and control their behavior.

# Main API
## Initialization
The first function that needs to be called is uthread_init(int quantum_usecs).

It initializes the thread library and takes as input the length of a program quantum in microseconds. 

## Creating a new thread
To create a new thread, use uthread_spawn(thread_entry_point entry_point).

The thread is added to the end of the READY threads list. 

## Terminating a thread
To terminate a thread, use uthread_terminate(int tid). 

It terminates the thread with ID tid and deletes it from all relevant control structures.

## Blocking a thread
To block a thread, use uthread_block(int tid). 

It blocks the thread with ID tid.
The thread may be resumed later using uthread_resume.

See Docs for more information.

## Resuming a blocked thread
To resume a blocked thread, use uthread_resume(int tid). 

It resumes a blocked thread with ID tid and moves it to the READY state.

See Docs for more information.


## Sleeping a thread
To block the RUNNING thread for a specified number of quantums, use uthread_sleep(int num_quantums).

See Docs for more information.

