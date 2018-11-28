#ifndef REPL_POLICY_H
#define REPL_POLICY_H

#include <vector>
#include <map>

struct repl_cand_t {
	repl_cand_t() : id(0) {}
	repl_cand_t(uint32_t _id) : id(_id) {}
	uint32_t operator*() const { return id; }
protected:
	uint32_t id;
};

class mem_event_t;
class repl_policy_t {
public:
	repl_policy_t(uint32_t _lines) : lines(_lines) {}
	virtual void update(uint32_t id, mem_event_t *req) = 0;
	virtual uint32_t rank(mem_event_t *req, std::vector<repl_cand_t> *cands) = 0;
	virtual void replaced(uint32_t id) = 0;
protected:
	uint32_t lines;
};

class lru_repl_policy_t: public repl_policy_t {
public:
	lru_repl_policy_t(uint32_t _lines) 
		: repl_policy_t(_lines), timestamps(lines, 0) {}
	void update(uint32_t id, mem_event_t *req);
	uint32_t rank(mem_event_t *req, std::vector<repl_cand_t> *cands);
	void replaced(uint32_t id);
protected:
	std::vector<uint32_t> timestamps;
	uint32_t timestamp;
};

template <typename T>
repl_policy_t *create_repl_policy(uint32_t _lines) {
	return new T(_lines);
}

const std::map<std::string, repl_policy_t*(*)(uint32_t _lines)> 
repl_policy_type_map = {
	{"lru", &create_repl_policy<lru_repl_policy_t>}
};

#endif
