/**
 * This file contains unit tests for the `unordered_difference` algorithm.
 */

#include <d2/detail/unordered_difference.hpp>

#include <boost/assign.hpp>
#include <gtest/gtest.h>
#include <iterator>
#include <set>
#include <string>
#include <vector>


namespace {
struct empty_difference_tag { };

template <typename Container>
struct unordered_difference_test : testing::Test {
    template <typename Difference>
    void validate_difference(Container const& a, Container const& b,
                             Difference const& expected_) {
        std::multiset<char> actual, expected(expected_.begin(),
                                             expected_.end());
        d2::detail::unordered_difference(a, b,
                                    std::inserter(actual, actual.end()));
        ASSERT_TRUE(expected == actual);
    }

    void validate_difference(Container const& a, Container const& b,
                             empty_difference_tag) {
        std::vector<char> actual;
        d2::detail::unordered_difference(a, b, std::back_inserter(actual));
        ASSERT_TRUE(actual.empty());
    }
};

TYPED_TEST_CASE_P(unordered_difference_test);


using boost::assign::list_of;

TYPED_TEST_P(unordered_difference_test, both_empty_ranges_yield_empty_diff) {
    TypeParam s1, s2;
    this->validate_difference(s1, s2, empty_difference_tag());
}

TYPED_TEST_P(unordered_difference_test, first_empty_range_yields_empty_diff) {
    TypeParam s1, s2 = list_of('a')('b')('c');
    this->validate_difference(s1, s2, empty_difference_tag());
}

TYPED_TEST_P(unordered_difference_test, second_empty_range_yields_first_range) {
    TypeParam s1 = list_of('a')('b')('c'), s2;
    this->validate_difference(s1, s2, s1);
}

TYPED_TEST_P(unordered_difference_test,
                            duplicates_in_first_range_are_copied_to_output) {
    TypeParam s1 = list_of('a')('b')('a')('c')('a'), s2 = list_of('a')('c');
    this->validate_difference(s1, s2, list_of('b')('a')('a'));
}


REGISTER_TYPED_TEST_CASE_P(
    unordered_difference_test,
        both_empty_ranges_yield_empty_diff,
        first_empty_range_yields_empty_diff,
        second_empty_range_yields_first_range,
        duplicates_in_first_range_are_copied_to_output
);

INSTANTIATE_TYPED_TEST_CASE_P(
    with_std_string,
    unordered_difference_test,
    std::string
);

INSTANTIATE_TYPED_TEST_CASE_P(
    with_std_vector,
    unordered_difference_test,
    std::vector<char>
);
} // end anonymous namespace
