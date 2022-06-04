#ifndef PROC_H
#define PROC_H

#include <cstdint>
#include <memory>
#include <string>

namespace proc {
	struct process {
		process(void) {};
		process(unsigned long, std::string, std::string, std::string);
		~process(void);

		void printpid(void);

		unsigned long pid;
		std::string procname;
		std::string username;
		std::string domain;
		void * handle;
	};

	std::unique_ptr<const int32_t[]> getmem(const struct proc::process & proc, const uintptr_t addr, const uint32_t size);
	std::unique_ptr<struct proc::process> openpid(const unsigned long pid);
}

#endif // PROC_H
