/**
 * This file contains unit tests for the bounded input/output sequences.
 */

#include <d2/detail/bounded_io_sequence.hpp>

#include <boost/assign.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>


using namespace d2;
using namespace d2::detail;
using namespace boost::assign;

TEST(bounded_io_sequence, test_should_load_and_save_strings_correctly) {
    std::string initial("abcdef");
    std::stringstream result;

    result << make_bounded_output_sequence(initial);
    ASSERT_EQ("6:abcdef", result.str());

    result.str("3:xyz");
    initial.clear();
    result >> make_bounded_input_sequence(initial);
    ASSERT_EQ("xyz", initial);
}

TEST(bounded_io_sequence, test_should_save_and_load_nested_bounded_sequences){
    std::vector<bounded_for_io<std::string> > data = list_of<std::string>
        ("foo")("bar")("baz")
    ;
    std::vector<std::string> initial(boost::begin(data), boost::end(data));
    std::stringstream stream;

    stream << make_bounded_output_sequence(data);
    data.clear();
    stream >> make_bounded_input_sequence(data);

    std::vector<std::string> passed_through_stream(boost::begin(data),
                                                   boost::end(data));
    ASSERT_TRUE(initial == passed_through_stream);
}
