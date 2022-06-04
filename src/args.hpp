#ifndef ARGS_HPP
#define ARGS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace args {
	struct args {
		unsigned long pid;
		uintptr_t base_addr;
		bool color;
		uintptr_t offset;
	};

	struct args::args handleargs(int argc, char * argv[]);
	unsigned long getpid(void);
	uintptr_t getbaseaddr(void);
}

#endif // ARGS_HPP
