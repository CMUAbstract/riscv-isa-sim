#ifndef UUID_H
#define UUID_H

#include <string>
#include <uuid/uuid.h>

namespace hstd {

	struct uuid {
		uuid() { uuid_generate_time_safe(id); }
		uuid(uuid const& copy) { uuid_copy(id, copy.id); }
		uuid& operator=(uuid rhs) {
			rhs.swap(*this);
			return *this;
		}
		void swap(uuid& other) noexcept {
			std::swap(id,  other.id);
		}
		static uuid generate(void) {
			uuid generated;
			uuid_generate_time_safe(generated.id);
			return generated;
		}
		std::string to_string(void) const {
			char id_str[37];
		    uuid_unparse_lower(id, id_str);
		    return id_str;
		}
		bool operator<(const uuid &other) { 
			return uuid_compare(id, other.id) < 0; 
		}
		bool operator==(const uuid &other) { 
			return uuid_compare(id, other.id) == 0; 
		}
		uuid_t id;
	};

};

namespace std {
	template <>
	struct hash<hstd::uuid> {
		size_t operator()(const hstd::uuid& id) const {
			return std::hash<std::string>{}(id.to_string());
		}
	};
}

#endif