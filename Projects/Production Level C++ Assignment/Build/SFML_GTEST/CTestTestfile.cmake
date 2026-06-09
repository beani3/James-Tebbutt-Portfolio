# CMake generated Testfile for 
# Source directory: C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST
# Build directory: C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/build/SFML_GTEST
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[RunUnitTests]=] "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/build/bin/Debug/RunUnitTests.exe")
  set_tests_properties([=[RunUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;100;add_test;C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[RunUnitTests]=] "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/build/bin/Release/RunUnitTests.exe")
  set_tests_properties([=[RunUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;100;add_test;C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[RunUnitTests]=] "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/build/bin/MinSizeRel/RunUnitTests.exe")
  set_tests_properties([=[RunUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;100;add_test;C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[RunUnitTests]=] "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/build/bin/RelWithDebInfo/RunUnitTests.exe")
  set_tests_properties([=[RunUnitTests]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;100;add_test;C:/Users/james/OneDrive/Desktop/Production Level C++ Assignment/SFML_GTEST/CMakeLists.txt;0;")
else()
  add_test([=[RunUnitTests]=] NOT_AVAILABLE)
endif()
subdirs("../_deps/googletest-build")
subdirs("../_deps/sfml-build")
