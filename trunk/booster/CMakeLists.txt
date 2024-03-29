#
#  Copyright (c) 2010 Artyom Beilis (Tonkikh)
#
#  Distributed under the Boost Software License, Version 1.0. (See
#  accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 2.6)
project(booster)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CPack)
enable_testing()
#
# Most important includes
#

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(lib/test)

if(WIN32 OR CYGWIN)
	set(IS_WINDOWS TRUE)
else()
	set(IS_WINDOWS FALSE)
endif()

if(WIN32 AND NOT CYGWIN)
	set(WINDOWS_NATIVE TRUE)
else()
	set(WINDOWS_NATIVE FALSE)
endif()

if(CYGWIN)
	set(IS_CYGWIN TRUE)
else()
	set(IS_CYGWIN FALSE)
endif()



# Options

if(NOT LIBDIR)
	set(LIBDIR lib CACHE STRING "Library installation directory" FORCE)
endif()

option(USE_STLPORT		"Build with STLPort library" OFF)
option(USE_LIBCXX		"Build with CXX library" OFF)
option(USE_PTHREAD	"Use pthreads API on windows" OFF)
option(DISABLE_SHARED	"Disable shared libraries build" OFF)
option(DISABLE_STATIC	"Disable static libraries build" OFF)
option(DISABLE_ICU_LOCALE	"Disable icu locale backend" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	option(DISABLE_STD_LOCALE	"Disable std locale backend" ON)
else()
	option(DISABLE_STD_LOCALE	"Disable std locale backend" OFF)
endif()
option(USE_WINDOWS6_API		"Use Windows 6 API (Vista, 7)" OFF)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" OR APPLE)
	option(DISABLE_POSIX_LOCALE	"Disable POSIX locale backend" OFF)
else()
	option(DISABLE_POSIX_LOCALE	"Disable POSIX locale backend" ON)
endif()

if(IS_WINDOWS) 
	option(DISABLE_WINAPI_LOCALE	"Disable Win32API locale backend" OFF)
else()
	option(DISABLE_WINAPI_LOCALE	"Disable Win32API locale backend" ON)
endif()

if(WINDOWS_NATIVE)
	option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" ON)
else()
	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" ON)
	else()
		option(DISABLE_ICONV	"Disable iconv library (default On on Windows)" OFF)
	endif()
endif()

if(USE_LIBCXX)
	find_path(STLPORT_INCLUDE iostream)
	find_library(STLPORT_LIB c++)
	if(NOT STLPORT_INCLUDE OR NOT STLPORT_LIB)
		message(FATAL_ERROR "Can't find stlport include or library")
	else()
		include_directories(${STLPORT_INCLUDE})
	endif()
endif()


if(USE_STLPORT) 
	find_path(STLPORT_INCLUDE stlport/iostream)
	find_library(STLPORT_LIB stlport)
	if(NOT STLPORT_INCLUDE OR NOT STLPORT_LIB)
		message(FATAL_ERROR "Can't find stlport include or library")
	else()
		include_directories(${STLPORT_INCLUDE}/stlport)
	endif()
endif()

# Fixing options if libraries are not found

if(NOT DISABLE_ICU_LOCALE)
	message("-- Looking for ICU libraries")

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		if(MSVC)
			set(ICU_SUFFIX "d")
		endif()
	endif()

	find_library(ICU_UC icuuc${ICU_SUFFIX})
	find_library(ICU_DATA NAMES icudt icudata)
	find_library(ICU_I18N NAMES icuin${ICU_SUFFIX} icui18n${ICU_SUFFIX})
	find_path(ICU_INCLUDE_DIR unicode/unistr.h)

	if(ICU_UC AND ICU_DATA AND ICU_I18N AND ICU_INCLUDE_DIR)
		message("-- ICU Found, building booster locale")
		include_directories(${ICU_INCLUDE_DIR})
	else()
		message("-- ICU not found, disabling ICU localization backend")
		set(DISABLE_ICU_LOCALE ON)
	endif()
endif()


if(NOT DISABLE_ICU_LOCALE)
	add_definitions(-DBOOSTER_LOCALE_WITH_ICU)
endif()

# Checking for iconv

if(NOT DISABLE_ICONV) 
	check_cxx_source_compiles(
		"#include <iconv.h> 
		 int main() { iconv_t v=iconv_open((char *)0,(char *)0); }"
		 LIBC_ICONV)
	if(NOT LIBC_ICONV)
		find_path(ICONV_INCLUDE_DIR iconv.h)
		find_library(ICONV_LIB iconv)
		if(ICONV_LIB AND ICONV_INCLUDE_DIR)
			set(BOOSTER_LOCALE_HAVE_ICONV 1)
			include_directories(${ICONV_INCLUDE_DIR})
		endif()
	else()
		set(BOOSTER_LOCALE_HAVE_ICONV 1)
	endif()

	if(BOOSTER_LOCALE_HAVE_ICONV)
		add_definitions(-DBOOSTER_LOCALE_WITH_ICONV)
	else()
		add_definitions(-DBOOSTER_LOCALE_NO_ICONV)
	endif()
else()
	add_definitions(-DBOOSTER_LOCALE_NO_ICONV)
endif(NOT DISABLE_ICONV)

if(DISABLE_STD_LOCALE) 
	add_definitions(-DBOOSTER_LOCALE_NO_STD_BACKEND)
endif()

if(DISABLE_POSIX_LOCALE) 
	add_definitions(-DBOOSTER_LOCALE_NO_POSIX_BACKEND)
endif()

if(DISABLE_WINAPI_LOCALE) 
	add_definitions(-DBOOSTER_LOCALE_NO_WINAPI_BACKEND)
endif()


#############################################################################
#
# Setup various build flags for different supported compilers and systems
#
#############################################################################

if(CMAKE_COMPILER_IS_GNUCXX)
	check_cxx_source_compiles(
		"#if __GNUC__ < 4
		#error
		#endif
		int main() {}"
		GCC_IS_GCC4)

	set(CXX_FLAGS "-Wall -Wextra")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -pthreads")
	endif()

	if(NOT GCC_IS_GCC4)
		# Uninitalized checks are bogous under gcc-3.4
		set(CXX_FLAGS "${CXX_FLAGS} -Wno-uninitialized")
	endif()

	if(IS_WINDOWS)
		if(GCC_IS_GCC4) 
			# Very important, otherwise process would not start under cygwin
			set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--enable-auto-import")
		else()
			# gcc-3 does not have shared library for libstdc++ -- cause dll faitures with locale
			set(DISABLE_SHARED ON)
		endif()
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	set(CXX_FLAGS "-Wall")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CXX_FLAGS "-Wall -Wextra")
elseif(MSVC)
	set(CXX_FLAGS "/EHsc /W3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
	#
	# We use STL port under Sun Studio, standard library is broken
	#
	
	set(CXX_FLAGS "-library=stlport4 -xannotate=no")

	if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(CXX_FLAGS "${CXX_FLAGS} -mt")
	endif()
endif()

if(NOT IS_WINDOWS)
	check_function_exists(dlopen BOOSTER_HAVE_DLOPEN)
	if(NOT BOOSTER_HAVE_DLOPEN)
		find_library(BOOSTER_LIB_DL dl)
	endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

#############################################################################
#
# Set default RelWithDebInfo build
#
#############################################################################

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	if(MSVC)
		set(ICU_SUFFIX "d")
		set(PCRE_SUFFIX "d")
		if(NOT BOOSTER_SUFFIX)
			set(BOOSTER_SUFFIX "-d")
		else()
			set(BOOSTER_SUFFIX "${BOOSTER_SUFFIX}d")
		endif()
	endif()
endif()

if(IS_WINDOWS)
	find_library(WS2_32 ws2_32)
else()
	check_function_exists(socket HAVE_SOCKET)
	if(NOT HAVE_SOCKET)
		check_library_exists(socket socket "" HAVE_LIB_SOCKET)
		if(NOT HAVE_LIB_SOCKET)
			message(FATAL " No library with socket found")
		else(NOT HAVE_LIB_SOCKET)
			find_library(LIB_SOCKET socket)
		endif(NOT HAVE_LIB_SOCKET)
	endif(NOT HAVE_SOCKET)

	check_function_exists(gethostbyname HAVE_GETHOSTBYNAME)
	if(NOT HAVE_GETHOSTBYNAME)
		check_library_exists(socket gethostbyname "" LIB_SOCKGETHOSTBYNAME)
		if(NOT LIB_SOCKGETHOSTBYNAME)
			check_library_exists(nsl gethostbyname "" HAVE_LIB_NSL)
			if(NOT HAVE_LIB_NSL)
				message(FATAL " No library with gethostbyname found")
			else(NOT HAVE_LIB_NSL)
				find_library(LIB_NSL nsl)
			endif(NOT HAVE_LIB_NSL)
		endif(NOT LIB_SOCKGETHOSTBYNAME)
	endif(NOT HAVE_GETHOSTBYNAME)
endif()



check_cxx_source_compiles(
	"int main() { volatile int v=0; return __sync_bool_compare_and_swap(&v,0,1); }"
	BOOSTER_HAS_GCC_SYNC)

check_cxx_source_compiles(
	"#include <bits/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	BOOSTER_HAVE_GCC_BITS_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <ext/atomicity.h> 
	using __gnu_cxx::__exchange_and_add;
	int main(){ volatile int x=0; return __exchange_and_add(&x,1);}"
	BOOSTER_HAVE_GCC_EXT_EXCHANGE_AND_ADD)

check_cxx_source_compiles(
	"#include <sys/types.h>
	#include <machine/atomic.h>
	int main() { volatile unsigned v=0; return atomic_cmpset_int(&v,1,0); }"
	BOOSTER_HAVE_FREEBSD_ATOMIC)

check_cxx_source_compiles(
	"#include <execinfo.h>
	int main() { backtrace(0,0); }"
	BOOSTER_HAVE_EXECINFO)

check_cxx_source_compiles(
	"#include <stdexcept>
	 #include <iostream>
	 #include <stdlib.h>
	 #include <string.h>
	 #include <sstream>
	 #include <typeinfo>
	 #include <new>
	  extern \"C\" {
	   extern void* _Unwind_GetIP (void *);
	   extern int _Unwind_Backtrace(int (*)(void *,void *),void *);
	 }

	int main() { _Unwind_Backtrace(0,0); _Unwind_GetIP(0); }"
	BOOSTER_HAVE_UNWIND_BACKTRACE)


