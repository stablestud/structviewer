#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "main.hpp"
#include "proc.hpp"
#include "struct.hpp"

unsigned long structview::base_struct::size = 32;

static bool iscloseenough(const uintptr_t base, const uintptr_t addr);

static std::string retcharcolor(const char hex)
{
	constexpr char normal[] = { 0x1b, '[', '0', 'm', 0x0 };
	constexpr char bold[] = { 0x1b, '[', '1', 'm', 0x0 };

	constexpr char bblack[] = { 0x1b, '[', '4', '0', 'm', 0x0 };
	constexpr char bwhite[] = { 0x1b, '[', '1', '0', '7', 'm', 0x0 };
	constexpr char bred[] = { 0x1b, '[', '4', '1', 'm', 0x0 };
	constexpr char blred[] = { 0x1b, '[', '1', '0', '1', 'm', 0x0 };
	constexpr char bgreen[] = { 0x1b, '[', '4', '2', 'm', 0x0 };
	constexpr char blgreen[] = { 0x1b, '[', '1', '0', '2', 'm', 0x0 };
	constexpr char byellow[] = { 0x1b, '[', '4', '3', 'm', 0x0 };
	constexpr char blyellow[] = { 0x1b, '[', '1', '0', '3', 'm', 0x0 };
	constexpr char bblue[] = { 0x1b, '[', '4', '4', 'm', 0x0 };
	constexpr char blblue[] = { 0x1b, '[', '1', '0', '4', 'm', 0x0 };
	constexpr char bmagenta[] = { 0x1b, '[', '4', '5', 'm', 0x0 };
	constexpr char blmagenta[] = { 0x1b, '[', '1', '0', '5', 'm', 0x0 };
	constexpr char bcyan[] = { 0x1b, '[', '4', '6', 'm', 0x0 };
	constexpr char blcyan[] = { 0x1b, '[', '1', '0', '6', 'm', 0x0 };
	constexpr char blgray[] = { 0x1b, '[', '4', '7', 'm', 0x0 };
	constexpr char bdgray[] = { 0x1b, '[', '1', '0', '0', 'm', 0x0 };

	constexpr char fblack[] = { 0x1b, '[', '3', '0', 'm', 0x0 };
	constexpr char fwhite[] = { 0x1b, '[', '9', '7', 'm', 0x0 };

	std::string code;

	switch (hex) {
	case '0':
		code = bblack;
		break;
	case '1':
		code = bdgray;
		break;
	case '2':
		code = blgray;
		code += fblack;
		break;
	case '3':
		code = bblue;
		break;
	case '4':
		code = blblue;
		break;
	case '5':
		code = bcyan;
		break;
	case '6':
		code = blcyan;
		code += fblack;
		break;
	case '7':
		code = bgreen;
		break;
	case '8':
		code = blgreen;
		code += fblack;
		break;
	case '9':
		code = bred;
		break;
	case 'A':
	case 'a':
		code = blred;
		break;
	case 'B':
	case 'b':
		code = byellow;
		code += fblack;
		break;
	case 'C':
	case 'c':
		code = blyellow;
		code += fblack;
		break;
	case 'D':
	case 'd':
		code = bmagenta;
		break;
	case 'E':
	case 'e':
		code = blmagenta;
		break;
	case 'F':
	case 'f':
		code = bwhite;
		code += fblack;
		break;
	}
	return code + bold + hex + normal;
}

static std::string colorhex(const uintptr_t value)
{
	std::ostringstream out("");
	out << std::hex << std::showbase << value;
	std::string hex(out.str());

	if (::arguments.color) {
		out.str("");

		for (const char i : hex) {
			out << retcharcolor(i);
		}
	}
	return out.str();
}

structview::structure::structure(const uintptr_t addr, struct proc::process & proc) : baseaddr(addr), data(new struct structview::base_struct), iscopy(false), points_to(), pointed_from()
{
	std::unique_ptr<const int32_t[]> buffer = proc::getmem(proc, addr, structview::base_struct::size);
	
	if (buffer.get() == nullptr)
		return;

	data->ptr1 = buffer[0];
	data->ptr2 = buffer[1];
	data->ptr3 = buffer[2];
	data->unkwn1 = buffer[3];
	data->value = buffer[4];
	data->flags.f1 = reinterpret_cast<const int8_t *>(&(buffer[5]))[0x0];
	data->flags.f2 = reinterpret_cast<const int8_t *>(&(buffer[5]))[0x1];
	data->unkwn2 = buffer[6];
	data->unkwn3 = buffer[7];
}

