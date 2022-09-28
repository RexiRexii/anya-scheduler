#pragma once
#include <Windows.h>
#include "../utilities/utilities.hpp"

class anya_scheduler
{
	std::uintptr_t scheduler_main;
	std::uintptr_t scheduler_iter_start;
	std::uintptr_t scheduler_iter_end;
	std::uintptr_t scheduler_script_context;
	std::uintptr_t scheduler_frame_delay;

public:
	section_t text;

public:
	explicit anya_scheduler();

	std::uintptr_t get_job_by_name(std::string_view to_get) const;
	std::vector<job_t*> get_all_jobs() const;
	std::uintptr_t get_script_context() const;

	std::uintptr_t hook_job(std::uintptr_t job, void* to_hook) const;
	void hook_scripts_job() const;
};

extern anya_scheduler meme;