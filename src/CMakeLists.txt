add_executable (unindent unindent.cpp normalizer.cpp)

target_link_libraries (unindent PRIVATE project_options project_warnings)
target_link_libraries (unindent PRIVATE fmt)


add_executable (validate validate.cpp normalizer.cpp sam2_parser.cpp)

target_link_libraries (validate PRIVATE project_options project_warnings)
target_link_libraries (validate PRIVATE fmt)
target_link_libraries (validate PRIVATE taocpp::pegtl)
