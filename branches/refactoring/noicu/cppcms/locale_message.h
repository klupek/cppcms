//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_MESSAGE_H_INCLUDED
#define CPPCMS_LOCALE_MESSAGE_H_INCLUDED

#include <cppcms/config.h>
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <locale>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include "locale_formatting.h"



namespace cppcms {
    namespace locale {
        ///
        /// \defgroup message Message Formatting (translation) 
        ///
        ///This module provides message translation functionality, i.e. allow your application to speak native language
        ///
        /// @{
        /// 

        class info;

        ///
        /// \cond INTERNAL
        ///
        template<typename CharType>
        struct base_message_format: public std::locale::facet
        {
        };
       
       
        template<typename CharType>
        class message_format : public base_message_format<CharType>
        {
        public:

            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            message_format(size_t refs = 0) : 
                base_message_format<CharType>(refs)
            {
            }

            virtual char_type const *get(int domain_id,char const *context,char const *id) const = 0;
            virtual char_type const *get(int domain_id,char const *context,char const *single_id,int n) const = 0;
            virtual int domain(std::string const &domain) const = 0;

#if defined (__SUNPRO_CC) && defined (_RWSTD_VER)
            std::locale::id& __get_id (void) const { return id; }
#endif
            static message_format<CharType> *create(info const &,std::vector<std::string> const &paths,std::vector<std::string> const &domains);

        protected:
            virtual ~message_format()
            {
            }

        };

        /// \endcond

        ///
        /// \brief This class represents a message that can be converted to specific locale message
        ///
        /// It holds original ASCII string that is queried in the dictionary when converting to output string.
        /// The created string may be UTF-8, UTF-16, UTF-32 or other 8-bit encoded string according to target 
        /// character type and locale encoding.
        ///
        struct message {
        public:

            ///
            /// Create default empty message
            /// 
            message() :
                n_(0),
                c_id_(""),
                c_context_(""),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple message from 0 terminated string. The string should exist
            /// until message is destroyed. Generally useful with static constant strings
            /// 
            explicit message(char const *id) :
                n_(0),
                c_id_(id),
                c_context_(""),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple plural form message from 0 terminated strings. The strings should exist
            /// until message is destroyed. Generally useful with static constant strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(char const *single,char const *plural,int n) :
                n_(n),
                c_id_(single),
                c_context_(""),
                c_plural_(plural)
            {
            }

            ///
            /// Create a simple message from 0 terminated strings, with context
            /// information. The string should exist
            /// until message is destroyed. Generally useful with static constant strings
            /// 
            explicit message(char const *context,char const *id) :
                n_(0),
                c_id_(id),
                c_context_(context),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple plural form message from 0 terminated strings, with context. The strings should exist
            /// until message is destroyed. Generally useful with static constant strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(char const *context,char const *single,char const *plural,int n) :
                n_(n),
                c_id_(single),
                c_context_(context),
                c_plural_(plural)
            {
            }


            ///
            /// Create a simple message from string.
            ///
            explicit message(std::string const &id) :
                n_(0),
                c_id_(0),
                c_context_(0),
                c_plural_(0),
                id_(id)
            {
            }

            ///
            /// Create a simple plural form message from strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(std::string const &single,std::string const &plural,int number) :
                n_(number),
                c_id_(0),
                c_context_(0),
                c_plural_(0),
                id_(single),
                plural_(plural)
            {
            }

            ///
            /// Create a simple message from string with context.
            ///
            explicit message(std::string const &context,std::string const &id) :
                n_(0),
                c_id_(0),
                c_context_(0),
                c_plural_(0),
                id_(id),
                context_(context)
            {
            }

            ///
            /// Create a simple plural form message from strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(std::string const &context,std::string const &single,std::string const &plural,int number) :
                n_(number),
                c_id_(0),
                c_context_(0),
                c_plural_(0),
                id_(single),
                context_(context),
                plural_(plural)
            {
            }