Check_cxx_source_compiles(
	"#include <atomic.h>
	int main() { volatile unsigned v=0; return atomic_add_int_nv(&v,1); }"
	BOOSTER_HAVE_SOLARIS_ATOMIC)

check_cxx_source_compiles(
	"#include <libkern/OSAtomic.h>
	int main() { int32_t v=0; return OSAtomicAdd32(1,&v); }"
	BOOSTER_HAVE_MAC_OS_X_ATOMIC)

check_cxx_source_compiles(
	"#include <stdint.h>
	int main() { int64_t x=10; return 0; }"
	BOOSTER_HAVE_STDINT_H)

check_cxx_source_compiles(
	"#include <inttypes.h>
	int main() { int64_t x=10; return 0; }"
	BOOSTER_HAVE_INTTYPES_H)

if(IS_WINDOWS)
	set(CMAKE_REQUIRED_LIBRARIES ${WS2_32})
	check_cxx_source_compiles(
	"#include <winsock2.h>
	#include <windows.h>
	int main()
	{ struct sockaddr_in6 in6; struct in6_addr in6addr; ::inet_pton(AF_INET6,\"::1\",&in6addr); return 0; }"
	BOOSTER_AIO_HAVE_PF_INET6)
else()
	set(CMAKE_REQUIRED_LIBRARIES ${LIB_SOCKGETHOSTBYNAME} ${LIB_SOCKET} ${LIB_NSL})
	check_cxx_source_compiles(
	"#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	int main()
	{ struct sockaddr_in6 in6; struct in6_addr in6addr; ::inet_pton(AF_INET6,\"::1\",&in6addr); return 0; }"
	BOOSTER_AIO_HAVE_PF_INET6)
endif()



set(BOOSTER_SRC	
	lib/ptime/src/posix_time.cpp
	lib/ptime/src/ctime.cpp
	lib/regex/src/pcre_regex.cpp
	lib/system/src/posix_error.cpp
	lib/system/src/windows_error.cpp
	lib/aio/src/aio_category.cpp
	lib/aio/src/deadline_timer.cpp
	lib/aio/src/endpoint.cpp
	lib/aio/src/io_service.cpp
	lib/aio/src/reactor.cpp
	lib/aio/src/select_iterrupter.cpp
	lib/aio/src/basic_io_device.cpp
	lib/aio/src/basic_socket.cpp
	lib/aio/src/acceptor.cpp
	lib/aio/src/stream_socket.cpp
	lib/smart_ptr/src/sp_counted_base.cpp
	lib/smart_ptr/src/atomic_counter.cpp
	lib/shared_object/src/shared_object.cpp
	lib/log/src/log.cpp
	lib/iostreams/src/streambuf.cpp
	lib/nowide/src/convert.cpp
	lib/backtrace/src/backtrace.cpp
	lib/locale/src/encoding/codepage.cpp
	lib/locale/src/shared/date_time.cpp
	lib/locale/src/shared/format.cpp
	lib/locale/src/shared/formatting.cpp
	lib/locale/src/shared/generator.cpp
	lib/locale/src/shared/ids.cpp
	lib/locale/src/shared/localization_backend.cpp
	lib/locale/src/shared/message.cpp
	lib/locale/src/shared/mo_lambda.cpp
	lib/locale/src/util/codecvt_converter.cpp
	lib/locale/src/util/default_locale.cpp
	lib/locale/src/util/info.cpp
	lib/locale/src/util/locale_data.cpp
)

if(NOT DISABLE_ICU_LOCALE)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/locale/src/icu/boundary.cpp
		lib/locale/src/icu/codecvt.cpp
		lib/locale/src/icu/collator.cpp
		lib/locale/src/icu/conversion.cpp
		lib/locale/src/icu/date_time.cpp
		lib/locale/src/icu/formatter.cpp
		lib/locale/src/icu/icu_backend.cpp
		lib/locale/src/icu/numeric.cpp
		lib/locale/src/icu/time_zone.cpp
		)
endif()


if(NOT DISABLE_POSIX_LOCALE)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/locale/src/posix/codecvt.cpp
		lib/locale/src/posix/collate.cpp
		lib/locale/src/posix/converter.cpp
		lib/locale/src/posix/numeric.cpp
		lib/locale/src/posix/posix_backend.cpp
		)
endif()

if(NOT DISABLE_WINAPI_LOCALE)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/locale/src/win32/collate.cpp
		lib/locale/src/win32/converter.cpp
		lib/locale/src/win32/lcid.cpp
		lib/locale/src/win32/numeric.cpp
		lib/locale/src/win32/win_backend.cpp
		)
endif()

if(NOT DISABLE_STD_LOCALE)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/locale/src/std/codecvt.cpp
		lib/locale/src/std/collate.cpp
		lib/locale/src/std/converter.cpp
		lib/locale/src/std/numeric.cpp
		lib/locale/src/std/std_backend.cpp
	)
