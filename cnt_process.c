#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_process.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

// Platforma özgü kodlar (örnek olarak Windows ve Linux)

#ifdef _WIN32

Process* create_process(const char* command, char** args) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char command_line[1024];
    strcpy(command_line, command);
    if (args != NULL) {
        for (int i = 0; args[i] != NULL; i++) {
            strcat(command_line, " ");
            strcat(command_line, args[i]);
        }
    }

    if (!CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return NULL;
    }

    Process* process = (Process*)malloc(sizeof(Process));
    process->pid = pi.dwProcessId;
    // ... diğer işlem bilgilerini ayarlayın ...

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return process;
}

int wait_for_process(Process* process) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, process->pid);
    if (hProcess == NULL) {
        return -1;
    }

    WaitForSingleObject(hProcess, INFINITE);

    DWORD exit_code;
    GetExitCodeProcess(hProcess, &exit_code);

    CloseHandle(hProcess);
    return exit_code;
}

bool terminate_process(Process* process) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process->pid);
    if (hProcess == NULL) {
        return false;
    }

    bool result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}

Process* get_process_info(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return NULL;
    }

    // ... işlem bilgilerini alın ...

    CloseHandle(hProcess);
    return NULL; // Örnek için NULL döndürülüyor
}

#else

Process* create_process(const char* command, char** args) {
    pid_t pid = fork();
    if (pid == -1) {
        return NULL;
    }

    if (pid == 0) {
        // Child process
        if (execvp(command, args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process
    Process* process = (Process*)malloc(sizeof(Process));
    process->pid = pid;
    // ... diğer işlem bilgilerini ayarlayın ...
    return process;
}

int wait_for_process(Process* process) {
    int status;
    waitpid(process->pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

bool terminate_process(Process* process) {
    return kill(process->pid, SIGTERM) == 0;
}

Process* get_process_info(int pid) {
    // ... /proc dosya sistemini kullanarak işlem bilgilerini alın ...
    return NULL; // Örnek için NULL döndürülüyor
}

#endif

void free_process(Process* process) {
    free(process);
}