structview::structure::structure(void) : baseaddr(0), data(nullptr), iscopy(true) {}
structview::structure::structure(structview::structure && move) : baseaddr(move.baseaddr), data(std::move(move.data)), iscopy(move.iscopy) {}
structview::structure::structure(const structview::structure & copy) : baseaddr(copy.baseaddr), data(copy.data), iscopy(true) {}

bool structview::structure::isptrto(const struct structview::structure & target) const
{
	for (const uintptr_t i : this->getptrs()) {
		if (this->ptrat(i, target)) {
			return true;
		}
	}

	return false;
}

bool structview::structure::ptrat(const uintptr_t ptr, const struct structview::structure & target) const
{
	if ((ptr >= target.baseaddr) && (ptr < (target.baseaddr + structview::base_struct::size)))
		return true;
	else
		return false;
}

std::string structview::stringptr(const uintptr_t ptr, const std::string & name, const uintptr_t base)
{
	std::ostringstream out("");
	out << std::setprecision(1) << std::hex << std::showbase << std::fixed;
	out << name << ": " << colorhex(ptr) << " @ " << std::showpos << std::dec << structview::baseoffset(base, ptr) << " (" << std::showpoint << static_cast<double>(structview::baseoffset(base, ptr)) / structview::base_struct::size << " struct)";
	return out.str();
}

std::string structview::stringunkwn(const uintptr_t value, const std::string & name, const uintptr_t base)
{
	std::ostringstream out("");
	out << std::hex << std::showbase << std::fixed;
	out << name << ":\n";
	out << structview::stringvalue(value);
	out << "\n\t[offset @ " << std::dec << std::showpos << structview::baseoffset(base, value) << std::noshowpos << " Bytes]";
	return out.str();
}

std::string structview::stringvalue(const uintptr_t value)
{
	std::ostringstream out("");
	out << std::showbase << std::fixed;
	out << "\t[unsigned: " << std::dec << value << '/' << std::hex << value << "]\n";
	out << "\t[signed: " << std::dec << static_cast<intptr_t>(value) << '/' << std::hex << static_cast<intptr_t>(value) << "]\n";
	out << "\t[float: " << std::dec << *reinterpret_cast<const float *const>(&value) << "f/" << *reinterpret_cast<const double *const>(&value) << "d]";
	return out.str();
}

std::string structview::structure::stringstruct(void) const
{
	char normal[]={ 0x1b, '[', '0', 'm', 0x0 };
	char underline[]={ 0x1b, '[', '4', 'm', 0x0 };
	std::ostringstream out(""), ptrto(""), ptrfrom("");
	out << underline << "Structure @ " << std::hex << std::showbase << this->baseaddr << " (+" << structview::base_struct::size << '=' << (this->baseaddr + this->data->size) << ')' << normal;

	ptrto << "Structures pointed from this: " << std::dec << this->points_to.size();
	unsigned ptrton = 0;
	for (const struct structview::structure & i : this->points_to) {
		std::string name("struct");
		name += std::to_string(ptrton++);
		ptrto << std::endl << '\t' << structview::stringptr(i.baseaddr, name, this->baseaddr);
	}

	ptrfrom << "Structures pointing to this: " << std::dec << this->pointed_from.size();
	unsigned ptrfromn = 0;
	for (const struct structview::structure & i : this->pointed_from) {
		std::string name("struct");
		name += std::to_string(ptrfromn++);
		ptrfrom << std::endl << '\t' << structview::stringptr(i.baseaddr, name, this->baseaddr);
	}

	return out.str() + '\n' + ptrto.str() + '\n' + ptrfrom.str();
}

void structview::structprint(const structview::finder & f, const structview::structure & st) 
{
	std::cout << std::endl << st.stringstruct() << std::endl;
	std::cout << "Data values:" << std::endl;
	std::cout << structview::stringptr(st.data->ptr1, "next", st.baseaddr) << std::endl
		<< structview::stringptr(st.data->ptr2, "prev", st.baseaddr) << std::endl
		<< structview::stringptr(st.data->ptr3, "ptr3", st.baseaddr) << std::endl;
	std::cout << structview::stringunkwn(st.data->unkwn1, "unkwn1", st.baseaddr) << std::endl;
	std::cout << "value:\n" << structview::stringvalue(st.data->value) << std::endl;
	std::cout << "flag1: " << static_cast<bool>(st.data->flags.f1) << '\n';
	std::cout << "flag2: " << static_cast<bool>(st.data->flags.f2) << '\n';
	std::cout << structview::stringunkwn(st.data->unkwn2, "unkwn2", st.baseaddr) << std::endl
		<< structview::stringunkwn(st.data->unkwn3, "unkwn3", st.baseaddr) << std::endl;
}

intptr_t structview::baseoffset(const uintptr_t base, const uintptr_t addr)
{
	return base - addr;
}

