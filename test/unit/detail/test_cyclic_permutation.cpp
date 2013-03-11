/**
 * This file contains unit tests for the `is_cyclic_permutation` algorithm.
 */

#include <d2/detail/cyclic_permutation.hpp>

#include <boost/assign.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include <vector>


namespace {
template <typename Container>
struct cyclic_permutation_test
    : testing::Test
{ };

TYPED_TEST_CASE_P(cyclic_permutation_test);


using boost::assign::list_of;
using d2::detail::is_cyclic_permutation;

TYPED_TEST_P(cyclic_permutation_test, diff_size_ranges_are_not_cyclic_perms) {
    TypeParam s1 = list_of('a')('b')('c')('d'), s2 = list_of('a');
    ASSERT_FALSE(is_cyclic_permutation(s1, s2));
    ASSERT_FALSE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, behaves_well_on_half_empty_input) {
    TypeParam s1 = list_of('a')('b')('c')('d'), s2;
    ASSERT_FALSE(is_cyclic_permutation(s1, s2));
    ASSERT_FALSE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, behaves_well_on_completely_empty_input) {
    TypeParam s1, s2;
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, catches_left_shifted_by_one) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('b')('c')('d')('e')('f')('a');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, catches_right_shifted_by_one) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('f')('a')('b')('c')('d')('e');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, catches_when_equal_input) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('a')('b')('c')('d')('e')('f');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, behaves_well_with_duplicate_values) {
    TypeParam s1 = list_of('a')('b')('a')('b')('a')('b')('c')('d'),
              s2 = list_of('b')('a')('b')('a')('b')('c')('d')('a');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, behaves_well_with_shifts_larger_than_one) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('e')('f')('a')('b')('c')('d');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, unrelated_strings_are_not_cyclic_perms) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('e')('f')('g')('h')('i')('j');
    ASSERT_FALSE(is_cyclic_permutation(s1, s2));
    ASSERT_FALSE(is_cyclic_permutation(s2, s1));
}

TYPED_TEST_P(cyclic_permutation_test, check_with_iterators) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('e')('f')('a')('b')('c')('d');
    ASSERT_TRUE(is_cyclic_permutation(s1.begin(), s1.end(),
                                      s2.begin(), s2.end()));
    ASSERT_TRUE(is_cyclic_permutation(s2.begin(), s2.end(),
                                      s1.begin(), s1.end()));
}

TYPED_TEST_P(cyclic_permutation_test, use_custom_predicate) {
    TypeParam s1 = list_of('a')('b')('c')('d')('e')('f'),
              s2 = list_of('e')('f')('a')('b')('c')('d');
    ASSERT_TRUE(is_cyclic_permutation(s1, s2, std::equal_to<char>()));
    ASSERT_TRUE(is_cyclic_permutation(s2, s1, std::equal_to<char>()));
}

REGISTER_TYPED_TEST_CASE_P(
    cyclic_permutation_test,
        diff_size_ranges_are_not_cyclic_perms,
        behaves_well_on_half_empty_input,
        behaves_well_on_completely_empty_input,
        catches_left_shifted_by_one,
        catches_right_shifted_by_one,
        catches_when_equal_input,
        behaves_well_with_duplicate_values,
        behaves_well_with_shifts_larger_than_one,
        unrelated_strings_are_not_cyclic_perms,
        check_with_iterators,
        use_custom_predicate
);

INSTANTIATE_TYPED_TEST_CASE_P(
    with_std_list,
    cyclic_permutation_test,
    std::list<char>
);

INSTANTIATE_TYPED_TEST_CASE_P(
    with_std_string,
    cyclic_permutation_test,
    std::string
);

INSTANTIATE_TYPED_TEST_CASE_P(
    with_std_vector,
    cyclic_permutation_test,
    std::vector<char>
);
} // end anonymous namespace
