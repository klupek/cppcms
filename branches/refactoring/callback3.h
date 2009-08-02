#ifndef CPPCMS_UTIL_CALLBACK3_H
#define CPPCMS_UTIL_CALLBACK3_H
#include "clone_ptr.h"

namespace cppcms { namespace util {

template<typename P1,typename P2,typename P3>
class callback3 {

	struct callable {
		virtual void call(P1,P2,P3) = 0;
		virtual callable *clone() const = 0;
		virtual ~callable() {};
	};

	template<typename T>
	struct callable_functor : public callable{
		T func;
		callable_functor(T f) : func(f) {}
		virtual ~callable_functor() { }
		virtual void call(P1 p1,P2 p2,P3 p3) { func(p1,p2,p3); }
		virtual callable *clone() const { return new callable_functor<T>(func); }
	};

	clone_ptr<callable> call_ptr;
public:
	typedef void result_type;

	void operator()(P1 p1,P2 p2,P3 p3) const
	{
		if(call_ptr.get()) {
			call_ptr->call(p1,p2,p3);
		}
	}

	callback3(){}

	template<typename T>
	callback3(T c) : call_ptr(new callable_functor<T>(c)) 
	{
	}
	
	template<typename T>
	callback3 const &operator=(T c)
	{
		call_ptr.reset(new callable_functor<T>(c));
		return *this;
	}

	void swap(callback3 &other)
	{
		call_ptr.swap(other.call_ptr);
	}
};



}} // cppcms util


#endif