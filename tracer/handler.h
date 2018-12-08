#ifndef HANDLER_H
#define HANDLER_H

template<class T>
struct handler_t {
	handler_t(): handler_t(1) {}
	handler_t(size_t _q_size): q_size(_q_size), q_idx(0) {}
	virtual void process(T) = 0;
	bool get_status() { return q_idx < q_size; }
	size_t q_size;
	size_t q_idx;
};

#define GET_STATUS_FXN 															\
	template<class T>															\
	bool get_status() { return handler_t<T>::get_status(); }

#endif