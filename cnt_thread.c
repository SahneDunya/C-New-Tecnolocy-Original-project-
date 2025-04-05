#include <stdio.h>
#include <stdlib.h>
#include "cnt_thread.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// Platforma özgü kodlar (örnek olarak Windows ve Linux)

#ifdef _WIN32

Thread* create_thread(ThreadFunction function, void* arg) {
    Thread* thread = (Thread*)malloc(sizeof(Thread));
    thread->thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, arg, 0, &thread->thread_id);
    if (thread->thread_handle == NULL) {
        free(thread);
        return NULL;
    }
    return thread;
}

int wait_for_thread(Thread* thread) {
    WaitForSingleObject(thread->thread_handle, INFINITE);
    DWORD exit_code;
    GetExitCodeThread(thread->thread_handle, &exit_code);
    return exit_code;
}

bool terminate_thread(Thread* thread) {
    return TerminateThread(thread->thread_handle, 0);
}

#else

Thread* create_thread(ThreadFunction function, void* arg) {
    Thread* thread = (Thread*)malloc(sizeof(Thread));
    if (pthread_create(&thread->thread_id, NULL, function, arg) != 0) {
        free(thread);
        return NULL;
    }
    return thread;
}

int wait_for_thread(Thread* thread) {
    void* exit_code;
    pthread_join(thread->thread_id, &exit_code);
    return (int)(intptr_t)exit_code;
}

bool terminate_thread(Thread* thread) {
    return pthread_cancel(thread->thread_id) == 0;
}

#endif

void free_thread(Thread* thread) {
    free(thread);
}