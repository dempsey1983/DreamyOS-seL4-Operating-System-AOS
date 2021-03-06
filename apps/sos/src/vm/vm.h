/*
 * Virtual memory
 *
 * Cameron Lonsdale & Glenn McGuire
 */

#ifndef _VM_H_
#define _VM_H_

#include <limits.h>
#include <proc/proc.h>

/* Modes of access */
#define ACCESS_READ 0
#define ACCESS_WRITE 1

/* Maximum cap, as we use the left most bit to specify if the frame is resident or in the page file */
#define MAX_CAP_ID BIT((sizeof(seL4_CPtr) * CHAR_BIT) - 1) /* 2^31 */

/* Max id is also the mask to get the evicted bit */
#define EVICTED_BIT MAX_CAP_ID

/* Forward declaration of a process */
typedef struct _proc proc;

/* Struct for the top level of the page table. Known as a page directory */
typedef struct page_dir {
    seL4_Word *directory; /* Virtual address to the top level page directory */
    seL4_CPtr *kernel_page_table_caps; /* Array of in-kernel page table caps */
} page_directory;

/* WARNING: If this grows in size, algorithms will have to change */
/* 
 * Left most bit represents if the page is evicted or not.
 * If evicted (1), the id is the section in the pagefile where the page is stored
 * else (0), the value is the cap value.
 */
typedef struct {
    seL4_CPtr page; 
} page_table_entry;

/* 
 * Handle a vm fault.
 * @param pid, pid of the calling process
 */ 
void vm_fault(seL4_Word pid);

/*
 * Create a page directory
 * @returns pointer to page_directory on success, else NULL
 */
page_directory *page_directory_create(void);

/*
 * Destroy a page directory
 * @param dir, the page directory to destroy
 * @returns 0 on success, else 1
 */
int page_directory_destroy(page_directory *dir);

/*
 * Insert a page into the two level page table
 * @param directory, the page directory to insert into
 * @param vaddr, the virtual address of the page
 * @param sos_cap, the capability of the page created by sos
 * @param kernel_cap, a capability of the page table created by the kernel
 * @returns 0 on success, else 1
 */
int page_directory_insert(page_directory *directory, seL4_Word vaddr, seL4_CPtr sos_cap, seL4_CPtr kernel_cap);

/*
 * Given a vaddr, retrieve the cap for the page
 * @param directory, the page directory to insert into
 * @param vaddr, the virtual address of the page
 * @param cap, the cap for the page represented by vaddr
 * @returns 0 on success, else 1
 */
int page_directory_lookup(page_directory *dir, seL4_Word page_id, seL4_CPtr *cap);

/*
 * Given a vaddr, mark the page as evicted
 * @param directory, the page directory to insert into
 * @param page_id, the virtual address of the page
 * @param free_id, id in the pagefile where this page is now stored
 * @returns 0 on success, else 1
 */
int page_directory_evict(page_directory *dir, seL4_Word page_id, seL4_Word free_id);

/*
 * Translate a process virtual address to the sos vaddr of the frame.
 * The frame is mapped in if translation failed.
 * @param vaddr, the addr to translate
 * @param access_type, the type of access to the memory, for permissions checking
 * @returns sos virtual address on successl, else (seL4_Word)NULL
 */
seL4_Word vaddr_to_sos_vaddr(proc *curproc, seL4_Word vaddr, seL4_Word access_type);

/*
 * Copy in bytes from user process
 * @param curproc, the process to copy in from
 * @param dst, destination buffer, of size nbytes
 * @param vaddr_src, virtual source address
 * @param nbytes, the maximum number of bytes to copy
 * @returns 0 on success, else 1
 */
int copy_in(proc *curproc, void *dst, seL4_Word vaddr_src, seL4_Word nbytes);

/*
 * Copy out bytes to user process
 * @param curproc, the process to copy out to
 * @param dst, destination buffer, of size nbytes, in process vaddr
 * @param src, source address
 * @param nbytes, the maximum number of bytes to copy
 * @returns 0 on success, else 1
 */
int copy_out(proc *curproc, seL4_Word dst, char *src, seL4_Word nbytes);

/*
 * Given a vaddr, try to map in that page 
 * @param curproc, the process to map the page into
 * @param vaddr, the vaddr of the page to map in
 * @param access_type, the type of access requested to that memory
 * @param kvaddr[out], the kvaddr of the frame
 * @returns 0 on success, else 1
 */
int vm_map(proc *curproc, seL4_Word vaddr, seL4_Word access_type, seL4_Word *kvaddr);

/*
 * Given a process, counts the number of used pages
 * @param curproc, the proc to count the pages in the PD
 * @returns number of pages in the page table used
 */
unsigned page_directory_count(proc *curproc);

#endif /* _VM_H_ */
