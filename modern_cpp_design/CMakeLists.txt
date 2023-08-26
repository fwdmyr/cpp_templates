macro(package_add_executable EXECNAME FILES)
    add_executable(${EXECNAME} ${FILES}) 
    target_link_libraries(${EXECNAME} ${ARGN})
    target_compile_features(${EXECNAME} PRIVATE cxx_std_17)
endmacro()

