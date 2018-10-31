#ifndef MISC_H
#define MISC_H

#include <unordered_map>
#include <string>

extern size_t glob_guid;

static size_t gen_guid(size_t len = 0) {
	if(len > 0) {
		std::string str;
	    for(size_t i = 0; i < len; i++){
	        auto d = rand() % 26 + 'a';
	        str.push_back(d);
	    }
	    return std::hash<std::string>{}(str);
	}
	return glob_guid++;
}

#endif