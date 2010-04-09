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
#ifndef CPPCMS_AIO_TIMER_H
#define CPPCMS_AIO_TIMER_H

#include "defs.h"
#include "noncopyable.h"
#include "callback1.h"
#include "hold_ptr.h"

namespace cppcms {
	class service;
	namespace aio {
		///
		/// \brief timer is a class that is used to run asynchronous requests after certain time period
		///

		class CPPCMS_API timer : public util::noncopyable {
		public:

			///
			/// Create a timer, it recieves the service as parameter
			///
			timer(service &srv);
			~timer();

			typedef util::callback1<bool> handler;

			//
			// true if was error or cancelation
			//

			///
			/// Start asynchronous wait procedure. handler \a h will be called with true if error 
			/// had occured or the timer was stopped.
			///
			/// Should be called after expires_from_now or expires_at were called.
			///
			/// You should call them each time before \a async_wait is called
			///
			void async_wait(handler const &h);

			///
			/// Abort timer
			///
			void cancel();

			///
			/// Set expiration time in seconds
			///
			void expires_from_now(int seconds);

			///
			/// Set expiration time in miliseconds
			///
			void expires_from_now(int seconds,int milliseconds);
			///
			/// Set expiration point. At should be in future
			///
			void expires_at(time_t at);
		private:
			struct data;
			util::hold_ptr<data> d;
		};
	}
}



#endif
