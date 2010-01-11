#ifndef CPPCMS_SCGI_H
#define CPPCMS_SCGI_H

#include "config.h"
#include <cgicc/CgiInput.h>
#include <boost/noncopyable.hpp>
#include <ostream>
#include <streambuf>
#include <map>
#include <unistd.h>
#include "cppcms_error.h"
#include "cgi_api.h"

namespace cppcms {
using namespace std;
#if !defined(CPPCMS_EMBEDDED) || defined(CPPCMS_EMBEDDED_THREAD)
namespace scgi {
	class scgi_outbuffer : public streambuf
	{
		int fd;
	public:
		scgi_outbuffer(int descriptor) : fd(descriptor) {};
		virtual int overflow ( int c = EOF );
		virtual streamsize xsputn(char const *s,streamsize n);
		virtual ~scgi_outbuffer();
	};

};

class scgi_session : public cgicc::CgiInput,
			public cgi_session,
			public cgicc_connection
{
	int socket;
	scgi::scgi_outbuffer buf;
	map<string,string> envmap;
	cgicc::Cgicc *cgi_ptr;
	std::ostream out_stream;
	long long content_lenght_limit;
public:
	scgi_session(int s,long long limit) :
		 socket(s),
		 buf(s),
		 cgi_ptr(NULL),
		 out_stream(&buf),
		 content_lenght_limit(limit)
	{
	}
	virtual size_t read(char *data, size_t length);
	virtual std::string getenv(const char *var);
	virtual cgicc_connection &get_connection();
	virtual bool prepare();
	virtual string env(char const *variable);
	virtual cgicc::Cgicc &cgi();
	virtual ostream &cout();
	virtual ~scgi_session();
};


class scgi_api : public cgi_api {
	int fd;
	long long limit;
public:
	scgi_api(string socket,int backlog,long long limit);
	virtual int get_socket();
	virtual cgi_session *accept_session();
	virtual ~scgi_api();
};
#endif

};


#endif
