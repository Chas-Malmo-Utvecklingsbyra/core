#define _POSIX_SOURCE
#include "process_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "logger/logger.h"
#include "string/strdup.h"

static pid_t g_initialize_pid = 0;

bool ProcessManager_RemoveProcess(ProcessManager *manager, pid_t pid)
{
     if(manager == NULL || !manager->initialized)
        return false;
        
    for (size_t i = 0; i < manager->process_count; i++)
    {
        if (manager->processes[i].pid == pid)
        {
            LOG_WRITE((Logger *)manager->context, "Removing process with PID %d from manager", pid);
            for (size_t j = i; j < manager->process_count - 1; j++)
            {
                manager->processes[j] = manager->processes[j + 1];
            }
            memset(&manager->processes[manager->process_count - 1], 0, sizeof(ManagedProcess));
            manager->process_count--;
            return true;
        }
    }
    return false;
}

bool ProcessManager_Init(ProcessManager *manager, void *context)
{
    if (manager == NULL) return false;

    memset(manager, 0, sizeof(ProcessManager));
    manager->process_count = 0;
    manager->initialized = true;
    manager->context = (Logger *)context ? context : NULL;

    g_initialize_pid = getpid();

    return true;
}

void ProcessManager_Destroy(ProcessManager *manager)
{
    if (manager == NULL || !manager->initialized)
        return;

    // Close pipes
    for (size_t i = 0; i < manager->process_count; i++)
    {
        ManagedProcess *proc = &manager->processes[i];
        if (proc->has_pipes)
        {
            close(proc->pipe_parent_to_child[0]);
            close(proc->pipe_parent_to_child[1]);
            close(proc->pipe_child_to_parent[0]);
            close(proc->pipe_child_to_parent[1]);
        }
    }

    manager->initialized = false;
}

pid_t ProcessManager_Spawn(ProcessManager *manager, const char *name, ProcessEntryPoint entry, void *context, bool create_pipes)
{
    if (manager == NULL || !manager->initialized || entry == NULL)
        return -1;

    Logger *logger = (Logger *)manager->context; 
    
    if (manager->process_count >= PROCESS_MANAGER_MAX_PROCESSES)
    {
        if (logger != NULL)
            Logger_Write(logger,"Cannot spawn process %s: max process limit reached", name);
        return -1;
    }

    ManagedProcess *proc = &manager->processes[manager->process_count];
    memset(proc, 0, sizeof(ManagedProcess));

    strncpy(proc->name, name ? name : "unnamed", PROCESS_MANAGER_MAX_NAME_LENGTH - 1);
    proc->has_pipes = create_pipes;

    // Create pipes if requested
    if (create_pipes)
    {
        if (pipe(proc->pipe_parent_to_child) == -1)
        {
            if (logger != NULL)
                Logger_Write(logger, "%s", "Failed to create parent->child pipe");
            return -1;
        }

        if (pipe(proc->pipe_child_to_parent) == -1)
        {
            if (logger != NULL)
                Logger_Write(logger, "%s", "Failed to create child->parent pipe");
            close(proc->pipe_parent_to_child[0]);
            close(proc->pipe_parent_to_child[1]);
            return -1;
        }
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        // Fork failed
        if (logger != NULL)
            Logger_Write(logger,"Failed to fork process %s: %s", name ? name : "unnamed", strerror(errno));
        if (create_pipes)
        {
            close(proc->pipe_parent_to_child[0]);
            close(proc->pipe_parent_to_child[1]);
            close(proc->pipe_child_to_parent[0]);
            close(proc->pipe_child_to_parent[1]);
        }
        return -1;
    }
    else if (pid == 0)
    {
        // Child process
        if (create_pipes)
        {
            // Close unused pipe ends
            close(proc->pipe_parent_to_child[1]); // Close write end of parent->child
            close(proc->pipe_child_to_parent[0]); // Close read end of child->parent
        }

        int exit_code = entry(context);
        exit(exit_code);
    }
    else
    {
        // Parent process
        if (create_pipes)
        {
            // Close unused pipe ends
            close(proc->pipe_parent_to_child[0]); // Close read end of parent->child
            close(proc->pipe_child_to_parent[1]); // Close write end of child->parent
        }

        proc->pid = pid;
        proc->status = PROCESS_STATUS_RUNNING;
        manager->process_count++;

        if (logger != NULL)
            Logger_Write(logger, "Spawned process %s with PID %d", proc->name, pid);

        return pid;
    }

    return -1;
}

