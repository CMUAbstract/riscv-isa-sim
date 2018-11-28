#include "core_event.h"

#include <sstream>

#include "working_set.h"

std::string insn_event_t::to_string() {
	std::ostringstream os;
	os << " (" << cycle << ", 0x" << std::hex << data->ws.pc << ")"; 
	return os.str();
}