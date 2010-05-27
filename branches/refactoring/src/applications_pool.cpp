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
#define CPPCMS_SOURCE
#include <cppcms/applications_pool.h>
#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/json.h>
#include <set>
#include <vector>
#include <cppcms/config.h>
#include <booster/regex.h>
#include <booster/shared_ptr.h>
#include <booster/thread.h>

namespace cppcms {

	struct applications_pool::basic_app_data : public booster::noncopyable {
		basic_app_data(std::string script) :
			script_name(script),
			match(0),
			use_regex(0)
		{
		}
		basic_app_data(std::string script,std::string pat,int select) :
			script_name(script),
			expr(pat),
			match(select),
			use_regex(1)
		{
		}
		std::string script_name;
		booster::regex expr;
		int match;
		bool use_regex;
	private:
		void check() const
		{
			if(	!script_name.empty() 
				&& script_name[0]!='/' 
				&& script_name[0]!='.' 
				&& script_name!="*")
			{
				throw cppcms_error("Scipt name should be either '*', start with '.' or '/' or be empty");
			}
		}
	};
	
	struct applications_pool::app_data : public applications_pool::basic_app_data {
		app_data(std::string script,std::auto_ptr<applications_pool::factory> f) :
			basic_app_data(script),
			factory(f),
			size(0)
		{
		}
		app_data(std::string script,std::string pat,int select,std::auto_ptr<applications_pool::factory> f) :
			basic_app_data(script,pat,select),
			factory(f),
			size(0)
		{
		}

		std::auto_ptr<applications_pool::factory> factory;

		int size;
		std::set<application *> pool;

		~app_data()
		{
			std::set<application *>::iterator p;
			for(p=pool.begin();p!=pool.end();++p) {
				delete *p;
			}
		}

	};
	struct applications_pool::long_running_app_data : public applications_pool::basic_app_data
	{
		long_running_app_data(std::string script) :
			basic_app_data(script)
		{
		}
		long_running_app_data(std::string script,std::string pat,int select) :
			basic_app_data(script,pat,select)
		{
		}
	};

	struct applications_pool::_data {
		std::vector<booster::shared_ptr<app_data> > apps;
		typedef std::map<application *,booster::shared_ptr<long_running_app_data> > long_running_aps_type;
		long_running_aps_type long_running_aps;
		int limit;
		booster::recursive_mutex mutex;
	};
	typedef booster::unique_lock<booster::recursive_mutex> lock_it;


applications_pool::applications_pool(service &srv,int limit) :
	srv_(&srv),
	d(new applications_pool::_data())
{
	d->limit=limit;
}
applications_pool::~applications_pool()
{
}

std::string applications_pool::script_name()
{
	return srv_->settings().get("service.default_script_name","*");
}

void applications_pool::mount(std::auto_ptr<factory> aps)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(script_name(),aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string path_info,int select)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(script_name(),path_info,select,aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string script_name)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(script_name,aps)));
}
void applications_pool::mount(std::auto_ptr<factory> aps,std::string script_name,std::string path_info,int select)
{
	lock_it lock(d->mutex);
	d->apps.push_back(booster::shared_ptr<app_data>(new app_data(script_name,path_info,select,aps)));
}

void applications_pool::mount(booster::intrusive_ptr<application> app)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(script_name()));
}
void applications_pool::mount(booster::intrusive_ptr<application> app,std::string path_info,int select)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(script_name(),path_info,select));
}
void applications_pool::mount(booster::intrusive_ptr<application> app,std::string script_name)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(script_name));
}
void applications_pool::mount(booster::intrusive_ptr<application> app,std::string script_name,std::string path_info,int select)
{
	lock_it lock(d->mutex);
	d->long_running_aps[app.get()]=
		booster::shared_ptr<long_running_app_data>(new long_running_app_data(script_name,path_info,select));
}

bool applications_pool::matched(basic_app_data &data,std::string script_name,std::string path_info,std::string &matched)
{
	std::string const expected_name=data.script_name;
	if(expected_name!="*" && !expected_name.empty()) {
		if(expected_name[0]=='/') {
			if(script_name!=expected_name)
				return false;
		}
		else { // if(sn[0]=='.') 
			if(	script_name.size() <= expected_name.size() 
				|| script_name.substr(script_name.size() - expected_name.size())!=expected_name)
			{
				return false;
			}
		}
	}
	else if(expected_name=="*" && script_name.empty())
		return false;
	else if(expected_name.empty() && !script_name.empty())
		return false;
	
	booster::cmatch match;
	if(!data.use_regex) {
		matched=path_info;
		return true;
	}
	else if(booster::regex_match(path_info.c_str(),match,data.expr)) {
		matched=match[data.match];
		return true;
	}
	else {
		return false;
	}
}


booster::intrusive_ptr<application> applications_pool::get(std::string script_name,std::string path_info,std::string &m)
{
	lock_it lock(d->mutex);
	for(unsigned i=0;i<d->apps.size();i++) {
		if(!matched(*d->apps[i],script_name,path_info,m))
			continue;

		if(d->apps[i]->pool.empty()) {
			booster::intrusive_ptr<application> app=(*d->apps[i]->factory)(*srv_).release();
			app->pool_id(i);
			return app;
		}
		d->apps[i]->size--;
		booster::intrusive_ptr<application> app(*(d->apps[i]->pool.begin()));
		d->apps[i]->pool.erase(app.get());
		return app;
	}
	for(_data::long_running_aps_type::iterator p=d->long_running_aps.begin();p!=d->long_running_aps.end();++p){
		if(!matched(*p->second,script_name,path_info,m))
			continue;
		booster::intrusive_ptr<application> app=p->first;
		return app;
	}
	return 0;
}

void applications_pool::put(application *app)
{
	lock_it lock(d->mutex);
	if(!app) return;
	int id=app->pool_id();
	if(id < 0) {
		d->long_running_aps.erase(app);
		delete app;
		return;
	}
	if(unsigned(id) >= d->apps.size() || d->apps[id]->size >= d->limit) {
		delete app;
		return;
	}
	d->apps[id]->pool.insert(app);
	d->apps[id]->size++;
}


} //cppcms
