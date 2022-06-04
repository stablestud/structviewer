#include <windows.h>

#include <memoryapi.h>
#include <winbase.h>
#include <wtsapi32.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "proc.hpp"

static bool findproc(const unsigned long pid, std::unique_ptr<struct proc::process> & procinfo);
static bool pidexists(const unsigned long pid, const WTS_PROCESS_INFO procs[], const unsigned long pcount, unsigned long & pos);
static std::pair<std::string, std::string> getuname(const PSID & usersid);

proc::process::process(const unsigned long pid, const std::string procname, const std::string username, const std::string domain)
{
	this->pid = pid;
	this->procname = procname;
	this->username = username;
	this->domain = domain;
	this->handle = nullptr;
}

proc::process::~process(void)
{
	if (this->handle != nullptr)
		CloseHandle(this->handle);
}

void proc::process::printpid(void)
{
	std::cout << this->pid;
	if (!this->procname.empty())
		std::cout << " <" << this->procname << ">";
	if (!this->username.empty())
		std::cout << " " << this->username;
	if (!this->domain.empty())
		std::cout << "@" << this->domain;
}

std::unique_ptr<struct proc::process> proc::openpid(const unsigned long pid)
{
	std::unique_ptr<struct proc::process> proc;
	if (!findproc(pid, proc))
		return nullptr;

	proc->handle = OpenProcess(PROCESS_VM_READ, false, proc->pid);
	if (proc->handle == NULL) {
		std::cerr << "Cannot open process ";
		proc->printpid();
		std::cout << std::endl;
		return nullptr;
	}

	std::cout << "Opened ";
	proc->printpid();
	std::cout << std::endl;

	return proc;
}

static bool findproc(const unsigned long pid, std::unique_ptr<struct proc::process> & procinfo)
{
	unsigned long procn;
	WTS_PROCESS_INFO * allprocs;

	if (!WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0x0, 0x1, &allprocs, &procn)) {
		std::cerr << "Cannot retrieve systems process list." << std::endl;
		return false;
	}

	unsigned long arrpos;
	if (!pidexists(pid, allprocs, procn, arrpos)) {
		std::cerr << "Cannot find pid '" << pid << "'" << std::endl;
		return false;
	}

	std::pair<std::string, std::string> sidname = getuname(allprocs[arrpos].pUserSid);
	procinfo.reset(new proc::process(allprocs[arrpos].ProcessId, allprocs[arrpos].pProcessName, sidname.first, sidname.second));

	if (0 == procinfo->pid)
		procinfo->procname = "System";

	WTSFreeMemory(allprocs);

	return true;
}

static bool pidexists(const unsigned long pid, const WTS_PROCESS_INFO procs[], const unsigned long pcount, unsigned long & pos)
{
	bool found = false;

	for (unsigned long i = 0; i < pcount; i++) {
		if (procs[i].ProcessId == pid) {
			found = true;
			pos = i;
			break;
		}
	}

	return found;
}

static std::pair<std::string, std::string> getuname(const PSID & usersid)
{
	std::string uname;
	std::string dname;
	TCHAR cuname[256];
	TCHAR cdname[256];
	unsigned long lcname = sizeof(cuname) / sizeof(TCHAR);
	unsigned long ldname = sizeof(cdname) / sizeof(TCHAR);
	SID_NAME_USE atype;

	if (!LookupAccountSid(nullptr, usersid, cuname, &lcname, cdname, &ldname, &atype)) {
		return std::pair<std::string, std::string>("", "");
	}

	uname = std::string(cuname);
	dname = std::string(cdname);

	if (uname.empty()) 
		uname = "Unknown"; 

	return std::make_pair(uname, dname);
}

std::unique_ptr<const int32_t[]> proc::getmem(const struct proc::process & proc, const uintptr_t addr, const uint32_t size)
{
	std::unique_ptr<int32_t[]> buffer(new int32_t[size]);
	if (!ReadProcessMemory(proc.handle, reinterpret_cast<void *>(addr), buffer.get(), size, nullptr))
		return nullptr;
	return buffer;
}
