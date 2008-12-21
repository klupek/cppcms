#ifndef SESSION_FILE_STORAGE_H
#define SESSION_FILE_STORAGE_H

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>
#include "session_storage.h"

namespace cppcms {

namespace storage {

class io : private boost::noncopyable {
protected:
	std::string dir;
	int lock_id(std::string const &sid) const;
	std::string mkname(std::string const &sid) const;
	virtual void close(int fid) const;
public:
	std::string const &get_dir() const { return dir; }	
	io(std::string d) : dir(d) {}
	virtual void wrlock(std::string const &sid) const = 0;
	virtual void rdlock(std::string const &sid) const = 0;
	virtual void unlock(std::string const &sid) const = 0;
	virtual void write(std::string const &sid,time_t timestamp,void const *buf,size_t len) const;
	virtual bool read(std::string const &sid,time_t &timestamp,std::vector<unsigned char> *out) const;
	virtual void unlink(std::string const &sid) const;
	virtual ~io(){};
};

class local_io : public io {
protected:
	pthread_rwlock_t *locks;
public:
	local_io(std::string dir,pthread_rwlock_t *l);
	virtual void wrlock(std::string const &sid) const;
	virtual void rdlock(std::string const &sid) const;
	virtual void unlock(std::string const &sid) const;
};

class nfs_io : public io {
	int fid;
protected:
	virtual void close(int fid);
public:
	nfs_io(std::string dir);
	virtual void wrlock(std::string const &sid) const;
	virtual void rdlock(std::string const &sid) const;
	virtual void unlock(std::string const &sid) const;
	~nfs_io();
};

class thread_io : public local_io
{
	pthread_rwlock_t *create_locks();
public:
	thread_io(std::string dir);
	~thread_io();
};

class shmem_io : public local_io
{
	int creator_pid;
	pthread_rwlock_t *create_locks();
public:
	shmem_io(std::string dir);
	~shmem_io();
};

} // storage

class session_file_storage : public session_server_storage {
	boost::shared_ptr<storage::io> io;
public:
	static void gc(boost::shared_ptr<storage::io>);
	session_file_storage(boost::shared_ptr<storage::io> io_) : io(io_) {}
	virtual void save(std::string const &sid,time_t timeout,std::string const &in);
	virtual bool load(std::string const &sid,time_t *timeout,std::string &out);
	virtual void remove(std::string const &sid) ;
	virtual ~session_file_storage(){};
};

} // cppcms


#endif
