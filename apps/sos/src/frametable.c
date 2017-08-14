/*
 * Frametable Implementation
 * Glenn McGuire and Cameron Lonsdale
 */

#include "frametable.h"

#include <assert.h>
#include <strings.h>

#include <cspace/cspace.h>
#include <sys/panic.h>

#include "vmem_layout.h"
#include "ut_manager/ut.h"
#include "mapping.h"

#include <stdio.h> // DEBUG
#include <sys/debug.h>
#define verbose 5

#define PAGE_SIZE (1 << seL4_PageBits)
#define PAGE_ALIGN(addr) ((addr + PAGE_SIZE-1) & ~(PAGE_SIZE-1))
#define ADDR_TO_INDEX(paddr) ((paddr - ut_base) >> seL4_PageBits)
#define INDEX_TO_ADDR(index) (ut_base + (index << seL4_PageBits))

/* filthy macro to round up the number of bits for ut_alloc, except when > 14 */
#define ROUND_ALLOC_BITS(x) ((x<=4)?4:((x<=9)?9:((x<=10)?10:((x<=12)?12:((x<=14)?14:x)))))
#define FT_PORTION 0.8
#define MAX_ALLOC_BITS 14

static inline int log_two(seL4_Word value);
static inline seL4_Word upper_power_of_two(seL4_Word v);

static seL4_Word ut_base; /* the base of the UT chunk we reference from */

/* Sizing Constraints */
static seL4_Word frame_table_max = 0;
static seL4_Word frame_table_cnt = 0;

/* The frame table is an array of frame entries */
frame_entry *frame_table = NULL;

/*
 * Initialise the frame table
 * @return 0 on success, else 1
 */
int
frame_table_init()
{
    /* Table already exists */
    if (frame_table)
        return 1;
    
    /* find the size of the UT block we have to play with */
    seL4_Word high;
    ut_find_memory(&ut_base, &high);

    dprintf(0, "****** ut memory starts at %x and ends at %x for a size of %x and %d pages\n", ut_base, high, high-ut_base, (high-ut_base)/4096);
    
    /* 
     * Set upper bound on frames that can be allocated 
     * We want to reserve an amount of UT memory for SOS.
     * If SOS can't allocate memory for TCBs, caps, etc, then it will assert failure instead of return failure.
     *
     * We set aside 80% of the memory for user application frames, and 20% of UT for SOS.
     */
    frame_table_max = ((high - ut_base) / PAGE_SIZE) * FT_PORTION;

    /* Allocate the table with enough pages */
    seL4_Word n_pages = PAGE_ALIGN(high - ut_base) / PAGE_SIZE;
    //frame_table = (frame_entry *)malloc(sizeof(frame_entry) * n_pages);
    
    seL4_Word paddr = 0; 
    seL4_Word num_bytes = sizeof(frame_entry) * n_pages;
    seL4_Word num_bytes_rounded = upper_power_of_two(num_bytes);
    seL4_Word num_bits = log_two(num_bytes_rounded);
    dprintf(0, "*** num_bytes=%d, num_bytes_rounded=%d, num_bits=%d\n", num_bytes, num_bytes_rounded, num_bits);
    assert(num_bits > MAX_ALLOC_BITS); /* frametable should always be larger than 2**14 in size */
    seL4_Word num_allocs = 2<<(num_bits-MAX_ALLOC_BITS-1);
    dprintf(0, "** need to make %d allocs of %d bits for a total of %d bytes\n", num_allocs, 14, (2<<13)*num_allocs);
    for(int i = 0; i < num_allocs; i++)
        paddr = ut_alloc(MAX_ALLOC_BITS);
    
    seL4_ARM_Page frame_cap;

    /* Retype the untyped memory to a frame object */
    if (cspace_ut_retype_addr(paddr, seL4_ARM_SmallPageObject, num_bits, cur_cspace, &frame_cap) != 0)
        dprintf(0, "** retype error :)\n");

    /* Map the page into SOS virtual address space */
    seL4_Word vaddr = PHYSICAL_VSTART + paddr;
    if (map_page(frame_cap, seL4_CapInitThreadPD, vaddr, seL4_AllRights, seL4_ARM_Default_VMAttributes) != 0)
        dprintf(0, "** map error :)\n");

    frame_table = (frame_entry *)vaddr;

    if (!frame_table)
        return 1;

    dprintf(0, "** returned :)\n");

    return 0;
}

/* 
 * Rounds a value up to the next value of two
 * @param value to round
 * @returns the provided value rounded 
 */
static inline seL4_Word upper_power_of_two(seL4_Word v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/*
 * Given a value that is a power of two, finds the set single bit
 * @param the power of two value
 * @returns the bit set
 */
static inline int log_two(seL4_Word value)
{
    seL4_Word bit_num = 0;
    while (((value & 1) == 0) && value > 1) {
        value >>= 1;
        bit_num++;
    }
    return bit_num;
}

/*
 * Reserve a physical frame
 * @param[out] virtual address of the frame
 * @returns ID of the frame. Index into the frametable
 */
seL4_Word
frame_alloc(seL4_Word *vaddr)
{
    seL4_Word paddr;
    seL4_ARM_Page frame_cap;

    /* Check if frame_table is initialised */
    if (!frame_table)
        goto frame_alloc_error;

    /* Ensure we aren't exceeding limits */
    if (frame_table_cnt >= frame_table_max)
        goto frame_alloc_error;

    /* Grab a page sized chunk of untyped memory */
    if ((paddr = ut_alloc(seL4_PageBits)) == (seL4_Word)NULL)
        goto frame_alloc_error;

    /* Retype the untyped memory to a frame object */
    if (cspace_ut_retype_addr(paddr, seL4_ARM_SmallPageObject, seL4_PageBits, cur_cspace, &frame_cap) != 0)
        goto frame_alloc_error;

    /* Map the page into SOS virtual address space */
    *vaddr = PHYSICAL_VSTART + paddr;
    if (map_page(frame_cap, seL4_CapInitThreadPD, *vaddr, seL4_AllRights, seL4_ARM_Default_VMAttributes) != 0)
        goto frame_alloc_error;

    /* Zero out the memory */
    bzero((void *)(*vaddr), PAGE_SIZE);

    /* Offset the paddr to align with our ut block */
    seL4_Word p_id = ADDR_TO_INDEX(paddr);

    /* Store the metadata in the frame table */
    frame_table[p_id].cap = frame_cap;
    frame_table_cnt++;

    return p_id;

    /* On error, set the vaddr to null and return -1 */
    frame_alloc_error:
        *vaddr = (seL4_Word)NULL;
        return -1;
}

/*
 * Free an allocated frame
 * @param frame_id of the frame to be freed.
 */
void
frame_free(seL4_Word frame_id)
{
    /* Check if frame_table is initialised */
    if (!frame_table)
        return;

    /* Get the cap from the frame_table */
    seL4_CPtr frame_cap = frame_table[frame_id].cap;

    /* Ensure this is a frame we can have allocated */
    if (!frame_cap)
        return;
    
    /* Unmap the page */
    seL4_ARM_Page_Unmap(frame_cap);
    frame_table_cnt--;

    /* Delete the capability and release the memory back to UT */
    cspace_delete_cap(cur_cspace, frame_cap);
    ut_free(INDEX_TO_ADDR(frame_id), seL4_PageBits);
}
