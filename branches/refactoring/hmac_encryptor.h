///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_HMAC_ENCRYPTOR_H
#define CPPCMS_HMAC_ENCRYPTOR_H
#include "base_encryptor.h"
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {
namespace sessions {
namespace impl {

class CPPCMS_API hmac_cipher : public base_encryptor {
	void hash(unsigned char const *,size_t,unsigned char md5[16]);
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout);
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL);
	hmac_cipher(std::string key);
	struct block {
		union {
			char data[16];
			uint64_t counter;
		} seed;
		char buf[16];
		unsigned left;
	};
private:
	static boost::thread_specific_ptr<block> seed;
	void salt(char *s,int n);
};

} // impl
} // sessions
} // cppcms


#endif
