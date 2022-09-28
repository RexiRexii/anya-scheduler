// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <thread>

#include "task_scheduler/task_scheduler.hpp"

anya_scheduler task_scheduler;

void main()
{
    mem_utils::console();

    task_scheduler.hook_scripts_job();
    mem_utils::dbgprintf("Hooked scripts job\n--------\n");

    task_scheduler.get_all_jobs();
    mem_utils::dbgprintf("--------\nGotten all the jobs\n");

    const auto script_context = task_scheduler.get_script_context();
    mem_utils::dbgprintf("Script Context: 0x%X\n", script_context);
}

bool __stdcall DllMain( void*, std::int32_t call_reason, void* )
{
    if (call_reason == 1)
        std::thread{ main }.detach();

    return true;
}
