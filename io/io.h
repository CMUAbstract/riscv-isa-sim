#ifndef IO_H
#define IO_H

#include "json11.h"
#include "tinycon.h"

namespace io {
	using tinycon = tinyConsole;
	using json = json11::Json;
	class serializable {
	public:
		virtual json to_json() const = 0;
	};
};

#endif