            ///
            /// Message class can be explicitly converter to string class
            ///

            template<typename CharType>
            operator std::basic_string<CharType> () const
            {
                return str<CharType>();
            }

            ///
            /// Translate message to the string in default global locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str() const
            {
                std::locale loc;
                return str<CharType>(loc,0);
            }
            
            ///
            /// Translate message to string in the locale \a locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale) const
            {
                return str<CharType>(locale,0);
            }
           
            ///
            /// Translate message to string using locale \a locale and message domain  \ a domain_id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale,std::string domain_id) const
            {
                int id=0;
                if(std::has_facet<message_format<CharType> >(locale))
                    id=std::use_facet<message_format<CharType> >(locale).domain(domain_id);
                return str<CharType>(locale,id);
            }

            ///
            /// Translate message to string using defailt locale message domain  \ a domain_id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::string domain_id) const
            {
                int id=0;
                std::locale locale;
                if(std::has_facet<message_format<CharType> >(locale))
                    id=std::use_facet<message_format<CharType> >(locale).domain(domain_id);
                return str<CharType>(locale,id);
            }

            
            ///
            /// Translate message to string using locale \a loc and message domain index  \ a id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &loc,int id) const
            {
                std::basic_string<CharType> buffer;                
                CharType const *ptr = write(loc,id,buffer);
                if(ptr == buffer.c_str())
                    return buffer;
                else
                    buffer = ptr;
                return buffer;
            }


            ///
            /// Translate message and write to stream \a out, using imbued locale and domain set to the 
            /// stream
            ///
            template<typename CharType>
            void write(std::basic_ostream<CharType> &out) const
            {
                std::locale const &loc = out.getloc();
                int id = ext_value(out,flags::domain_id);
                std::basic_string<CharType> buffer;
                out << write(loc,id,buffer);
            }

        private:
            
            template<typename CharType>
            CharType const *write(std::locale const &loc,int domain_id,std::basic_string<CharType> &buffer) const
            {
                CharType const *translated = 0;
                static const CharType empty_string[1] = {0};

                char const *id = c_id_ ? c_id_ : id_.c_str();
                char const *context = c_context_ ? c_context_ : context_.c_str();
                char const *plural = c_plural_ ? c_plural_ : (plural_.empty() ? 0 : plural_.c_str());
                
                if(*id == 0 || context == 0)
                    return empty_string;
                
                if(std::has_facet<message_format<CharType> >(loc)) {
                    message_format<CharType> const &msg = std::use_facet<message_format<CharType> >(loc);
                    
                    if(!plural) {
                        translated = msg.get(domain_id,context,id);
                    }
                    else {
                        translated = msg.get(domain_id,context,id,n_);
                    }
                }

                if(!translated) {
                    char const *msg = plural ? ( n_ == 1 ? id : plural) : id;

                    while(*msg)
                        buffer+=static_cast<CharType>(*msg++);

                    translated = buffer.c_str();
                }
                return translated;
            }

            /// members

            int n_;
            char const *c_id_;
            char const *c_context_;
            char const *c_plural_;
            std::string id_;
            std::string context_;
            std::string plural_;
        };

        ///
        /// Translate message \a msg and write it to stream
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,message const &msg)
        {
            msg.write(out);
            return out;
        }


        ///
        /// Translate a message, \a msg is not copied 
        ///
        inline message translate(char const *msg)
        {
            return message(msg);
        }
        ///
        /// Translate a message in context, \a msg and \a context are not copied 
        ///
        inline message translate(char const *context,char const *msg)
        {
            return message(context,msg);
        }
        ///
        /// Translate a plural message form, \a single and \a plural are not copied 
        ///
        inline message translate(char const *single,char const *plural,int n)
        {
            return message(single,plural,n);
        }
        ///
        /// Translate a plural message from in constext, \a context, \a single and \a plural are not copied 
        ///
        inline message translate(char const *context,char const *single,char const *plural,int n)
        {
            return message(context,single,plural,n);
        }
        
