macro(package_add_executable EXECNAME FILES)
    add_executable(${EXECNAME} ${FILES}) 
    target_link_libraries(${EXECNAME} ${ARGN})
    target_compile_features(${EXECNAME} PRIVATE cxx_std_17)
endmacro()

package_add_executable(chapter4_variadic_templates chapter4_variadic_templates.cpp)
package_add_executable(chapter5_tricky_basics chapter5_tricky_basics.cpp)
package_add_executable(chapter6_enable_if chapter6_enable_if.cpp)
package_add_executable(chapter8_compile_time_programming chapter8_compile_time_programming.cpp)
package_add_executable(chapter11_generic_libraries chapter11_generic_libraries.cpp)
package_add_executable(chapter19_implementing_traits chapter19_implementing_traits.cpp)
package_add_executable(chapter20_overloading_on_type_properties chapter20_overloading_on_type_properties.cpp)
package_add_executable(chapter21_templates_and_inheritance chapter21_templates_and_inheritance.cpp)
package_add_executable(chapter22_bridging_static_and_dynamic_polymorphism chapter22_bridging_static_and_dynamic_polymorphism.cpp)
package_add_executable(chapter23_metaprogramming chapter23_metaprogramming.cpp)
package_add_executable(chapter24_typelists chapter24_typelists.cpp)
