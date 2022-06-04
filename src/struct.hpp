#ifndef STRUCT_HPP
#define STRUCT_HPP

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace structview {
	struct base_struct {
		uint32_t ptr1	: 32;	// 0x0
		uint32_t ptr2	: 32;	// 0x4
		uint32_t ptr3	: 32;	// 0x8
		uint32_t unkwn1	: 32;	// 0xC
		uint32_t value	: 32;	// 0x10
		struct {		// 0x14
			bool f1	: 1;	// 0x14
			bool f2	: 1;	// 0x15
		} flags;
		uint32_t unkwn2	: 32; 	// 0x18
		uint32_t unkwn3	: 32;	// 0x1C
		static unsigned long size;
	};				// 4 byte * 8 = 32

	intptr_t baseoffset(const uintptr_t base, const uintptr_t addr);
	std::string stringptr(const uintptr_t ptr, const std::string & name, const uintptr_t base);
	std::string stringunkwn(const uintptr_t value, const std::string & name, const uintptr_t base);
	std::string stringvalue(const uintptr_t value);

	struct structure {
		const uintptr_t baseaddr;
		const bool iscopy;
		std::shared_ptr<struct structview::base_struct> data;
		std::vector<struct structview::structure> points_to;
		std::vector<struct structview::structure> pointed_from;

		structure(void);
		structure(const structview::structure & copy);
		structure(structview::structure && move);
		structure(const uintptr_t addr, struct proc::process & proc);

		bool operator==(const struct structview::structure &) const;
		bool operator==(const uintptr_t) const;

		bool isptrto(const struct structview::structure & target) const;
		std::vector<uintptr_t> getptrs(void) const;
		bool ptrat(const uintptr_t ptr, const struct structview::structure & target) const;
		std::string stringstruct(void) const;
	};

	class finder {
	public:
		finder(std::unique_ptr<struct proc::process> & proc);
		bool iterate(const uintptr_t addr);
		void refeachother(void);
		void print(void) const;
	private:
		std::vector<struct structview::structure> links;
		std::unique_ptr<struct proc::process> & proc;

		bool add(const uintptr_t addr);
		bool exist(const struct structview::structure & src) const;
		bool exist(const uintptr_t addr) const;
		void addcrossref(struct structview::structure & target);
		uintptr_t loweststruct(void) const;
		uintptr_t higheststruct(void) const;
		struct structview::structure find(const uintptr_t addr) const;
	};

	void structprint(const structview::finder & f, const structview::structure & st);
}

#endif // STRUCT_HPP
