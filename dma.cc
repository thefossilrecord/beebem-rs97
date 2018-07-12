
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "dma.h"

int map_handle = -1;
volatile uint16_t *dma_ptr = NULL;

#define DMA_SIZE ((320 * 480) * 2)

/*
int map_memory(unsigned long addr, unsigned long size)
	{
	map_handle = open("/dev/mem", O_RDWR | O_SYNC);
	if(map_handle < 0)
		{
		printf("failed to open /dev/mem\n");
		return -1;
		}

	dma_pointer = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, map_handle, addr);
	return 0;
	}

void unmap_memory(unsigned long size)
	{
	if(dma_pointer)
		{
		munmap((void*)dma_pointer, size);
		}
	if(map_handle!=-1)
		{
		close(map_handle);
		map_handle = -1;
		}
	}
*/

int init_dma()
	{
	uint32_t dma_addr = 0;

	map_handle = open("/dev/mem", O_RDWR | O_SYNC);
	if(map_handle < 0)
		{
		printf("failed to open /dev/mem\n");
		return 0;
		}

	volatile unsigned long *dma_pointer = (volatile unsigned long *)mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, map_handle, LCD_BASE);

	if(dma_pointer)
		{
		dma_addr = dma_pointer[LCDSA0];
		munmap((void*)dma_pointer, PAGE_SIZE);
		}

	if(dma_addr)
		{
		dma_ptr = (volatile uint16_t *)mmap(0, DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, map_handle, dma_addr);
		if(dma_ptr)
			memset((void *)dma_ptr, 0, sizeof(uint16_t) * (320 * 480));
		}
		
	return 1;
	}

void close_dma()
	{
	if(dma_ptr)
		{
		munmap((void *)dma_ptr, DMA_SIZE);
		dma_ptr = NULL;
		close(map_handle);
		map_handle = 0;
		}
	}

