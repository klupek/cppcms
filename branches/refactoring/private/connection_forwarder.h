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
#ifndef CPPCMS_CONNECTION_FORWARDER_H
#define CPPCMS_CONNECTION_FORWARDER_H

#include "application.h"

namespace cppcms {

	class CPPCMS_API connection_forwarder : public application {
	public:
		connection_forwarder(cppcms::service &srv,std::string const &ip,int port);
		~connection_forwarder();
		virtual void main(std::string);
	private:

		struct data;
		booster::hold_ptr<data> d;
		std::string ip_;
		int port_;
	};
}

#endif