#include "task_scheduler.hpp"

std::uintptr_t job_cache;
std::uintptr_t new_vftable[6];

// this is the constructor to the task scheduler
// we set up offsets and such here
// pretty useful yes this is why I really like using classes
anya_scheduler::anya_scheduler()
{
	this->text = mem_scanner::get_section(".text", false);

	const auto task_scheduler_pattern = mem_scanner::scan_pattern("\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x08\xE8\x00\x00\x00\x00\x8D\x0C\x24", "xxxxxxxxxx????xxx", {this->text.start, this->text.start + text.size})[0];
	const auto task_scheduler = (task_scheduler_pattern + 14) + *reinterpret_cast<std::uintptr_t*>(task_scheduler_pattern + 10);

	this->scheduler_main = reinterpret_cast<std::uintptr_t(*)()>(task_scheduler)();

	this->scheduler_iter_start = 308;
	this->scheduler_iter_end = 312;

	this->scheduler_script_context = 304;
	this->scheduler_frame_delay = 280;
}

// grab whichever job you inputted in the first arg
// make sure to input the name correctly (case sensitive) or you might get fucked
std::uintptr_t anya_scheduler::get_job_by_name(std::string_view to_get) const
{
	auto ret = 0;

	auto iterator = *reinterpret_cast<const std::uintptr_t*>(this->scheduler_main + this->scheduler_iter_start);
	const auto job_end = *reinterpret_cast<std::uintptr_t*>(this->scheduler_main + this->scheduler_iter_end);

	while (iterator != job_end)
	{
		const auto inst = *reinterpret_cast<job_t**>(iterator);

		if (inst->name == to_get.data())
			ret = reinterpret_cast<std::uintptr_t>(inst);

		iterator += 8;
	}

	return ret;
}

// for debugging / research purposes, use it if you want
// this will grab ALL the jobs and list them on the console if you want to use it
// make sure debug option is turned on in utilities.hpp
std::vector<job_t*> anya_scheduler::get_all_jobs() const
{
	std::vector<job_t*> jobs{};

	auto iterator = *reinterpret_cast<const std::uintptr_t*>(this->scheduler_main + this->scheduler_iter_start);
	const auto job_end = *reinterpret_cast<std::uintptr_t*>(this->scheduler_main + this->scheduler_iter_end);

	while (iterator != job_end)
	{
		const auto inst = *reinterpret_cast<job_t**>(iterator);
		jobs.emplace_back(inst);

		iterator += 8;
	}

	for (auto all_jobs : jobs)
		printf("Found job: %s\n", all_jobs->name.data());

	return jobs;
}

// you can use the task scheduler to grab the script context for your lua state
// pretty good method TBH
// I use this even in Misako
std::uintptr_t anya_scheduler::get_script_context() const
{
	const auto waiting_scripts = this->get_job_by_name("WaitingHybridScriptsJob");
	const auto script_context = *reinterpret_cast<const std::uintptr_t*>(waiting_scripts + this->scheduler_script_context);

	return script_context;
}

// do your task scheduler execution shit here
// this is pretty useful, and is required for those that are making an exploit
// you need your execution to be on sync with Roblox's, you NEED to do this
std::int32_t __fastcall scripts_job_hook(std::uintptr_t job, std::uintptr_t, std::uintptr_t to_call)
{
	// std::queue stuff here

	const auto func_t = reinterpret_cast<std::int32_t(__thiscall*)(std::uintptr_t, std::uintptr_t)>
		(job_cache)(job, to_call);

	return func_t;
}

// this is our job hook function
// we use this to hook functions like the one above (scripts_job_hook)
// and below it hooks the vftable yes
std::uintptr_t anya_scheduler::hook_job(std::uintptr_t job, void* to_hook) const
{
	std::memcpy(new_vftable, *reinterpret_cast<void**>(job), 4u * 6u);

	const auto ts_old_vftable = *reinterpret_cast<std::uintptr_t**>(job);
	new_vftable[2] = reinterpret_cast<std::uintptr_t>(to_hook);

	*reinterpret_cast<std::uintptr_t**>(job) = new_vftable;
	return ts_old_vftable[2];
}

// we hook the "WaitingHybridScriptsJob" for our execution to be on sync with Roblox
// this will make everything much stabler, including your execution
// without this scripts like "for i = 1, 10 do print(i) end" will return false output
void anya_scheduler::hook_scripts_job() const
{
	const auto waiting_scripts_job = this->get_job_by_name("WaitingHybridScriptsJob");
	job_cache = this->hook_job(waiting_scripts_job, scripts_job_hook);
}