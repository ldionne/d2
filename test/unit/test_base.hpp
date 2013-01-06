/**
 * This file defines helper methods for unit tests.
 * It also includes some headers that are commonly found in the tests.
 */

#ifndef TEST_UNIT_TEST_BASE_HPP
#define TEST_UNIT_TEST_BASE_HPP

#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>


namespace karma = boost::spirit::karma;
namespace qi = boost::spirit::qi;
namespace fs = boost::filesystem;

namespace std {
    // This is UB. Whatever. We can't say ASSERT_EQ(vector, vector) otherwise.
    template <typename Ostream, typename T, typename A>
    Ostream& operator<<(Ostream& os, vector<T, A> const& v) {
        os << karma::format('(' << karma::stream % ", " << ')', v);
        return os;
    }
} // end namespace std

#endif // !TEST_UNIT_TEST_BASE_HPP
