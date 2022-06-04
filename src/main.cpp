#include <windows.h>
#include <wtsapi32.h>

#include <cstdint>
#include <iostream>

#include "args.hpp"
#include "main.hpp"
#include "proc.hpp"
#include "struct.hpp"

struct args::args arguments;

int main(int argc, char * argv[])
{
	::arguments = args::handleargs(argc, argv);

	std::cout << std::showbase;
	std::cerr << std::showbase;

	std::unique_ptr<struct proc::process> proc(proc::openpid(::arguments.pid));
	if (nullptr == proc.get())
		return 1;

	struct structview::finder target(proc);
	target.iterate(::arguments.base_addr);
	target.refeachother();
	target.print();
}
