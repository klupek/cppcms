#include "session_sid.h"
#include "md5.h"
#include "session_storage.h"
#include "session_interface.h"
#include <fstream>
#include "cppcms_error.h"
#include "archive.h"
#include "worker_thread.h"

using namespace std;

namespace cppcms {
namespace details {

sid_generator::sid_generator()
{
	session_counter=0;
	ifstream urandom("/dev/urandom");
	if(!urandom.good() || urandom.get(uid,16).fail()) {
		throw cppcms_error("Failed to read /dev/urandom");
	}
}

std::string sid_generator::operator()()
{
	md5_state_t st;
	char res[33];
	unsigned char md5[16];
	md5_init(&st);
	md5_append(&st,(md5_byte_t*)uid,sizeof(uid));
	md5_append(&st,(md5_byte_t*)&session_counter,sizeof(session_counter));
	session_counter++;
	md5_finish(&st,md5);
	for(int i=0;i<16;i++) {
		snprintf(res+i*2,3,"%02x",md5[i]);
	}
	return std::string(res);
}


} // namespace details

namespace {
	struct cached_data : public serializable {
		time_t timeout;
		std::string data;
		virtual void load(archive &a)
		{
			a>>timeout>>data;
		}
		virtual void save(archive &a) const 
		{
			a<<timeout<<data;
		}

	};
}


bool session_sid::valid_sid(std::string const &id)
{
	if(id.size()!=32)
		return false;
	for(int i=0;i<32;i++) {
		char c=id[i];
		bool is_low_x_digit=('0'<=c && c<='9') || ('a'<=c && c<='f');
		if(!is_low_x_digit)
			return false;
	}
	return true;
}

string session_sid::key(std::string sid)
{
	return "cppcms_session_"+sid;
}

void session_sid::save(session_interface *session,std::string const &data,time_t timeout)
{
	string id=session->get_session_cookie();
	if(!valid_sid(id)) {
		id=sid(); // if id not valid create new one
	}
	else if(cache){
		session->get_worker().cache.rise(key(id));
	}
	storage->save(id,timeout,data);
	session->set_session_cookie(id); // Renew cookie or set new one
	if(cache) {
		cached_data cdata;
		cdata.timeout=timeout;
		cdata.data=data;
		session->get_worker().cache.store_data(
			key(id),
			cdata,
			set<string>(),
			timeout - time(NULL));
	}
}

bool session_sid::load(session_interface *session,std::string &data,time_t &timeout)
{
	string id=session->get_session_cookie();
	if(!valid_sid(id))
		return false;
	if(cache){
		cached_data cdata;
		if(session->get_worker().cache.fetch_data(key(id),cdata,false)) {
			data.swap(cdata.data);
			timeout=cdata.timeout;
			return true;
		}
	}
	if(!storage->load(id,&timeout,data)) {
		return false;
	}
	if(!cache)
		return true;
	
	cached_data cdata;
	cdata.timeout=timeout;
	cdata.data=data;
	
	session->get_worker().cache.store_data(
			key(id),
			cdata,
			set<string>(),
			timeout-time(NULL)
		);
	return true;
}

void session_sid::clear(session_interface *session)
{
	string id=session->get_session_cookie();
	if(valid_sid(id)) {
		storage->remove(id);
		if(cache)
			session->get_worker().cache.rise(key(id));
	}
}


} // namespace cppcms