        ///
        /// Translate a message, \a msg is copied 
        ///
        inline message translate(std::string const &msg)
        {
            return message(msg);
        }
        
        ///
        /// Translate a message in context,\a context and \a msg is copied 
        ///
        inline message translate(std::string const &context,std::string const &msg)
        {
            return message(context,msg);
        }
        ///
        /// Translate a plural message form in constext, \a context, \a single and \a plural are copied 
        ///
        inline message translate(std::string const &context,std::string const &single,std::string const &plural,int n)
        {
            return message(context,single,plural,n);
        }

        ///
        /// Translate a plural message form, \a single and \a plural are copied 
        ///

        inline message translate(std::string const &single,std::string const &plural,int n)
        {
            return message(single,plural,n);
        }

        // 
        // gettext compatibility functions
        //

        ///
        /// Translate message \a id according to locale \a loc
        ///
        inline std::string gettext(char const *id,std::locale const &loc=std::locale())
        {
            return message(id).str<char>(loc);
        }
        ///
        /// Translate plural form according to locale \a loc
        ///
        inline std::string ngettext(char const *s,char const *p,int n,std::locale const &loc=std::locale())
        {
            return message(s,p,n).str<char>(loc);
        }
        ///
        /// Translate message \a id according to locale \a loc in domain \a domain
        ///
        inline std::string dgettext(char const *domain,char const *id,std::locale const &loc=std::locale())
        {
            return message(id).str<char>(loc,domain);
        }

        ///
        /// Translate plural form according to locale \a loc in domain \a domain
        ///
        inline std::string dngettext(char const *domain,char const *s,char const *p,int n,std::locale const &loc=std::locale())
        {
            return message(s,p,n).str<char>(loc,domain);
        }
        ///
        /// Translate message \a id according to locale \a loc in context \a context
        ///
        inline std::string pgettext(char const *context,char const *id,std::locale const &loc=std::locale())
        {
            return message(context,id).str<char>(loc);
        }
        ///
        /// Translate plural form according to locale \a loc in context \a context
        ///
        inline std::string npgettext(char const *context,char const *s,char const *p,int n,std::locale const &loc=std::locale())
        {
            return message(context,s,p,n).str<char>(loc);
        }
        ///
        /// Translate message \a id according to locale \a loc in domain \a domain in context \a context
        ///
        inline std::string dpgettext(char const *domain,char const *context,char const *id,std::locale const &loc=std::locale())
        {
            return message(context,id).str<char>(loc,domain);
        }
        ///
        /// Translate plural form according to locale \a loc in domain \a domain in context \a context
        ///
        inline std::string dnpgettext(char const *domain,char const *context,char const *s,char const *p,int n,std::locale const &loc=std::locale())
        {
            return message(context,s,p,n).str<char>(loc,domain);
        }


        ///
        /// \cond INTERNAL
        ///
        
        template<>
        struct CPPCMS_API base_message_format<char> : public std::locale::facet 
        {
            base_message_format(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };
        
        template<>
        CPPCMS_API message_format<char> *message_format<char>::create(   info const &,
                                                                                std::vector<std::string> const &domains,
                                                                                std::vector<std::string> const &paths);
        /// \endcond

        ///
        /// @}
        ///

        namespace as {
            /// \cond INTERNAL
            namespace details {
                struct set_domain {
                    std::string domain_id;
                };
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out, set_domain const &dom)
                {
                    int id = std::use_facet<message_format<CharType> >(out.getloc()).domain(dom.domain_id);
                    ext_value(out,flags::domain_id,id);
                    return out;
                }
            } // details
            /// \endcond

            ///
            /// \addtogroup manipulators
            ///
            /// @{
            
            ///
            /// Manipulator for switching message domain in ostream,
            ///
            inline details::set_domain domain(std::string const &id)
            {
                details::set_domain tmp;
                tmp.domain_id = id;
                return tmp;
            }
            /// @}
        } // as
    } // locale 
} // boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

