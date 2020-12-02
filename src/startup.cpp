//
//  startup.cpp
//  kernel-codebase
//
//  Created by Nikita Ivanov on 01.12.2020.
//  Copyright © 2020 PlakOS. All rights reserved.
//

#include <startup.hpp>
#include <console.hpp>
#include <vga_console.hpp>
#include <printf.h>
#include <min_list.hpp>
#include <memory.hpp>
#include <ports.hpp>

extern char __cx_kernel_start_marker, __cx_kernel_end_marker;
 
namespace cx::os::kernel::detail
{
    vga::VgaConsole gVgaConsole;
    
    const auto gKernelStart = &__cx_kernel_start_marker;
    const auto gKernelEnd = &__cx_kernel_end_marker;
}

void cx::os::kernel::BeginKernelStartup(const multiboot_info_t& boot_info)
{
    using namespace detail; 
    gVgaConsole.UpdateCursor(0, 0);
    gVgaConsole.ClearScreen();
    
    console::SetupAnsiConsole(&gVgaConsole);
    gVgaConsole.ToggleVgaCursor(true, 0, 15);
    
    printf("*** LOAD: Loaded CXOS kernel @ 0x%08X..0x%08X\n", gKernelStart, gKernelEnd);
    
    auto entry = (mmap_entry_t*) boot_info.mmap_addr;
    
    while((unsigned int) entry < boot_info.mmap_addr + boot_info.mmap_length) {
        // We ignore the 0 entry since that causes issues
        if(entry->type == 1)
        {
            auto addr = (char*) entry->base_addr_low;
            auto length = entry->length_low;
            
            if(addr >= gKernelStart && addr < gKernelEnd)
            {
                printf("*** LOAD: Found in-kernel MM bit - adjusting\n");
                
                addr = gKernelEnd;
                length -= (addr - gKernelStart);
            }
            
            printf("*** LOAD: Adding memory region type=%d 0x%08X..0x%08X\n", entry->type, addr, (uint32_t) addr + length);
            memory::AddMemoryRegion(addr, length);
        }
        entry = (mmap_entry_t*) ((unsigned int) entry + entry->size + sizeof(entry->size));
    }
    
    memory::DumpMemoryRegions();
    
    auto ptr = malloc(0x1400);
    printf("Allocated 0x1400 @ 0x%08X\n", ptr);
    
    free(ptr);
    
    malloc(0x1400);
    malloc(0x140);
    malloc(0x1500);
    
    memory::DumpMemoryRegions();
    
    struct Foo : MinListNode<Foo>
    {
        int x;
    };
    
    Foo a, b, c, d;
    a.x = 5;
    b.x = 6;
    c.x = 10;
    d.x = 30;
     
    MinList<Foo> list;
    list.InsertNode(&a);
    
    for(auto& value : list)
        printf("value=%d\n", value.x);
    
    printf("\nish1.0 # ");
}
