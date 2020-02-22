/*
 * MIT License
 *
 * Copyright (c) 2020 Richard Healy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mmu.h"
#include "uart.h"

#define MMU_PAGE_TABLE_LEN      512

//
//MMU Page Table.
//
typedef u64_t page_table[MMU_PAGE_TABLE_LEN];

typedef struct _level_2_table {
    page_table table;
} __attribute__((packed, aligned(4096))) level_2_table;

typedef struct _level_3_tables {
    page_table tables[RTOS_MAX_TASKS];
} __attribute__((packed, aligned(4096))) level_3_tables;


level_2_table LEVEL_2_TABLE;
level_3_tables LEVEL_3_TABLES;

//Helper function.
void mmu_print_range(u64_t beg, u64_t end, u64_t div) {
    u64_t round = div - 1;
    uart_u64hex(beg);
    uart_puts("-");
    uart_u64hex(end);
    uart_puts(" [");
    uart_u64hex((beg + round) / div);
    uart_puts("-");
    uart_u64hex((end + round) / div);
    uart_puts("]");
}


//
//mmu_enable_level_3_table_rw_blks()
// Starting with 'beg' set 'num' blocks to be R/W non-executable in the
// level 3 table for 'task'.
//
void mmu_enable_level_3_table_rw_blks(u64_t task, u64_t beg, u64_t num) {
    u64_t i;
    for(i = beg; i < (beg + num); ++i) {
        LEVEL_3_TABLES.tables[task][i] =
            //Valid bit - [0:0]
            //Set = 0b1
            (u64_t) 0x0000000000000001 +
            //Type bit - [1:1]
            //TABLE = 0b1
            (u64_t) 0x0000000000000002 +
            //Attr Index - [4:2] 
            //DRAM = 0b001 (ATTR1 in MAIR_EL1 above)
            (u64_t) 0x0000000000000004 +
            //AP - [7:6]
            //RW_EL1 = 0b00
            (u64_t) 0x0000000000000000 + 
            //SH - [9:8]
            //Inner shareable = 0b11
            (u64_t) 0x0000000000000300 +
            //AF bit - [10:10]
            //Accessible = 0b1
            (u64_t) 0x0000000000000400 +
            //LVL3_OUTPUT_ADDR_4KiB - [47:12]
            (i << 12) +
            //PXN - [53:53]
            //No execute - 0b1
            (u64_t) 0x0020000000000000;
    }
}

//
//mmu_enable_level_3_table_rox_blks()
// Starting with 'beg' set 'num' blocks to be R/O executable in the
// level 3 table for 'task'.
//
void mmu_enable_level_3_table_rox_blks(u64_t task, u64_t beg, u64_t num) {
    u64_t i;
    for(i = beg; i < (beg + num); ++i) {
        LEVEL_3_TABLES.tables[task][i] =
            //Valid bit - [0:0]
            //Set = 0b1
            (u64_t) 0x0000000000000001 +
            //Type bit - [1:1]
            //TABLE = 0b1
            (u64_t) 0x0000000000000002 +
            //Attr Index - [4:2] 
            //DRAM = 0b001 (ATTR1 in MAIR_EL1 above)
            (u64_t) 0x0000000000000004 +
            //AP - [7:6]
            //RO_EL1 = 0b10
            (u64_t) 0x0000000000000080 +
            //SH - [9:8]
            //Inner shareable = 0b11
            (u64_t) 0x0000000000000300 +
            //AF bit - [10:10]
            //Accessible = 0b1
            (u64_t) 0x0000000000000400 +
            //LVL3_OUTPUT_ADDR_4KiB - [47:12]
            (i << 12) +
            //PXN - [53:53]
            //Disable execute never - 0b0
            (u64_t) 0x0000000000000000;
    }
}

//
//mmu_enable_level_3_table()
// Enable the level 3 table for a task.
//
// task  - task.
// robeg - byte offset of beginning of R/O memory.
// rolen - length of R/O memory in bytes.
//
void mmu_enable_level_3_table(u64_t task, u64_t robeg, u64_t rolen) {
//Physical memory offset in current task.
    u64_t task_mem_offst = task * MMU_TASK_MEMORY_SZ;
//Number of read-write non-executable 4kB blocks before ro.
    u64_t rwblks1 = (robeg + 0xFFF) / 0x1000;
//Number of read only executable 4kB blocks in current task.
    u64_t roblks = (rolen + 0xFFF) / 0x1000;
//Number of read-write non-executable 4kB blocks after ro.
    u64_t rwblks2 = MMU_PAGE_TABLE_LEN - (rwblks1 + roblks);

    uart_puts("mmu::init_page_tables(): Setting up level 3 table\n");

//Set up R/W memory (if any) before R/O memory starts.
    if(rwblks1) {
        uart_puts("RW : ");
        mmu_print_range(
            task_mem_offst, 
            task_mem_offst + robeg, 
            0x1000
        );
        uart_puts("\n");
        mmu_enable_level_3_table_rw_blks (task, 0, rwblks1);
    }

//Set up R/O executable code and data segment.
    if(roblks) {
        uart_puts("ROX: ");
        mmu_print_range(task_mem_offst + robeg, 
                        task_mem_offst + robeg + rolen,
                        0x1000);
        uart_puts("\n");
        mmu_enable_level_3_table_rox_blks(task, rwblks1, roblks);
    }

//Set up R/W memory (if any) after R/O memory ends.
    if(rwblks2) {
        uart_puts("RW : ");
        mmu_print_range(
            task_mem_offst + robeg + rolen, 
            task_mem_offst + (MMU_PAGE_TABLE_LEN * 0x1000), 
            0x1000
        );
        uart_puts("\n");
        mmu_enable_level_3_table_rw_blks(task, rwblks1 + roblks, rwblks2);
    }
}

void mmu_enable_level_2_table(u64_t task, u64_t lvl3_pos) {
    u64_t i;
//Memory offset in bytes for current task memory.
    u64_t task_mem_offst  = task * MMU_TASK_MEMORY_SZ;
//Offset into the level 2 table for current task.
    u64_t lvl2_offst = task * MMU_BLOCKS_PER_TASK;

    uart_puts("mmu_enable(): Setting up level 2 table\n");

//First 2MiB blocks in task describe read-write DRAM.
    uart_puts("RW : ");
    mmu_print_range(task_mem_offst, 
                    task_mem_offst + (0x200000 * (MMU_BLOCKS_PER_TASK - 1)), 
                    0x200000);
    uart_puts("\n");

    for (i = 0; i < MMU_BLOCKS_PER_TASK; ++i) {
        //Current offset into the level two table.
        u64_t cur_lvl2_offst = lvl2_offst + i;

        if(i == lvl3_pos) {
//This 2MiB block in task is subdivided into 512 4KiB (0x1000) block
//described by the 512 entries in the level 3 table. Tell the MMU to
//jump to the level 3 table when this block of addresses is accessed.
            mmu_print_range(task_mem_offst + (0x200000 * cur_lvl2_offst), 
                            task_mem_offst + (0x200000 * (cur_lvl2_offst + 1)), 
                            0x200000);
            uart_puts("->level 3 table.\n");

            LEVEL_2_TABLE.table[cur_lvl2_offst] =
                //Valid bit - [0:0]
                //Set = 0b1
                (u64_t) 0x00000001 +
                //Type bit - [1:1]
                //Table = 0b1
                (u64_t) 0x00000002 +
                //NEXT_LVL_TABLE_ADDR_4KiB - [47:12]
                //Tables are 4kB aligned so MMU doesn't use last 12 bits. 
                (u64_t) &LEVEL_3_TABLES.tables[task];
        } else {
            LEVEL_2_TABLE.table[cur_lvl2_offst] =
                //Valid bit - [0:0]
                //Set = 0b1
                (u64_t) 0x0000000000000001 +
                //Type bit - [1:1]
                //Block = 0b0
                (u64_t) 0x0000000000000000 +
                //Attr Index - [4:2] 
                //DRAM = 0b001 (ATTR1 in MAIR_EL1 above)
                (u64_t) 0x0000000000000004 +
                //AP - [7:6]
                //RW_EL1 = 0b00
                (u64_t) 0x0000000000000000 + 
                //SH - [9:8]
                //Inner shareable = 0b11
                (u64_t) 0x0000000000000300 +
                //AF bit - [10:10]
                //Accessible = 0b1
                (u64_t) 0x0000000000000400 +
                //LVL2_OUTPUT_ADDR_4KiB - [47:21]
                (cur_lvl2_offst << 21) +
                //PXN - [53:53]
                //No execute - 0b1
                (u64_t) 0x0020000000000000;
        }
    }
}

void mmu_enable(mmu_range_lst rolst, u64_t numtasks) {
    u64_t i;

//
// MAIR_EL1
//
// Describe to the MMU the attributes the memory/devices will have when they get 
// mapped into the virtual address space. MAIR_EL1 can have up to 8 different
// attributes. We just need two - one for DRAM and one for the MMIO peripherals.
//
// A field in a page table entry stored in DRAM references one of the attributes
// set here. When the MMU reads a page table entry it uses the attribute (0..7)
// for that memory access.
//
// High 4 bits sets cache hints applying to accesses from regions of the CPU
// considered to be 'outer' relative to the memory/device being accessed.
//
// Low 4 bits sets cache hints applying to accesses from regions of the CPU
// considered to be 'inner' relative to the memory/ device being accessed.
//
    asm (
        "msr    mair_el1, %0\n"
        :: "r" (
//These tell the MMU that the address blocks with attribute 1 are mapped
//to memory like DRAM.
            //ATTR1 High - [15:12]
            // Memory_OuterWriteBack_NonTransient_ReadAlloc_WriteAlloc = 0b1111
            (u64_t) 0x000000000000F000 +
            //ATTR1 Low - [11:8]
            // InnerWriteBack_NonTransient_ReadAlloc_WriteAlloc = 0b1111
            (u64_t) 0x0000000000000F00 +

//These tell the MMU that address blocks with attribute 0 are mapped
//to devices like the MMIO peripherals.
            //ATTR0 High - [7:4]
            // Device = 0b0000
            (u64_t) 0x0000000000000000 +
            //ATTR0 Low - [4:0]
            // Device nGnRE =  0b0100
            (u64_t) 0x0000000000000004 
        ) :
    );

//Set up the page tables for each task.
    for (i = 0; i < numtasks; ++i) {
        mmu_enable_level_2_table(i, MMU_BLOCKS_PER_TASK - 1);
        mmu_enable_level_3_table(i, rolst[i][0], rolst[i][1]);
    }

//Entries describe peripheral addresses starting at MMIO_BASE and extending
//to the end of the address space covered by the LVL2_TABLE.
    uart_puts("mmu_enable(): Mapping peripherals in level 2 table\n");
    mmu_print_range(MMIO_BASE, 0x40000000, 0x200000);
    uart_puts("\n");

    for (i = MMIO_BASE / 0x200000; i < MMU_PAGE_TABLE_LEN; ++i) {
        LEVEL_2_TABLE.table[i] =
            //Valid bit - [0:0]
            //Set = 0b1
            (u64_t) 0x0000000000000001 +
            //Type bit - [1:1]
            //Block = 0b0
            (u64_t) 0x0000000000000000 +
            //Attr Index - [4:2] 
            //Device = 0b000 (ATTR0 in MAIR_EL1 above)
            (u64_t) 0x0000000000000000 +
            //AP - [7:6]
            //RW_EL1 = 0b00
            (u64_t) 0x0000000000000000 + 
            //SH - [9:8]
            //Outer shareable = 0b10
            (u64_t) 0x0000000000000200 +
            //AF bit - [10:10]
            //Accessible = 0b1
            (u64_t) 0x0000000000000400 +
            //LVL2_OUTPUT_ADDR_4KiB - [47:21]
            (i << 21) +
            //PXN - [53:53]
            //No execute - 0b1
            (u64_t) 0x0020000000000000;        
    }
//FIXME: Need to finish this.
}
