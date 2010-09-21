#include "cppcms_error.h"
#include <iostream>
#include <string.h>
using namespace std;


namespace cppcms {

cppcms_error::cppcms_error(int err,std::string const &error) :
	std::runtime_error(error+":" + strerror(err))	
{
}

// Unfortunatly I can't use XSI-compliant strerror_r() under g++
// it always gives GNU strerror_r, thus it is wrapped

namespace {
	string strerror_wrapper(int value,char *buf)
	{
		return buf;
	}
	string strerror_wrapper(char const *err,char *buf)
	{
		return err;
	}
}

std::string cppcms_error::strerror(int err)
{
	char buf[256] = {0};
	return strerror_wrapper(strerror_r(err,buf,sizeof(buf)),buf);
}

}
