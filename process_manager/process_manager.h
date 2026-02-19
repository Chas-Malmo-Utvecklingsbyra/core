#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

#define PROCESS_MANAGER_MAX_PROCESSES 16
#define PROCESS_MANAGER_MAX_NAME_LENGTH 64

typedef int (*ProcessEntryPoint)(void *context);

typedef enum
{
    PROCESS_STATUS_NOT_STARTED,
    PROCESS_STATUS_RUNNING,
    PROCESS_STATUS_STOPPED,
    PROCESS_STATUS_FAILED
} ProcessStatus;

/**
 * @brief Structure representing a managed process, including its PID, name, status, and IPC pipes.
 * @param pid Process ID of the child process.
 * @param name Name of the process for easier identification.
 * @param status Current status of the process (running, stopped, etc.).
 * @param pipe_parent_to_child File descriptors for the pipe from parent to child (if IPC is enabled).
 * @param pipe_child_to_parent File descriptors for the pipe from child to parent (if IPC is enabled).
 * @param exit_code Exit code of the process after it has terminated.
 * @param has_pipes Indicates whether this process has IPC pipes set up.
 */
typedef struct
{
    pid_t pid;
    char name[PROCESS_MANAGER_MAX_NAME_LENGTH];
    ProcessStatus status;
    int pipe_parent_to_child[2];
    int pipe_child_to_parent[2];
    int exit_code;
    bool has_pipes;
} ManagedProcess;

/**
 * @brief Structure representing the process manager, which keeps track of all managed processes.
 * @param processes Array of managed processes.
 * @param process_count Current number of managed processes.
 * @param initialized Indicates whether the process manager has been initialized.
 * @param context Optional context pointer that can be used for logging or other purposes.
 */
typedef struct
{
    ManagedProcess processes[PROCESS_MANAGER_MAX_PROCESSES];
    size_t process_count;
    bool initialized;
    void *context;
} ProcessManager;

/**
 * @brief Initializes the process manager.
 * @param manager Pointer to the ProcessManager instance to initialize.
 * @param context Optional context pointer (e.g., for logging).
 * @return true on success, false on failure.
 */
bool ProcessManager_Init(ProcessManager *manager, void *context);

/**
 * @brief Destroys the process manager, cleaning up any resources.
 * @param manager Pointer to the ProcessManager instance to destroy.
 * @return void.
 */
void ProcessManager_Destroy(ProcessManager *manager);

/**
 * @brief Spawns a new process with the given entry point and context.
 * @param manager Pointer to the ProcessManager instance.
 * @param name Name of the process for identification.
 * @param entry Function pointer to the process entry point (must match ProcessEntryPoint signature).
 * @param context Context pointer to pass to the process entry point.
 * @param create_pipes Whether to create IPC pipes for communication with the child process.
 * @return PID of the spawned process on success, -1 on failure.
 */
pid_t ProcessManager_Spawn(ProcessManager *manager, const char *name, ProcessEntryPoint entry, void *context, bool create_pipes);

pid_t ProcessManager_SpawnByExecutable(ProcessManager *manager, const char *name, const char *executable_path, char *const args[], bool create_pipes);

/**
 * @brief Retrieves a managed process by its PID.
 * @param manager Pointer to the ProcessManager instance.
 * @param pid PID of the process to retrieve.
 * @return Pointer to the ManagedProcess if found, NULL if not found or on error.
 */
ManagedProcess *ProcessManager_GetByPID(ProcessManager *manager, pid_t pid);

/**
 * @brief Retrieves a managed process by its name.
 * @param manager Pointer to the ProcessManager instance.
 * @param name Name of the process to retrieve.
 * @return Pointer to the ManagedProcess if found, NULL if not found or on error.
 */
ManagedProcess *ProcessManager_GetByName(ProcessManager *manager, const char *name);

/**
 * @brief Gets the count of currently running processes.
 * @param manager Pointer to the ProcessManager instance.
 * @return Number of running processes.
 */
size_t ProcessManager_GetRunningCount(ProcessManager *manager);

/**
 * @brief Sends a signal to a process by its PID.
 * @param manager Pointer to the ProcessManager instance.
 * @param pid PID of the process to send the signal to.
 * @param signal Signal number to send (e.g., SIGTERM).
 * @return true on success, false on failure.
 */
bool ProcessManager_SendSignal(ProcessManager *manager, pid_t pid, int signal);
/**
 * @brief Sends a signal to a process by its name.
 * @param manager Pointer to the ProcessManager instance.
 * @param name Name of the process to send the signal to.
 * @param signal Signal number to send (e.g., SIGTERM).
 * @return true on success, false on failure.
 */
bool ProcessManager_SendSignalByName(ProcessManager *manager, const char *name, int signal);
/**
 * @brief Terminates all running processes managed by the process manager.
 * @param manager Pointer to the ProcessManager instance.
 * @return true if all processes were successfully signaled for termination, false if any failed.
 */
bool ProcessManager_TerminateAll(ProcessManager *manager);

// Pipe/IPC operations
bool ProcessManager_WriteToChild(ManagedProcess *process, const void *data, size_t size);
ssize_t ProcessManager_ReadFromChild(ManagedProcess *process, void *buffer, size_t size);
bool ProcessManager_WriteToParent(ManagedProcess *process, const void *data, size_t size);
ssize_t ProcessManager_ReadFromParent(ManagedProcess *process, void *buffer, size_t size);

// Process monitoring
bool ProcessManager_WaitForProcess(ProcessManager *manager, pid_t pid, int *status);
bool ProcessManager_CheckProcess(ProcessManager *manager, pid_t pid); // Non-blocking check
void ProcessManager_WaitForAll(ProcessManager *manager);

// Utility
void ProcessManager_PrintStatus(ProcessManager *manager);
bool ProcessManager_IsParent(void); // Returns true if calling process is parent
bool ProcessManager_IsChild(void);  // Returns true if calling process is child

#endif // PROCESS_MANAGER_H