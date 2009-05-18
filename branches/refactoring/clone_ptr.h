#ifndef CPPCMS_UTIL_CLONE_PTR_H
#define CPPCMS_UTIL_CLONE_PTR_H

namespace cppcms { namespace util {

	// Similar to auto_ptr, but actually creates
	// copy of target instead moving it
	template<typename T>
	class clone_ptr {
		T *ptr_;
	public:
		clone_ptr() : ptr_(0) {}
		clone_ptr(T *v) : ptr_(v) {}
		clone_ptr(clone_ptr const &other) : ptr_(0)
		{
			if(other.ptr_)
				ptr_=other.ptr_->clone();
		}
		clone_ptr const &operator=(clone_ptr const &other)
		{
			if(this != &other) {
				if(ptr_) {
					delete ptr_;
					ptr_=0;
				}
				if(other.ptr_) {
					ptr_=other.ptr_->clone();
				}
			}
			return *this;
		}
		~clone_ptr() {
			if(ptr_) delete ptr_;
		}

		T *get() const { return ptr_; }
		T &operator *() const { return *ptr_; }
		T *operator->() const { return ptr_; }

		T *release() { T *tmp=ptr_; ptr_=0; return tmp; }
		void reset(T *p=0)
		{
			if(ptr_) delete ptr_;
			ptr_=p;
		}
	};

}} // cppcms::util

#endif
