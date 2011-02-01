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
#include <cppcms/url_mapper.h>
#include <cppcms/application.h>
#include <cppcms/cppcms_error.h>
#include <map>

#include <stdlib.h>

namespace cppcms {
	struct url_mapper::data 
	{
		data() : parent(0),this_application(0) {}
		std::string this_name;
		application *parent;
		application *this_application;

		struct entry {
			std::vector<std::string> parts;
			std::vector<int> indexes;
			std::vector<std::string> keys;
			application *child;
			entry() : child(0) {}
		};

		typedef std::map<size_t,entry> by_size_type;
		typedef std::map<std::string,by_size_type> by_key_type;
		typedef std::map<std::string,std::string> helpers_type;

		by_key_type by_key;
		helpers_type helpers;
		std::string root;

		entry const &get_entry(std::string const &key,size_t params_no) const
		{
			by_key_type::const_iterator kp = by_key.find(key);
			if(kp == by_key.end())
				throw cppcms_error("url_mapper: key " + key + " not found");
			by_size_type::const_iterator sp = kp->second.find(params_no);
			if(sp == kp->second.end())
				throw cppcms_error("url_mapper: invalid number of parameters for " + key);
			return sp->second;
		}

		url_mapper &child(std::string const &name)
		{
			entry const &fmt = get_entry(name,1);
			if(!fmt.child) {
				throw cppcms_error("url_mapper: the key " + name + " is not child application key");
			}
			return fmt.child->mapper();
		}

		void map(	std::string const key,
				filters::streamable const * const *params,
				size_t params_no,
				std::map<std::string,std::string> const &data_helpers,
				std::ostream &output) const
		{

			entry const &formatting = get_entry(key,params_no);
			
			std::ostringstream ss;
			std::ostream *out = 0;

			if(parent) {
				ss.copyfmt(output);
				out = &ss;
			}
			else {
				out = &output;
				*out << root;
			}

			for(size_t i=0;i<formatting.parts.size();i++) {
				*out << formatting.parts[i];
				if( i < formatting.indexes.size() ) {
					if(formatting.indexes[i]==0) {
						std::string const &hkey = formatting.keys[i];
						std::map<std::string,std::string>::const_iterator p = data_helpers.find(hkey);
						if(p != data_helpers.end()) {
							*out << p->second;
						}
					}
					else {
						size_t index = formatting.indexes[i] - 1;
						if(index >= params_no) {
							throw cppcms_error("url_mapper: Index of parameter out of range");
						}
						(*params[index])(*out);
					}
				}
			}

			if(parent) {
				parent->mapper().map(output,this_name,ss.str());
			}

		}

	};

	void url_mapper::assign(std::string const &key,std::string const &url)
	{
		real_assign(key,url,0);
	}
	void url_mapper::real_assign(std::string const &key,std::string const &url,application *child)
	{
		if(	key.empty() 
			|| key.find('/') != std::string::npos 
			|| key ==".." 
			|| key == "." )
		{
			throw cppcms_error("cppcms::url_mapper: key may not be '' , '.' or '..' and must not include '/' in it");
		}

		data::entry e;
		std::string::const_iterator prev = url.begin(), p = url.begin();

		int max_index = 0;

		while(p!=url.end()) {
			if(*p=='{') {
				e.parts.push_back(std::string(prev,p));
				prev = p;
				while(p!=url.end()) {
					if(*p=='}') {
						std::string const hkey(prev+1,p);
						prev = p+1;
						if(hkey.size()==0) {
							throw cppcms_error("cppcms::url_mapper: empty index between {}");
						}
						bool all_digits = true;
						for(unsigned i=0;all_digits && i<hkey.size();i++) {
							if(hkey[i] < '0' || '9' <hkey[i])
								all_digits = false;
						}
						if(!all_digits) {
							e.indexes.push_back(0);
							e.keys.push_back(hkey);
						}
						else {
							int index = atoi(hkey.c_str());
							if(index == 0)
								throw cppcms_error("cppcms::url_mapper: index 0 is invalid");
							max_index = std::max(index,max_index);
							e.indexes.push_back(index);
							e.keys.resize(e.keys.size()+1);
						}
						break;
					}
					else
						p++;
				}
				if(p==url.end())
					throw cppcms_error("cppcms::url_mapper: '{' in url without '}'");
				p++;
			}
			else if(*p=='}') {
				throw cppcms_error("cppcms::url_mapper: '}' in url without '{'");
			}
			else
				p++;
		}
		if(child && max_index!=1) {
			throw cppcms_error("cppcms::url_mapper the application mapping should use only 1 parameter");
		}
		e.parts.push_back(std::string(prev,p));
		e.child = child;
		d->by_key[key][max_index] = e;
	}