endif()

if(NOT DISABLE_WINAPI_LOCALE OR NOT DISABLE_STD_LOCALE OR NOT DISABLE_POSIX_LOCALE)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/locale/src/util/gregorian.cpp)
endif()


if(USE_WINDOWS6_API)
	check_cxx_source_compiles(
		"
		#ifndef _WIN32_WINNT 
		#define _WIN32_WINNT 0x600
		#endif
		#include <windows.h>
		int main(){ SRWLOCK l; InitializeSRWLock(&l); }"
		HAVE_WINDOWS6_API
		)
endif()

if(USE_WINDOWS6_API AND HAVE_WINDOWS6_API)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/thread/src/thread_winapi.cpp
		lib/thread/src/thread_win6.cpp)
elseif(WINDOWS_NATIVE AND NOT USE_PTHREAD)
	set(BOOSTER_SRC ${BOOSTER_SRC}
		lib/thread/src/thread_winapi.cpp
		lib/thread/src/thread_win5.cpp)
else()
	find_path(PTHREAD_INC pthread.h)
	include_directories(${PTHREAD_INC})
	if(MSVC)
		find_library(LIB_PTHREAD pthreadVC2)
	else()
		find_library(LIB_PTHREAD NAMES pthread thr kse pthreadGC2)
	endif()

	set(BOOSTER_SRC ${BOOSTER_SRC} lib/thread/src/pthread.cpp)
