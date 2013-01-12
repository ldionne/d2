/**
 * This file contains a parametrizable unit test for the serialization
 * of objects through standard streams.
 */

#ifndef D2_TEST_SERIALIZATION_TEST_HPP
#define D2_TEST_SERIALIZATION_TEST_HPP

#include <algorithm>
#include <boost/spirit/include/karma.hpp>
#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include <vector>


namespace std {
    // We need that for ASSERT_EQ(vector, vector)
    template <typename T, typename A>
    ostream& operator<<(ostream& os, vector<T, A> const& vec) {
        os << boost::spirit::karma::format(
            '(' << boost::spirit::karma::stream % ", " << ')'
        , vec);
        return os;
    }
} // end namespace std

namespace d2 {
namespace test {

template <typename Policy>
struct SerializationTest : ::testing::Test {

};

TYPED_TEST_CASE_P(SerializationTest);

TYPED_TEST_P(SerializationTest, save_and_load_a_single_object) {
    std::stringstream stream;
    typename TypeParam::value_type saved(TypeParam::get_random_object());
    stream << saved;
    EXPECT_TRUE(stream) << "failed to save the object";

    typename TypeParam::value_type loaded;
    stream >> loaded;
    EXPECT_TRUE(stream) << "failed to load the object";

    ASSERT_EQ(saved, loaded) << "saving and loading seems to have succeeded, "
                                "but both objects are not the same";
}

TYPED_TEST_P(SerializationTest, save_and_load_several_objects) {
    static unsigned int const NUM_OBJECTS = 100;
    std::stringstream stream;
    std::vector<typename TypeParam::value_type> saved;
    std::generate_n(std::back_inserter(saved), NUM_OBJECTS,
                    &TypeParam::get_random_object);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<typename TypeParam::value_type>(stream));
    EXPECT_TRUE(stream) << "failed to save the objects";

    std::vector<typename TypeParam::value_type> loaded;
    std::copy(std::istream_iterator<typename TypeParam::value_type>(stream),
              std::istream_iterator<typename TypeParam::value_type>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream.eof()) << "failed to load the objects";

    ASSERT_EQ(saved, loaded);
}

REGISTER_TYPED_TEST_CASE_P(
    SerializationTest,
        save_and_load_a_single_object,
        save_and_load_several_objects);

} // end namespace test
} // end namespace d2

#endif // !D2_TEST_SERIALIZATION_TEST_HPP
