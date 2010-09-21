#ifndef THREAD_CHACHE_H
#define THREAD_CHACHE_H
#include "config.h"
#include "base_cache.h"
#include "cache_interface.h"
#include "pthread.h"
#include <map>
#include <list>
namespace cppcms {

using namespace std;

#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)

class thread_cache : public base_cache {
	pthread_mutex_t lru_mutex;
	pthread_rwlock_t access_lock;
	struct container {
		string data;
		typedef std::map<string,container>::iterator pointer;
		list<pointer>::iterator lru;
		list<multimap<string,pointer>::iterator> triggers;
		multimap<time_t,pointer>::iterator timeout;
	};
	typedef container::pointer pointer;
	std::map<string,container> primary;
	multimap<string,pointer> triggers;
	typedef multimap<string,pointer>::iterator triggers_ptr;
	multimap<time_t,pointer> timeout;
	typedef multimap<time_t,pointer>::iterator timeout_ptr;
	list<pointer> lru;
	typedef list<pointer>::iterator lru_ptr;
	unsigned limit;

	string *get(string const &key,set<string> *triggers);
	void delete_node(pointer p);
	void print_all();
	bool debug_mode;
	int fd;

public:
	void set_debug_mode(int fd) { debug_mode=true; this->fd=fd; };
	thread_cache(unsigned pages=0) : limit(pages) {
		pthread_mutex_init(&lru_mutex,NULL);
		pthread_rwlock_init(&access_lock,NULL);
		debug_mode=false;
	};
	void set_size(unsigned l) { limit=l; };
	virtual bool fetch_page(string const &key,string &output,bool gzip);
	virtual bool fetch(string const &key,archive &a,set<string> &tags);
	virtual void rise(string const &trigger);
	virtual void clear();
	virtual void stats(unsigned &keys,unsigned &triggers);
	virtual void store(string const &key,set<string> const &triggers,time_t timeout,archive const &a);
	virtual ~thread_cache();
};

class thread_cache_factory : public cache_factory{
	thread_cache *cache;
public:
	thread_cache_factory(unsigned n) : cache(new thread_cache(n)) {};
	virtual base_cache *get() const { return cache; };
	virtual void del(base_cache *p) const { };
	virtual ~thread_cache_factory() { delete cache; };
};

#endif

}

#endif