endif()



find_path(PCRE_INCLUDE pcre.h)
if(PCRE_INCLUDE)
	include_directories(${PCRE_INCLUDE})
endif()

if(NOT PCRE_INCLUDE)
	find_path(PCRE_INCLUDE2 pcre/pcre.h)
	include_directories(${PCRE_INCLUDE2}/pcre)
	if(NOT PCRE_INCLUDE2)
		message(FATAL_ERROR "Can't find PCRE include directory")
	endif()

endif()

find_library(PCRE_LIB NAMES pcre${PCRE_SUFFIX} pcre)
if(NOT PCRE_LIB)
	message(FATAL_ERROR "Can't find PCRE library")
endif()



set(LINK_LIBS )

if(NOT DISABLE_SHARED)
	add_library(booster SHARED ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster)
	if(IS_WINDOWS)
		set(BOOSTER_SHARED_DEFS ${BOOSTER_SHARED_DEFS} DLL_EXPORT)
	endif()
	set_target_properties(booster PROPERTIES COMPILE_DEFINITIONS "${BOOSTER_SHARED_DEFS}")
	set_target_properties(booster PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster PROPERTIES OUTPUT_NAME "booster${BOOSTER_SUFFIX}")

endif(NOT DISABLE_SHARED)	

if(NOT DISABLE_STATIC)
	add_library(booster-static STATIC ${BOOSTER_SRC})
	set(LINK_LIBS ${LINK_LIBS} booster-static)
	set_target_properties(booster-static PROPERTIES COMPILE_DEFINITIONS "${BOOST_STATIC_DEFS}")
	set_target_properties(booster-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(booster-static PROPERTIES OUTPUT_NAME "booster${BOOSTER_SUFFIX}")
endif(NOT DISABLE_STATIC)

install(DIRECTORY booster DESTINATION include
        PATTERN ".svn" EXCLUDE)


foreach(ALIB ${LINK_LIBS})
	if(LIB_SOCKGETHOSTBYNAME)
		target_link_libraries(${ALIB} ${LIB_SOCKGETHOSTBYNAME})
	endif(LIB_SOCKGETHOSTBYNAME)
	if(LIB_NSL)
		target_link_libraries(${ALIB} ${LIB_NSL})
	endif(LIB_NSL)

	if(LIB_SOCKET)
		target_link_libraries(${ALIB} ${LIB_SOCKET})
	endif(LIB_SOCKET)
	if(LIB_PTHREAD)
		target_link_libraries(${ALIB} ${LIB_PTHREAD})
	endif(LIB_PTHREAD)
	if(WS2_32)
		target_link_libraries(${ALIB} ${WS2_32})
	endif()
	target_link_libraries(${ALIB} ${PCRE_LIB})

	if(NOT DISABLE_ICU_LOCALE)
		target_link_libraries(${ALIB} ${ICU_UC})
		target_link_libraries(${ALIB} ${ICU_I18N})
		target_link_libraries(${ALIB} ${ICU_DATA})
	endif()

	if(BOOSTER_LIB_DL)
		target_link_libraries(${ALIB} ${BOOSTER_LIB_DL})
	endif()

	if(ICONV_LIB)
		target_link_libraries(${ALIB} ${ICONV_LIB})
	endif()

  	if(MSVC)
		target_link_libraries(${ALIB} dbghelp)
	endif()

  	if(IS_WINDOWS)
		target_link_libraries(${ALIB} psapi)
	endif()
	
	if(STLPORT_LIB)
		target_link_libraries(${ALIB} ${STLPORT_LIB} supc++)
	endif()


    
	if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.6)
		set_target_properties(${ALIB} PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/booster/build_config.h")
	else()
		set_target_properties(${ALIB} PROPERTIES PUBLIC_HEADER "${CMAKE_BINARY_DIR}/booster/build_config.h")
	endif()
	
	if(IS_WINDOWS)
		set_target_properties(${ALIB} PROPERTIES VERSION ${BOOSTER_SOVERSION} SOVERSION ${BOOSTER_SOVERSION})
	else()
		set_target_properties(${ALIB} PROPERTIES VERSION ${BOOSTER_VERSION} SOVERSION ${BOOSTER_SOVERSION})
	endif()



endforeach()


if(MSVC AND NOT DISABLE_STATIC)
	set_target_properties(booster-static PROPERTIES PREFIX "lib")
endif(MSVC AND NOT DISABLE_STATIC)

set(EXE_COM_DEFS "")

if(NOT DISABLE_SHARED)
	set(BOOSTER_LIB booster)
	if(IS_WINDOWS)
		set(EXE_COM_DEFS "${EXE_COM_DEFS}" "DLL_EXPORT")
	endif()
else()
	set(BOOSTER_LIB booster-static)
endif()

macro(add_booster_param_test MODULE TEST PARAMETER)
	set(TEST_SRC "lib/${MODULE}/test/test_${TEST}.cpp")
	set(TEST_NAME "test_${MODULE}_${TEST}")
	add_executable(${TEST_NAME} ${TEST_SRC})
	target_link_libraries(${TEST_NAME} ${BOOSTER_LIB})
	set_target_properties(${TEST_NAME} PROPERTIES COMPILE_DEFINITIONS "${EXE_COM_DEFS}")
	add_test(${TEST_NAME} ${TEST_NAME} ${PARAMETER})
	set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endmacro()

macro(add_booster_test MODULE TEST)
	add_booster_param_test(${MODULE} ${TEST} "")
endmacro()

add_booster_test(function function)
add_booster_test(function callback)
add_booster_test(ptime posix_time)
add_booster_test(thread thread)
if(UNIX)
	add_booster_test(thread fork)
endif()

add_booster_test(smart_ptr shared_ptr)
add_booster_test(smart_ptr atomic_counter)
add_booster_test(smart_ptr sp_counter)

add_booster_test(log log)
add_booster_test(nowide nowide)
add_booster_test(iostreams streambuf)

add_booster_test(regex regex)
add_booster_test(aio reactor)
add_booster_test(aio timer)
add_booster_test(aio event_loop)
add_booster_test(aio socket)
add_booster_test(aio endpoint)
add_booster_test(backtrace backtrace)
if(NOT IS_WINDOWS)
	add_booster_test(aio prefork)
endif()

add_booster_test(locale codepage)
add_booster_param_test(locale message "${CMAKE_CURRENT_SOURCE_DIR}/lib/locale/test")
add_booster_test(locale ios_prop)
add_booster_test(locale codepage_converter)
add_booster_test(locale date_time)
add_booster_test(locale generator)
add_booster_test(locale config)
add_booster_test(locale utf)

if(NOT DISABLE_ICU_LOCALE)
	add_booster_test(locale collate)
	add_booster_test(locale convert)
	add_booster_test(locale boundary)
	add_booster_test(locale formatting)
	add_booster_test(locale icu_vs_os_timezone)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set_target_properties(test_locale_formatting PROPERTIES COMPILE_FLAGS "-O0")
	endif()
endif()

if(NOT DISABLE_STD_LOCALE)
	add_booster_test(locale std_convert)
	add_booster_test(locale std_formatting)
	add_booster_test(locale std_collate)
endif()

if(NOT DISABLE_POSIX_LOCALE)
	add_booster_test(locale posix_collate)
	add_booster_test(locale posix_convert)
	add_booster_test(locale posix_formatting)
endif()


if(NOT DISABLE_WINAPI_LOCALE)
	add_booster_test(locale winapi_formatting)
	add_booster_test(locale winapi_collate)
	add_booster_test(locale winapi_convert)
endif()

set(BOOSTER_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
set(BOOSTER_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})

configure_file(lib/booster_build_config.cmake.h booster/build_config.h)

install(TARGETS ${LINK_LIBS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIBDIR}
	ARCHIVE DESTINATION ${LIBDIR}
	PUBLIC_HEADER DESTINATION include/booster)


