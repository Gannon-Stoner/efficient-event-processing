# CMake generated Testfile for 
# Source directory: /Users/gannonstoner/efficient-event-processing
# Build directory: /Users/gannonstoner/efficient-event-processing/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[all_unit_tests]=] "/Users/gannonstoner/efficient-event-processing/cmake-build-debug/unit_tests")
set_tests_properties([=[all_unit_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/gannonstoner/efficient-event-processing/CMakeLists.txt;61;add_test;/Users/gannonstoner/efficient-event-processing/CMakeLists.txt;0;")
subdirs("_deps/catch2-build")
subdirs("_deps/benchmark-build")
