include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCXXSourceCompiles)

# check for libz using the cmake supplied FindZLIB.cmake
find_package(ZLIB)
if(ZLIB_FOUND)
    set(HAVE_ZLIB 1)
else()
    set(HAVE_ZLIB 0)
endif()

# Determine whether or not your compiler supports move semantics.
check_cxx_source_compiles("
    #ifdef __clang__
    # pragma clang diagnostic error \"-Wc++11-extensions\" 
    #endif
    #include <utility>
    int func(int &&x) { return x - 1; }
    int main() { return func(std::move(1)); }
" SUPPORT_MOVE_SEMANTICS)

# Determine whether or not your compiler supports template alias.
check_cxx_source_compiles("
    #ifdef __clang__
    # pragma clang diagnostic error \"-Wc++11-extensions\" 
    #endif
    #include <vector>
    template <typename T> using myVector = std::vector<T>;
    int main() { return 0; }
" SUPPORT_TEMPLATE_ALIAS)

# Determine where shared_ptr<T> is defined regardless of C++11 support.
check_cxx_source_compiles("
    #include <memory>
    int main() { std::tr1::shared_ptr<int> x; return 0; }
" HAVE_STD_SHARED_PTR)

check_cxx_source_compiles("
    #include <tr1/memory>
    int main() { std::tr1::shared_ptr<int> x; return 0; }
" HAVE_TR1_SHARED_PTR)

check_cxx_source_compiles("
    #include <boost/shared_ptr.hpp>
    int main() { boost::shared_ptr<int> x; return 0; }
" HAVE_BOOST_SHARED_PTR)

# Determine whether your compiler supports codecvt header.
check_cxx_source_compiles("
  #include <codecvt>
  int main() { std::codecvt_utf8_utf16<wchar_t> x; return 0; }
" HAVE_CODECVT)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(CppUnit)
if(NOT CppUnit_FOUND AND BUILD_TESTS)
    message(STATUS "CppUnit not found, disabling tests.")
    set(BUILD_TESTS OFF)
endif()

