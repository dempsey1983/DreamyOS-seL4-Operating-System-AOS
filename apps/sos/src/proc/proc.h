/*
 * Process Management
 *
 * Cameron Lonsdale & Glenn McGuire
 */

#ifndef _PROC_H_
#define _PROC_H_

#include <sel4/sel4.h>
#include <cspace/cspace.h>

#include <vm/addrspace.h>
#include <vm/vm.h>
#include <vfs/file.h>
#include <utils/list.h>
#include <coro/picoro.h>

/* maximum saved proc name */
#define N_NAME 32

/* Maximum number of processes to support */
#define MAX_PROCS 32

/* process states enum */
typedef enum {
    ZOMBIE,
    RUNNING,
    BLOCKED /* Blocked on SOS */
} proc_states;

/* Process Struct */
typedef struct _proc {
    seL4_Word tcb_addr;
    seL4_TCB tcb_cap;               /* thread control block cap */
    seL4_CPtr ipc_buffer_cap;       /* ipc buffer cap */

    cspace_t *croot;                /* the cspace root */

    addrspace *p_addrspace;         /* the address space */
    fdtable *file_table;            /* file table ptr */
    list_t *children;               /* Linked list of children */

    pid_t waiting_on;               /* pid of the child proc is waiting on */
    coro waiting_coro;              /* Coroutine to resume when the wait is satisfied */

    pid_t ppid;                     /* parent pid */
    pid_t pid;                      /* pid of process */
    char *proc_name;                /* process name */
    int64_t stime;                  /* process start time */
    proc_states p_state;            /* enum of the process state */

    size_t blocked_ref;             /* Number of blocks on this process */
    bool kill_flag;                 /* Flag to specify if this process has received a kill signal */
    bool protected;                 /* Flag to specify if the process can be killed */
} proc;

/*
 * Bootstrap the init process
 * @returns pid of the init process
 */
pid_t proc_bootstrap(void);

/*
 * Start a process
 * @param _cpio_archive, the archive of binaries
 * @param app_name, executible name
 * @param fault_ep, endpoint for IPC
 * @return -1 on error, pid on success
 */
pid_t proc_start(char *_cpio_archive, char *app_name, seL4_CPtr fault_ep, pid_t parent_pid);

/*
 * Delete a process
 * Remove all metadata except the proc struct itself
 * @param victim, the proc to delete
 * @returns 0 on success, else 1
 */
int proc_delete(proc *victim);

/*
 * Destroy a process
 * @param current, the process to destroy
 */
void proc_destroy(proc *current);

/*
 * Check if pid is a child of curproc
 * @param curproc, parent process
 * @param pid, the pid of the process to test
 * @return boolean if pid is a child of curproc
 */
bool proc_is_child(proc *curproc, pid_t pid);

/*
 * Check if parent is waiting on child
 * @param parent, parent process
 * @param child, child process
 * @returns true or false
 */
bool proc_is_waiting(proc *parent, proc *child);

/* 
 * Get proc structure given pid
 * @param pid, the pid of the process
 * @returns process struct pointer
 */
proc *get_proc(pid_t pid);

/*
 * Change proc state
 * @param pid, the pid of the process to mark
 * @param state, the new state of the proc
 */
void proc_mark(proc *curproc, proc_states state);

/*
 * Reparent childre from curproc to pid
 * @param cur_parent, the current parent
 * @param new_parent, pid of the new parent
 * @returns 0 on success, else 1
 */
int proc_reparent_children(proc *cur_parent, pid_t new_parent);

#endif /* _PROC_H_ */
