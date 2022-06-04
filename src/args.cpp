#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "args.hpp"

enum class NEXT_ARG {
	NOTIN,
	PID,
	BASE,
	OFFSET
};

static void removetodo(std::vector<enum NEXT_ARG> & todo, enum NEXT_ARG rm)
{
	std::vector<enum NEXT_ARG>::iterator it = std::find(todo.begin(), todo.end(), rm);
	if (it != todo.end())
		todo.erase(it);
}

struct args::args args::handleargs(int argc, char * argv[])
{
	struct args::args arguments { 0, 0 };
	std::vector<enum NEXT_ARG> todo = { NEXT_ARG::PID, NEXT_ARG::BASE, NEXT_ARG::OFFSET };
	enum NEXT_ARG mode = NEXT_ARG::NOTIN;

	for (unsigned i = 1; i < argc; i++) {
		std::string arg(argv[i]);

		if (NEXT_ARG::NOTIN != mode) {
			std::istringstream in(arg);
			if (NEXT_ARG::PID == mode) {
				in >> arguments.pid;
				if (in.fail()) {
					std::cerr << "Invalid pid: " << arg << std::endl;
				} else {
					removetodo(todo, NEXT_ARG::PID);
				}
			} else if (NEXT_ARG::BASE == mode) {
				in >> std::hex >> arguments.base_addr;
				if (in.fail()) {
					std::cerr << "Invalid base addr: " << arg << std::endl;
				} else {
					removetodo(todo, NEXT_ARG::BASE);
				}
			} else if (NEXT_ARG::OFFSET == mode) {
				in >> std::hex >> arguments.offset;
				if (in.fail()) {
					std::cerr << "Invalid offset addr: " << arg << std::endl;
				} else {
					removetodo(todo, NEXT_ARG::OFFSET);
				}
			}
			mode = NEXT_ARG::NOTIN;
		} else if (arg == "--pid" || arg == "-p") {
			mode = NEXT_ARG::PID;
		} else if (arg == "--base" || arg == "-b") {
			mode = NEXT_ARG::BASE;
		} else if (arg == "--offset" || arg == "-o") {
			mode = NEXT_ARG::OFFSET;
		} else if (arg == "--color" || arg == "-c") {
			arguments.color = true;
		} else {
			std::cerr << "Invalid argument: " << arg << std::endl;
		}
	}

	for (enum NEXT_ARG i : todo) {
		switch (i) {
		case NEXT_ARG::PID:
			arguments.pid = getpid();
			break;
		case NEXT_ARG::BASE:
			arguments.base_addr = getbaseaddr();
			break;
		case NEXT_ARG::OFFSET:
			arguments.offset = 0x4000;
			break;
		}
	}

	return arguments;
}

unsigned long args::getpid(void)
{
	unsigned long pid;
	while (!std::cin.eof() && !std::cin.bad()) {
		std::cout << "Enter pid: ";
		std::cin >> pid;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}
		break;
	}
	return pid;
}

uintptr_t args::getbaseaddr(void)
{
	uintptr_t addr;
	while (!std::cin.eof() && !std::cin.bad()) {
		std::cout << "Enter base address of first structure: ";
		std::cin >> std::hex >> addr >> std::dec;

		if (std::cin.fail()) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}
		break;
	}
	return addr;
}
