/*
 * Frametable
 *
 * Glenn McGuire & Cameron Lonsdale
 */

#ifndef _FRAMETABLE_H_
#define _FRAMETABLE_H_

#include "pager.h"

/* Individual frame */
typedef struct {
    seL4_CPtr cap; /* The cap for the frame */
    enum chance_type chance; /* Second page replacement status */
    seL4_Word pid; /* ID of the process this page is mapped into */
    seL4_Word page_id; /* Page id of the process vaddr this page is mapped into */
} frame_entry;

/*
 * Initialise the frame table.
 * @param paddr, the physical address of the frame_table
 * @param size_in_bits, the size in bits of the frame_table
 * @param low, low address of the UT memory region
 * @param high, high address of the UT memory region
 * @return 0 on success, else 1
 */
int frame_table_init(seL4_Word paddr, seL4_Word size_in_bits, seL4_Word low, seL4_Word high);

/*
 * Reserve a physical frame.
 * @param[out] virtual address of the frame
 * @returns ID of the frame on success, else -1
 */
seL4_Word frame_alloc(seL4_Word *vaddr);

/*
 * Free an allocated frame.
 * @param frame_id of the frame to be freed.
 */
void frame_free(seL4_Word vaddr);

/*
 * Return the capabilty for a frame.
 * @param frame_id, id of the frame to lookup
 * @return capability of the frame, NULL on error.
 */
seL4_ARM_Page frame_table_get_capability(seL4_Word frame_id);

/*
 * Given a frame index, return the sos vaddr of the frame.
 * @param frame_id, id of the frame
 * @returns sos_vaddr for that frame
 */
seL4_Word frame_table_index_to_sos_vaddr(seL4_Word frame_id);

/*
 * Given the SOS vaddr of the frame, return its index.
 * @param sos_vaddr, virtual address of the frame inside SOS address space
 * @return id that corresponds to that vaddr
 */
seL4_Word frame_table_sos_vaddr_to_index(seL4_Word sos_vaddr);

/*
 * Translate physical address to sos virtual address.
 * @param paddr, the physical address
 * @returns sos virtual address
 */
seL4_Word frame_table_paddr_to_sos_vaddr(seL4_Word paddr);

/*
 * Allocate multiple contiguous frames.
 * @param[out] vaddr, sos vaddr to access the memory
 * @param nframes, number of frames to allocate
 * @return frame id of the starting first frame on success, else -1
 */
seL4_Word multi_frame_alloc(seL4_Word *vaddr, seL4_Word nframes);

/*
 * Return the lower and upper bounds of the frame table.
 * @param[out] lower, the lowest index
 * @param[out] upper, the upper index
 * @returns 0 on success, else 1
 */
int frame_table_get_limits(seL4_Word *lower, seL4_Word *upper);

/*
 * Return whether this frame is on it's first or second chance
 * @param id, the id of the frame
 * @param[out] chance, first or second chance
 * @returns 0 on success, else 1
 */
int frame_table_get_chance(seL4_Word frame_id, enum chance_type *chance);

/*
 * Set the chance type for a frame
 * @param frame_id, id of the frame
 * @param chance, the chance of the frame
 * @returns 0 on success, else 1
 */
int frame_table_set_chance(seL4_Word frame_id, enum chance_type chance);

/*
 * Set the process page id associated with this frame
 * @param frame_id, id of the frame
 * @param pid, the process id
 * @param page_id, id of the page
 * @returns 0 on success, else 1
 */
int frame_table_set_page_id(seL4_Word frame_id, seL4_Word pid, seL4_Word page_id);

/*
 * Get the pid and page_id where this frame is mapped into 
 * @param frame_id, id of the frame
 * @param[out] pid, pid of the process
 * @param[out] page_id, id of the page
 * @returns 0 on success, else 1
 */
int frame_table_get_page_id(seL4_Word frame_id, seL4_Word *pid, seL4_Word *page_id);

#endif /* _FRAMETABLE_H_ */