bool structview::structure::operator==(const struct structview::structure & t) const
{
	return this->baseaddr == t.baseaddr;
}

bool structview::structure::operator==(const uintptr_t t) const
{
	return this->baseaddr == t;
}

static bool iscloseenough(const uintptr_t base, const uintptr_t addr)
{
	uintptr_t maxoffset = ::arguments.offset; // 16 KiB
	intptr_t offset = addr - base;

	if (offset < static_cast<intptr_t>(maxoffset) && offset > -static_cast<intptr_t>(maxoffset))
		return true;
	else
		return false;
}

std::vector<uintptr_t> structview::structure::getptrs(void) const
{
	return std::vector<uintptr_t>() = { data->ptr1, data->ptr2, data->ptr3 };
}

structview::finder::finder(std::unique_ptr<struct proc::process> & proc) : links(), proc(proc) {}

void structview::finder::refeachother(void)
{
	for (struct structview::structure & i : this->links)
		this->addcrossref(i);
}

void structview::finder::addcrossref(struct structview::structure & target)
{
	for (struct structview::structure & i : this->links) {
		if (target.isptrto(i)) {
			target.points_to.push_back(i);
			i.pointed_from.push_back(target);
		}
	}
}

bool structview::finder::add(const uintptr_t addr)
{
	struct structview::structure target(addr, *(this->proc));
	if (nullptr == target.data.get()) {
		std::cerr << "Cannot read memory @ " << std::hex << addr << std::dec << std::endl;
		return false;
	}

	this->links.push_back(std::move(target));

	return true;
}

void structview::finder::print(void) const
{
	std::cout << "Total structs: " << this->links.size() << std::endl;
	std::cout << "Lowest struct: " << std::hex << colorhex(this->loweststruct()) << std::endl;
	std::cout << "Highest struct: " << colorhex(this->higheststruct()) << std::dec << std::endl;
	for (const struct structview::structure & i : this->links)
		structview::structprint(*this, i);
}

bool structview::finder::exist(const struct structview::structure & src) const
{
	for (const struct structview::structure & i : links)
		if (src == i)
			return true;

	return false;
}

bool structview::finder::exist(const uintptr_t addr) const
{
	for (const struct structview::structure & i : links)
		if (i == addr)
			return true;

	return false;
}

static std::string addprefix(const char c, unsigned width)
{
	return std::string(width * 2, c);
}

bool structview::finder::iterate(const uintptr_t addr)
{
	static unsigned d = 0;

	if (this->exist(addr)) {
		return false;
	}

	if (!this->add(addr)) {
		return false;
	}

	std::cout << addprefix('-', d) << std::hex << colorhex(addr) << std::dec << ": created struct\n";
	const std::vector<uintptr_t> ptrs(links.back().getptrs());

	std::vector<uintptr_t> added(0);

	for (const uintptr_t & i : ptrs) {
		if (!this->exist(i))
			added.push_back(i);
	}

	std::cout << addprefix(' ', d) << std::hex << colorhex(addr) << std::dec << ": points to " << added.size() << " new struct(s)";

	if (added.size()) {
		std::cout << ':';
		for (const uintptr_t & i : added) {
			std::cout << std::hex << ' ' << colorhex(i) << std::dec;
		}
	}
	std::cout << std::endl;

	std::cout << addprefix(' ', d) << std::hex << colorhex(addr) << ": points to ";
	for (const uintptr_t & i : ptrs) {
		std::cout << ' ' << colorhex(i);
	}
	std::cout << std::endl << std::dec;

	for (const uintptr_t & i : ptrs) {
		d++;
		if (iscloseenough(addr, i)) {
			this->iterate(i);
		} else {
			std::cout << addprefix(' ', d) << std::hex << i << ": cannot add struct, offset too high: " << std::dec << std::showpos << static_cast<intptr_t>(i - addr) << std::noshowpos << " Bytes" << std::endl;
		}
		d--;
	}

	return true;
}

uintptr_t structview::finder::loweststruct(void) const
{
	uintptr_t lowest = this->links.front().baseaddr;
	for (const struct structview::structure & i : this->links)
		if (i.baseaddr < lowest)
			lowest = i.baseaddr;

	return lowest;
}

uintptr_t structview::finder::higheststruct(void) const
{
	uintptr_t highest = this->links.front().baseaddr;
	for (const struct structview::structure & i : this->links)
		if (i.baseaddr > highest)
			highest = i.baseaddr;

	return highest;
}

struct structview::structure structview::finder::find(const uintptr_t addr) const
{
	for (const struct structview::structure & i : this->links)
		if (i.baseaddr == addr)
			return i;
	return structview::structure();
}
