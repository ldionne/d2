
include_directories(../d2mock/include)

add_custom_target(scenario)
function(add_scenario name)
    add_executable(scenario_${name} ${name}.cpp)
    target_link_libraries(scenario_${name} d2mock)
    add_dependencies(scenario scenario_${name})
    add_test(NAME ${name} COMMAND scenario_${name})
    set_target_properties(scenario_${name} PROPERTIES
                COMPILE_FLAGS -std=c++11
                COMPILE_DEFINITIONS D2MOCK_TRACKABLE_SYNC_OBJECT_ACCESS)
endfunction()

file(GLOB all_scenarios_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
foreach(scenario_file ${all_scenarios_files})
    string(REPLACE ".cpp" "" scenario_name ${scenario_file})
    add_scenario(${scenario_name})
endforeach()

add_executable(scenario_c_api test_c_api.c)
target_link_libraries(scenario_c_api d2mock)
add_dependencies(scenario scenario_c_api)
add_test(NAME c_api COMMAND scenario_c_api)
