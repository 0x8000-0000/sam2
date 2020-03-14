add_executable (unindent unindent.cpp)

target_link_libraries (unindent PRIVATE project_options project_warnings)
target_link_libraries (unindent PRIVATE fmt)
target_link_libraries (unindent PRIVATE -lstdc++fs)