pid_t ProcessManager_SpawnByExecutable(ProcessManager *manager, const char *name, const char *executable_path, char *const args[], bool create_pipes)
{
    if (manager == NULL || !manager->initialized || name == NULL || executable_path == NULL || args == NULL)
        return -1;

    Logger *logger = (Logger *)manager->context;
        
    if (manager->process_count >= PROCESS_MANAGER_MAX_PROCESSES)
    {
        if (logger != NULL)
            Logger_Write(logger, "Cannot spawn process %s: max process limit reached", name);
        return -1;
    }

    ManagedProcess *proc = &manager->processes[manager->process_count];
    memset(proc, 0, sizeof(ManagedProcess));

    strncpy(proc->name, name ? name : "unnamed", PROCESS_MANAGER_MAX_NAME_LENGTH - 1);
    proc->has_pipes = create_pipes;

    // Create pipes if requested
    if (create_pipes)
    {
        if (pipe(proc->pipe_parent_to_child) == -1)
        {
            if (logger != NULL)
                Logger_Write(logger, "%s", "Failed to create parent->child pipe");
            return -1;
        }

        if (pipe(proc->pipe_child_to_parent) == -1)
        {
            if (logger != NULL)
                Logger_Write(logger, "%s", "Failed to create child->parent pipe");
            close(proc->pipe_parent_to_child[0]);
            close(proc->pipe_parent_to_child[1]);
            return -1;
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process - count args and prepare for execv
        int argc = 0;
        while (args[argc] != NULL)
        {
            argc++;
        }

        // Allocate: program name + args + pipe FDs + NULL terminator
        // If create_pipes is true, we need 2 extra args for the FDs but each arg is two strings and it needs to be null-terminated
        char **exec_args = NULL;
        if (create_pipes)
        {
            exec_args = calloc(argc + 6, sizeof(char *));
        }
        else
        {
            exec_args = calloc(argc + 2, sizeof(char *));
        }
        
        if (exec_args == NULL)
        {
            fprintf(stderr, "Failed to allocate exec_args\n");
            exit(-1);
        }
        
        exec_args[0] = strdup(executable_path);
        for (int i = 0; i < argc; i++)
        {
            exec_args[i + 1] = args[i];
        }
        if (create_pipes)
        {
            char read_fd_arg[32] = "--read-fd"; // proc->pipe_parent_to_child[0]
            char write_fd_arg[32] = "--write-fd"; // proc->pipe_child_to_parent[1]
            
            exec_args[argc + 1] = strdup(read_fd_arg);
            sprintf(read_fd_arg, "%d", proc->pipe_parent_to_child[0]);
            exec_args[argc + 2] = strdup(read_fd_arg);

            exec_args[argc + 3] = strdup(write_fd_arg);
            sprintf(write_fd_arg, "%d", proc->pipe_child_to_parent[1]);
            exec_args[argc + 4] = strdup(write_fd_arg);
            
            exec_args[argc + 5] = NULL;
        }
        else
        {
            exec_args[argc + 1] = NULL;
        }
        
        execv(executable_path, exec_args);

        // If execv returns, it failed
        perror("execv failed");
        exit(-1);
    }
    else if (pid > 0)
    {
        // Parent process - register the child process
        if (create_pipes)
        {
            // Close unused pipe ends
            close(proc->pipe_parent_to_child[0]); // Close read end of parent->child
            close(proc->pipe_child_to_parent[1]); // Close write end of child->parent
        }

        proc->pid = pid;
        proc->status = PROCESS_STATUS_RUNNING;
        manager->process_count++;

        if (logger != NULL)
            Logger_Write(logger, "Spawned executable process with PID %d for command: %s", pid, executable_path);

        return pid;
    }
    else
    {
        // Fork failed
        if (logger != NULL)
            Logger_Write(logger, "Failed to fork for fetcher process");
        if (create_pipes)
        {
            close(proc->pipe_parent_to_child[0]);
            close(proc->pipe_parent_to_child[1]);
            close(proc->pipe_child_to_parent[0]);
            close(proc->pipe_child_to_parent[1]);
        }
        return -1;
    }
}

pid_t ProcessManager_ResstartProcess(ProcessManager *manager, pid_t pid, const char *name, const char *executable_path, char *const args[], bool create_pipes)
{
    if (manager == NULL || !manager->initialized || pid <= 0 || name == NULL || executable_path == NULL || args == NULL)
        return -1;

    Logger *logger = (Logger *)manager->context;

    LOG_WRITE(logger, "Restarting process with PID %d using executable: %s", pid, executable_path);

    ManagedProcess *proc = ProcessManager_GetByPID(manager, pid);
    if (proc == NULL)
    {
        LOG_WRITE(logger, "Process with PID %d not found for restart", pid);
        return false;
    }

    ProcessManager_SendSignal(manager, pid, SIGTERM);

    int status;
    if (!ProcessManager_WaitForProcess(manager, pid, &status))
        LOG_WRITE(logger, "Failed to wait for process with PID %d to exit", pid);

    if (ProcessManager_RemoveProcess(manager, pid) == false)
        LOG_WRITE(logger, "Failed to remove process with PID %d from manager", pid);
    
    pid_t new_pid = ProcessManager_SpawnByExecutable(manager, name, executable_path, args, create_pipes);
    if (new_pid < 0)    {
        LOG_WRITE(logger, "Failed to spawn new process for restart with executable: %s", executable_path);
        return -1;
    }

    return new_pid;
}

ManagedProcess *ProcessManager_GetByPID(ProcessManager *manager, pid_t pid)
{
    if (manager == NULL || !manager->initialized)
        return NULL;

    for (size_t i = 0; i < manager->process_count; i++)
    {
        if (manager->processes[i].pid == pid)
            return &manager->processes[i];
    }

    return NULL;
}

ManagedProcess *ProcessManager_GetByName(ProcessManager *manager, const char *name)
{
    if (manager == NULL || !manager->initialized || name == NULL)
        return NULL;

    for (size_t i = 0; i < manager->process_count; i++)
    {
        if (strncmp(manager->processes[i].name, name, PROCESS_MANAGER_MAX_NAME_LENGTH) == 0)
            return &manager->processes[i];
    }

    return NULL;
}

size_t ProcessManager_GetRunningCount(ProcessManager *manager)
{
    if (manager == NULL || !manager->initialized)
        return 0;

    size_t count = 0;
    for (size_t i = 0; i < manager->process_count; i++)
    {
        if (manager->processes[i].status == PROCESS_STATUS_RUNNING)
            count++;
    }

    return count;
}

bool ProcessManager_SendSignal(ProcessManager *manager, pid_t pid, int signal)
{
    if (manager == NULL || !manager->initialized)
        return false;

    Logger *logger = (Logger *)manager->context;
    ManagedProcess *proc = ProcessManager_GetByPID(manager, pid);
    if (proc == NULL)
    {
        Logger_Write(logger, "[ProcessManager] Process with PID %d not found", pid);
        return false;
    }

    if (kill(pid, signal) == -1)
    {
        perror("[ProcessManager] Failed to send signal");
        return false;
    }

    Logger_Write(logger, "Sent signal %d to process '%s' (PID %d)", signal, proc->name, pid);
    return true;
}

bool ProcessManager_SendSignalByName(ProcessManager *manager, const char *name, int signal)
{
    ManagedProcess *proc = ProcessManager_GetByName(manager, name);
    if (proc == NULL)
        return false;

    return ProcessManager_SendSignal(manager, proc->pid, signal);
}

bool ProcessManager_TerminateAll(ProcessManager *manager)
{
    if (manager == NULL || !manager->initialized)
        return false;

    bool all_success = true;

    for (size_t i = 0; i < manager->process_count; i++)
    {
        ManagedProcess *proc = &manager->processes[i];
        if (proc->status == PROCESS_STATUS_RUNNING)
        {
            if (!ProcessManager_SendSignal(manager, proc->pid, SIGTERM))
                all_success = false;
        }
    }

    return all_success;
}

bool ProcessManager_WriteToChild(ProcessManager *manager, pid_t pid, const void *data, size_t size)
{
    if (manager == NULL || !manager->initialized || data == NULL)
        return false;
        
    ManagedProcess *process = ProcessManager_GetByPID(manager, pid);
    
    if (process == NULL || !process->has_pipes || data == NULL)
        return false;

    ssize_t written = write(process->pipe_parent_to_child[1], data, size);
    return written == (ssize_t)size;
}

ssize_t ProcessManager_ReadFromChild(ProcessManager *manager, pid_t pid, void *buffer, size_t size)
{
    if (manager == NULL || !manager->initialized || buffer == NULL)
        return -1;
        
    ManagedProcess *process = ProcessManager_GetByPID(manager, pid);
    if (process == NULL || !process->has_pipes)
        return -1;
    
    fcntl(process->pipe_child_to_parent[0], F_SETFL, O_NONBLOCK);
    return read(process->pipe_child_to_parent[0], buffer, size);
}

bool ProcessManager_WriteToParent(ManagedProcess *process, const void *data, size_t size)
{
    if (process == NULL || !process->has_pipes || data == NULL)
        return false;

    ssize_t written = write(process->pipe_child_to_parent[1], data, size);
    return written == (ssize_t)size;
}

ssize_t ProcessManager_ReadFromParent(ManagedProcess *process, void *buffer, size_t size)
{
    if (process == NULL || !process->has_pipes || buffer == NULL)
        return -1;

    return read(process->pipe_parent_to_child[0], buffer, size);
}

bool ProcessManager_WaitForProcess(ProcessManager *manager, pid_t pid, int *status)
{
    if (manager == NULL || !manager->initialized)
        return false;

    ManagedProcess *proc = ProcessManager_GetByPID(manager, pid);
    if (proc == NULL)
        return false;

    int stat_loc;
    pid_t result = waitpid(pid, &stat_loc, 0);

    if (result == -1)
    {
        perror("[ProcessManager] waitpid failed");
        proc->status = PROCESS_STATUS_FAILED;
        return false;
    }

    if (status != NULL)
        *status = stat_loc;

    proc->exit_code = WEXITSTATUS(stat_loc);
    proc->status = PROCESS_STATUS_STOPPED;

    printf("[ProcessManager] Process '%s' (PID %d) exited with code %d\n",
           proc->name, pid, proc->exit_code);

    return true;
}

bool ProcessManager_CheckProcess(ProcessManager *manager, pid_t pid)
{
    if (manager == NULL || !manager->initialized)
        return false;

    ManagedProcess *proc = ProcessManager_GetByPID(manager, pid);
    if (proc == NULL)
        return false;

    int stat_loc;
    pid_t result = waitpid(pid, &stat_loc, WNOHANG); // Non-blocking

    if (result == 0)
    {
        // Process still running
        return true;
    }
    else if (result == pid)
    {
        // Process has terminated
        proc->exit_code = WEXITSTATUS(stat_loc);
        proc->status = PROCESS_STATUS_STOPPED;
        return false;
    }
    else
    {
        // Error
        proc->status = PROCESS_STATUS_FAILED;
        return false;
    }
}

void ProcessManager_WaitForAll(ProcessManager *manager)
{
    if (manager == NULL || !manager->initialized)
        return;

    for (size_t i = 0; i < manager->process_count; i++)
    {
        ManagedProcess *proc = &manager->processes[i];
        if (proc->status == PROCESS_STATUS_RUNNING)
        {
            ProcessManager_WaitForProcess(manager, proc->pid, NULL);
        }
    }
}

void ProcessManager_PrintStatus(ProcessManager *manager)
{
    if (manager == NULL || !manager->initialized)
        return;

    printf("=== Process Manager Status ===\n");
    printf("Total processes: %zu\n", manager->process_count);
    printf("Running: %zu\n", ProcessManager_GetRunningCount(manager));
    printf("\nProcesses:\n");

    for (size_t i = 0; i < manager->process_count; i++)
    {
        ManagedProcess *proc = &manager->processes[i];
        const char *status_str;

        switch (proc->status)
        {
        case PROCESS_STATUS_NOT_STARTED:
            status_str = "NOT_STARTED";
            break;
        case PROCESS_STATUS_RUNNING:
            status_str = "RUNNING";
            break;
        case PROCESS_STATUS_STOPPED:
            status_str = "STOPPED";
            break;
        case PROCESS_STATUS_FAILED:
            status_str = "FAILED";
            break;
        default:
            status_str = "UNKNOWN";
            break;
        }

        printf("  [%zu] %s (PID: %d) - %s", i, proc->name, proc->pid, status_str);
        if (proc->status == PROCESS_STATUS_STOPPED)
            printf(" (exit code: %d)", proc->exit_code);
        if (proc->has_pipes)
            printf(" [IPC enabled]");
        printf("\n");
    }
    printf("==============================\n");
}

bool ProcessManager_IsParent(void)
{
    return getpid() == g_initialize_pid;
}

bool ProcessManager_IsChild(void)
{
    return getpid() != g_initialize_pid && g_initialize_pid != 0;
}