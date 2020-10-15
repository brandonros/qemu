/*
 * TriCore Baseboard System emulation.
 *
 * Copyright (c) 2013-2014 Bastian Koppelmann C-Lab/University Paderborn
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
./configure --target-list=tricore-softmmu
make -j 4
./tricore-softmmu/qemu-system-tricore --machine tricore_testboard -nographic -D /tmp/qemu.log -d in_asm,out_asm,op,cpu,exec,mmu -singlestep
*/

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "cpu.h"
#include "net/net.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "exec/address-spaces.h"
#include "elf.h"
#include "hw/tricore/tricore.h"
#include "qemu/error-report.h"

/* Board init.  */

static struct tricore_boot_info tricoretb_binfo;
static struct memory_region memory_regions[] = {
    {.start = 0x50000000, .end = 0x5001DFFF, .size = 0x1E000, .name: "CPU2_DSPR"},
    {.start = 0x5001E000, .end = 0x5001FFFF, .size = 0x2000, .name: "CPU2_DCACHE"},
    {.start = 0x50100000, .end = 0x50107FFF, .size = 0x8000, .name: "CPU2_PSPR"},
    {.start = 0x50108000, .end = 0x5010BFFF, .size = 0x4000, .name: "CPU2_PCACHE"},

    {.start = 0x60000000, .end = 0x6001DFFF, .size = 0x1E000, .name: "CPU1_DSPR"},
    {.start = 0x6001E000, .end = 0x6001FFFF, .size = 0x2000, .name: "CPU1_DCACHE"},
    {.start = 0x60100000, .end = 0x60107FFF, .size = 0x8000, .name: "CPU1_PSPR"},
    {.start = 0x60108000, .end = 0x6010BFFF, .size = 0x4000, .name: "CPU1_PCACHE"},

    {.start = 0x70000000, .end = 0x7001DFFF, .size = 0x1E000, .name: "CPU0_DSPR"},
    {.start = 0x70100000, .end = 0x70105FFF, .size = 0x6000, .name: "CPU0_PSPR"},
    {.start = 0x70106000, .end = 0x70107FFF, .size = 0x2000, .name: "CPU0_PCACHE"},

    {.start = 0x80000000, .end = 0x801FFFFF, .size = 0x200000, .name: "PMU0"},
    {.start = 0x80200000, .end = 0x803FFFFF, .size = 0x200000, .name: "PMU1"},

    {.start = 0xAF000000, .end = 0xAF0FFFFF, .size = 0x100000, .name: "DF0"},

    {.start = 0xf0000000, .end = 0xffffffff, .size = 0x10000000, .name: "PERIPHERAL"},
};
static int num_memory_regions = sizeof(memory_regions) / sizeof(struct memory_region);

#define SBOOT_SIZE 0x8000
#define SBOOT_START 0x80000000
#define ENTRY_POINT 0x80000020

static void tricoreboard_init(MachineState *machine)
{
    TriCoreCPU *cpu;
    CPUTriCoreState *env;
    cpu = TRICORE_CPU(cpu_create(machine->cpu_type));
    env = &cpu->env;
    MemoryRegion *sysmem = get_system_memory();
    // set RAM size
    tricoretb_binfo.ram_size = machine->ram_size;
    // init memory regions
    for (int i = 0; i < num_memory_regions; ++i) {
      struct memory_region memory_region = memory_regions[i];
      MemoryRegion *ptr = g_new(MemoryRegion, 1);
      memory_region_init_ram(ptr, NULL, memory_region.name, memory_region.size, &error_fatal);
      memory_region_add_subregion(sysmem, memory_region.start, ptr);
    }
    // load into memory
    FILE *fp = fopen("sboot.bin", "rb");
    int fd = fileno(fp);
    uint8_t *buf = malloc(SBOOT_SIZE);
    fread(buf, SBOOT_SIZE, 1, fp);
    int prot = 0;
    int flags = 0;
    int offset = 0;
    target_mmap(SBOOT_START, SBOOT_SIZE, prot, flags, fd, offset);
    free(buf);
    fclose(fp);
    // set entry point
    env->PC = ENTRY_POINT;
}

static void ttb_machine_init(MachineClass *mc)
{
    mc->desc = "a minimal TriCore board";
    mc->init = tricoreboard_init;
    mc->default_cpu_type = TRICORE_CPU_TYPE_NAME("tc27x");
}

DEFINE_MACHINE("tricore_testboard", ttb_machine_init)
