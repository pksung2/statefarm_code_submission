#include "pages.h"
#include "types.h"
#include "lib.h"

uint32_t pageDirectory[KILOBYTE] __attribute__((aligned(FOUR_KB)));
uint32_t pageTable[KILOBYTE] __attribute__((aligned(FOUR_KB)));
uint32_t pageTableTerminal[KILOBYTE] __attribute__((aligned(FOUR_KB)));

void init_pages(uint32_t * ptr);

/*  init_pd
 *  DESCRIPTION: This function initializes the Page Directory and enables paging.
 *  INPUTS: NONE
 *	OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Maps physical to virtual memory and linearly addresses kernel and
 *		video memory. 
 *
 *  CREDIT: http://www.osdever.net/tutorials/view/implementing-basic-paging
 *		http://littleosbook.github.io/#paging
 *		http://wiki.osdev.org/Setting_Up_Paging
 */
void init_pd()
{
	int i;

	// Initialize Page Table (first 4MB)
	// Input address of pageTable with present (P) flag set
	pageDirectory[0] = (unsigned int)pageTable | PRESENT_F;

	// fill table with 'NOT PRESENT' pages
	for(i = 0; i < KILOBYTE ; i ++ )
	{
		pageTable[i] = 0;
	}

	// Map video memory divided by 4096 
	pageTable[VIDEO_LOC / FOUR_KB] = PRESENT_F | PRIVILEGE_F | VIDEO_LOC;

	// Map Kernel in Virtual Memory
	pageDirectory[1] = KERNEL | SIZE_F | GLOBAL_F | PRESENT_F;

	// Make 'NOT PRESENT' pages in the rest of VM after Kernel allocation
	for(i = 2; i < KILOBYTE ; i++)
	{
		// After 8MB, all PDE's point to 4MB pages
		// Set 'SIZE' and 'NOT PRESENT' flags 
		pageDirectory[i] = SIZE_F;
	}

	// Enable paging by setting Control Registers
	init_pages((uint32_t*)&pageDirectory);
}
