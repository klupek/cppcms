#ifndef CPPCMS_HTTP_RESPONSE_H
#define CPPCMS_HTTP_RESPONSE_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

#include <string>
#include <iostream>

namespace cppcms {
namespace cgi { class io; }
namespace http {

	class context;
	class cookie;
	class CPPCMS_API response : public util::noncopyable {
	public:
		// RFC 2616 sec. 6.1.1
		typedef enum {
			continue_transfer = 100,
			switching_protocol = 101,
			ok = 200,
			created = 201,
			accepted = 202,
			non_authoritative_information = 203,
			no_content = 204,
			reset_content = 205,
			partial_content = 206,
			multiple_choices = 300,
			moved_permanently = 301,
			found = 302,
			see_other = 303,
			not_modified = 304,
			use_proxy = 305,
			temporary_redirect = 307,
			bad_request = 400,
			unauthorized = 401,
			payment_required = 402,
			forbidden = 403,
			not_found = 404,
			method_not_allowed = 405,
			not_acceptable = 406,
			proxy_authentication_required = 407,
			request_time_out = 408,
			conflict = 409,
			gone = 410,
			precondition_failed = 412,
			request_entity_too_large = 413,
			request_uri_too_large = 414,
			unsupported_media_type = 415,
			requested_range_not_satisfiable = 416,
			expectation_failed = 417,
			internal_server_error = 500,
			not_implemented = 501,
			bad_gateway = 502,
			service_unavailable = 503,
			gateway_timeout = 504,
			http_version_not_supported = 505
		} status_type;
		typedef enum {
		// synchronous io
			normal, // write request, use buffering, compression,
			nogzip, // as normal but disable gzip
			direct, // use direct connection for transferring huge
				// amount of data, for example big csv, file download
		// async TODO
			asynchronous,
				// the data is buffered and transferred asynchronously
				// in one chunk only, for long poll
			asynchronous_chunked,
				// allow many chunks being transferred, each
				// push transferees one chunk
			asynchronous_multipart,
				// use multipart transfer encoding, requires parameter
				// content type, default is "multipart/mixed"
			asynchronous_raw
				// use your own asynchronous data transfer

		} io_mode_type;


		// Standard HTTP Response Headers RFC 2616

		void accept_ranges(std::string const &);
		void age(unsigned seconds);
		void allow(std::string const &);
		void cache_control(std::string const &);
		void content_encoding(std::string const &);
		void content_language(std::string const &);
		void content_length(unsigned long long len);
		void content_location(std::string const &);
		void content_md5(std::string const &);
		void content_range(std::string const &);
		void content_type(std::string const &);
		void date(time_t);
		void etag(std::string const &);
		void expires(time_t);
		void last_modified(time_t);
		void location(std::string const &);
		void pragma(std::string const &);
		void proxy_authenticate(std::string const &);
		void retry_after(std::string const &);
		void retry_after(unsigned);
		void status(int code);
		void status(int code,std::string const &message);
		void trailer(std::string const &);
		void transfer_encoding(std::string const &);
		void vary(std::string const &);
		void via(std::string const &);
		void warning(std::string const &);
		void www_authenticate(std::string const &);


		void set_header(std::string const &name,std::string const &value);
		std::string get_header(std::string const &name);
		void erase_header(std::string const &);
		void clear();

		void set_content_header(std::string const &content_type);
		void set_html_header();
		void set_xhtml_header();
		void set_plain_text_header();
		void set_redirect_header(std::string const &location,int status = found);
		void set_cookie(cookie const &);

		io_mode_type io_mode();
		void io_mode(io_mode_type);
		std::ostream &out();
		void copy_to_cache(std::string const &key);

		static std::string make_http_time(time_t);
		static char const *status_to_string(int status);

		response(context &);
		~response();
	private:
		bool need_gzip();
		void write_http_headers();

		struct data;
		util::hold_ptr<data> d;

		context &context_;
		std::ostream *stream_;
		io_mode_type io_mode_;
		std::string cache_key_;

		uint32_t disable_compression_ : 1;
		uint32_t ostream_requested_ : 1;
		uint32_t copy_to_cache_ : 1;
		uint32_t reserved_ : 29;
	};

} /* http */
} /* cppcms */


#endif