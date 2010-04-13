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
#ifndef CPPCMS_SERVICE_H
#define CPPCMS_SERVICE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"
#include "function.h"
#include <locale>


namespace cppcms {
	namespace locale { class generator; }
	namespace impl {
		class service;
	}

	class applications_pool;
	class thread_pool;
	class session_pool;
	class cache_pool;
	class views_pool;
	
	namespace json {
		class value;
	}

	class CPPCMS_API service : public util::noncopyable
	{
	public:
		service(json::value const &v);
		service(int argc,char *argv[]);
		~service();

		void run();
		void shutdown();

		json::value const &settings();
		
		cppcms::applications_pool &applications_pool();
		cppcms::thread_pool &thread_pool();
		cppcms::session_pool &session_pool();
		cppcms::views_pool &views_pool();
		cppcms::cache_pool &cache_pool();

		locale::generator const &generator();
		std::locale locale();
		std::locale locale(std::string const &name);

		cppcms::impl::service &impl();

		void post(function<void()> const &handler);
		void after_fork(function<void()> const &handler);
		
		int threads_no();
		int procs_no();
		int process_id();
	private:
		void setup();
		void setup_acceptor(json::value const &,int);
		void load_settings(int argc,char *argv[]);
		void stop();
		void start_acceptor();
		void setup_exit_handling();
		bool prefork();
		util::hold_ptr<impl::service> impl_;
	};

} //




#endif