	void url_mapper::set_value(std::string const &key,std::string const &value)
	{
		root_mapper()->d->helpers[key]=value;
	}
	void url_mapper::clear_value(std::string const &key)
	{
		root_mapper()->d->helpers.erase(key);
	}

	url_mapper &url_mapper::child(std::string const &name)
	{
		return d->child(name);
	}
	
	url_mapper *url_mapper::root_mapper()
	{
		if(d->parent)
			return &d->parent->root()->mapper();
		else
			return this;
	}

	void url_mapper::mount(std::string const &name,std::string const &url,application &app)
	{
		app.mapper().d->parent = d->this_application;
		app.mapper().d->this_name = name;
		real_assign(name,url,&app);
		// Copy all values to root most one
		std::map<std::string,std::string> &values = app.mapper().d->helpers;
		std::map<std::string,std::string>::const_iterator p;
		for(p=values.begin();p!=values.end();++p) {
			set_value(p->first,p->second);
		}
		values.clear();
	}

	url_mapper::url_mapper(cppcms::application *my_app) : d(new url_mapper::data())
	{
		d->this_application = my_app;
	}
	url_mapper::~url_mapper()
	{
	}

	std::string url_mapper::root()
	{
		return d->root;
	}

	void url_mapper::root(std::string const &r)
	{
		d->root = r;
	}


	url_mapper &url_mapper::get_mapper_for_key(std::string const &key,std::string &real_key)
	{
		url_mapper *mapper = this;
		size_t pos = 0;
		if(key.empty()) {
			real_key.clear();
			return *mapper;
		}
		if(key[0]=='/') {
			mapper = &mapper->topmost();
			pos = 1;
		}
		for(;;) {
			size_t end = key.find('/',pos);
			if(end == std::string::npos) {
				real_key = key.substr(pos);
				return *mapper;
			}
			size_t chunk_size = end - pos;
			if(key.compare(pos,chunk_size,".") == 0)
				; // Just continue where we are
			else if(key.compare(pos,chunk_size,"..")==0) {
				mapper = &mapper->parent();
			}
			else {
				std::string subapp = key.substr(pos,chunk_size);
				mapper = &mapper->child(subapp);
			}
			pos = end + 1;
		}
	}


	void url_mapper::real_map(	std::string const key,
					filters::streamable const * const *params,
					size_t params_no,
					std::ostream &output)
	{
		std::string real_key;
		url_mapper &mp = get_mapper_for_key(key,real_key);
		mp.d->map(real_key,params,params_no,topmost().d->helpers,output);
	}

	void url_mapper::map(	std::ostream &out,	
				std::string const &key)
	{
		real_map(key,0,0,out);
	}

	void url_mapper::map(	std::ostream &out,	
				std::string const &key,
				filters::streamable const &p1)
	{
		filters::streamable const *params[1]= { &p1 };
		real_map(key,params,1,out);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2)
	{
		filters::streamable const *params[2] = { &p1,&p2 };
		real_map(key,params,2,out);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3)
	{
		filters::streamable const *params[3] = { &p1,&p2,&p3 };
		real_map(key,params,3,out);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4)
	{
		filters::streamable const *params[4] = { &p1,&p2,&p3,&p4 };
		real_map(key,params,4,out);
	}

	url_mapper &url_mapper::parent()
	{
		if(d->parent) {
			return d->parent->mapper();
		}
		throw cppcms_error("url_mapper: no parent found");
	}
	
	url_mapper &url_mapper::topmost()
	{
		application *app = d->this_application;
		if(!app)
			return *this;
		while(app->mapper().d->parent)
			app = app->mapper().d->parent;
		return app->mapper();
	}

	
}
