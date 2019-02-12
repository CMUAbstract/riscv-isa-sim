#ifndef LOG_H
#define LOG_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define assert_msg(cond, args...)												\
	if (unlikely(!(cond))) {													\
	fprintf(stderr, "%s => Failed assertion on %d: ",							\
	__FILE__, __LINE__);														\
	fprintf(stderr, args);														\
	fprintf(stderr, "\n");														\
	fflush(stderr);																\
	*reinterpret_cast<int *>(0L) = 42; /*SIGSEGVs*/								\
	exit(1);																	\
	};

#define warn_msg(cond, args...)													\
	if (unlikely(!(cond))) {													\
	fprintf(stderr, "%s => Warning @ %d: ",										\
	__FILE__, __LINE__);														\
	fprintf(stderr, args);														\
	fprintf(stderr, "\n");														\
	fflush(stderr);																\
	};

#define COMMENT SLASH(/)
#define SLASH(s) /##s

#define LOG_ERROR COMMENT
#define LOG_WARN COMMENT
#define LOG_INFO COMMENT
#define LOG_DEBUG COMMENT
#define LOG_LEVEL 3

#if LOG_LEVEL > 0
	#undef LOG_ERROR
	#undef LOG_WARN
	#define LOG_ERROR
	#define LOG_WARN
	#if LOG_LEVEL > 1
		#undef LOG_INFO
		#define LOG_INFO
		#if LOG_LEVEL > 2
			#undef LOG_DEBUG
			#define LOG_DEBUG
		#endif
	#endif
#endif

#if 0
enum typelog {
    DEBUG,
    INFO,
    WARN,
    ERROR,
};

#define LOG(level) LOG##level
#define ASSERT(level, cond) Log(level, __FILE__, __LINE__, cond)

class Log {
public:
	Log(typelog level, const char *file, const uint32_t line)
		: Log(level, file, line, true) {
		assert = false;
	}
	Log(typelog level, const char *file, const uint32_t line, bool _cond) {
		assert = true;
        cond = _cond;
		if(level == ERROR || level == WARN) {
			output = &std::cerr;
		} else {
			output = &std::cout;
		}
        if(cond) {
            std::string label = std::string("[");
            label += get_level(level);
            label += " (" + std::string(file);
            label += " @ " + std::to_string(line);
            label += ")]=> ";
            operator << (label);
        }
    }
    ~Log() {
    	if(!opened && cond) return;
    	opened = false;
		if(!assert || cond) {
			ostream << std::endl;
			std::cout << "HERE" << std::endl;
			*output << ostream.str();
			return;
		}
    	abort();
    }
    template<class T>
    Log &operator<<(const T &msg) {
        if(cond && output != nullptr) {
            opened = true;
            ostream << msg;
        }
        return *this;
    }
protected:
	inline std::string get_level(typelog level) {
        switch(level) {
            case DEBUG: return std::string("DEBUG");
            case INFO: return std::string("\033[0;32mINFO\033[0m");
            case WARN:  return std::string("\033[0;33mWARN\033[0m");
            case ERROR: return std::string("\033[0;31mERROR\033[0m");
            default: return std::string("\033[0;36mDEBUG\033[0m");
        }
    }
    bool opened = false;
    bool assert = false;
	bool cond = false;
	std::ostringstream ostream;
	std::ostream *output;
};
#endif

#endif