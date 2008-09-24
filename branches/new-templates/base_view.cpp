#include "base_view.h"
#include "worker_thread.h"

namespace cppcms {

string base_view_impl::escape(string const &s)
{
	string content;
	unsigned i,len=s.size();
	content.reserve(len*3/2);
	for(i=0;i<len;i++) {
		char c=s[i];
		switch(c){
			case '<': content+="&lt;"; break;
			case '>': content+="&gt;"; break;
			case '&': content+="&amp;"; break;
			case '\"': content+="&quot;"; break;
			default: content+=c;
		}
	}
	return content;
}

string base_view_impl::urlencode(string const &s)
{
	string content;
	unsigned i,len=s.size();
	content.reserve(3*len);
	for(i=0;i<len;i++){
		char c=s[i];
		if(	('a'<=c && c<='z')
			|| ('A'<=c && c<='Z')
			|| ('0'<=c && c<='9'))
		{
			content+=c;
		}
		else {
			switch(c) {
				case '-':
				case '_':
				case '.':
				case '~':
				case '!':
				case '*':
				case '\'':
				case '(':
				case ')':
				case ';':
				case ':':
				case '@':
				case '&':
				case '=':
				case '+':
				case '$':
				case ',':
				case '/':
				case '?':
				case '%':
				case '#':
				case '[':
				case ']':
					content+=c;
					break;
				default:
				{
					char buf[4];
					snprintf(buf,sizeof(buf),"%%%02x",(unsigned)(c));
					content.append(buf,3);
				}
			};
		}
	};
	return content;
}

void base_view_impl::set_worker(worker_thread *w)
{
	base_worker=w;
	cout_ptr=&w->cout;
};

char const *base_view_impl::gettext(char const *s)
{
	return base_worker->gettext(s);
};

char const *base_view_impl::ngettext(char const *s,char const *p,int n)
{
	return base_worker->ngettext(s,p,n);
};

namespace details {

views_storage &views_storage::instance() {
	static views_storage this_instance;
	return this_instance;
};

void views_storage::add_view(string t,string v,view_factory_t f)
{
	storage[t][v]=f;
}

void views_storage::remove_views(string t)
{
	storage.erase(t);
}

base_view_impl *views_storage::fetch_view(string t,string v)
{
	templates_t::iterator p=storage.find(t);
	if(p==storage.end()) return NULL;
	template_views_t::iterator p2=p->second.find(v);
	if(p2==p->second.end()) return NULL;
	view_factory_t &f=p2->second;
	return f();
}

};
}// CPPCMS
