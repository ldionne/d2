/**
 * This file is a wrapper on top of boost's exceptions.
 */

#ifndef D2_DETAIL_EXCEPTIONS_HPP
#define D2_DETAIL_EXCEPTIONS_HPP

#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <exception>


namespace d2 {

#define D2_THROW(e) BOOST_THROW_EXCEPTION(e)

/**
 * Base class for exceptions in _d2_.
 */
struct Exception : virtual boost::exception, virtual std::exception {
    virtual char const* what() const throw() {
        return "d2::Exception";
    }
};

} // end namespace d2

#endif // !D2_DETAIL_EXCEPTIONS_HPP
