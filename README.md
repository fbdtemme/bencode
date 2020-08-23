![](docs/images/bencode.svg)

[![build](https://github.com/fbdtemme/bencode/workflows/build/badge.svg?branch=master)](https://github.com/fbdtemme/bencode/actions?query=workflow%3Abuild)
[![docs](https://github.com/fbdtemme/bencode/workflows/documentation/badge.svg?branch=master)](https://fbdtemme.github.io/bencode/)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/5cc3eec94d8a486dab62afeab5130def)](https://app.codacy.com/manual/floriandetemmerman/bencode?utm_source=github.com&utm_medium=referral&utm_content=fbdtemme/bencode&utm_campaign=Badge_Grade_Dashboard)
[![codecov](https://codecov.io/gh/fbdtemme/bencode/branch/master/graph/badge.svg)](https://codecov.io/gh/fbdtemme/bencode)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[**Features**](#Features) |
[**Status**](#Status) |
[**Documentation**](#Documentation) | 
[**Examples**](#Examples) |
[**Building**](#Building) | 
[**Integration**](#Integration) |
[**License**](#License)

A header-only C++20 bencode serialization/deserialization library.

## Features

 *  Convenient owning representation of bencoded data with `bvalue`.
 *  Fast and memory efficient read-only, non-owning representation into stable buffers of bencoded data with `bview`.
 *  Build-in serialization/deserializaton for most standard containers.
 *  Support for serializing/deserializing to/from user-defined types. 
 *  Parse directly to custom types by satisfying the `EventConsumer` concept.
 *  Throwing and non throwing variants of common functions.
 *  Iterative parsing to protect against stack overflow attacks.

## Status

This library is still under development, but should be fairly stable. 
The API may change at any release prior to 1.0.0.

## Documentation

Documentation is available on the [bencode GitHub pages](https://fbdtemme.github.io/bencode/)

## Examples

Decode a value to a `bvalue`.

```cpp
// All examples use namespace bc for brevity
namespace bc = bencode;
```

```cpp
#include <bencode/bvalue.hpp> 

using namespace bc::literals;

// create a bvalue from a string literal.
bc::bvalue b = "l3:fooi2ee"_bvalue;
// get a std::string reference to the first value in the lsit
auto& v1 = get_string(b[0]);
// get the integer value of the second value in the list
auto v2 = get_integer(b[1]);
```

Decode a value to a `bview`.

```cpp
#include <bencode/bview.hpp> 

namespace bc = bencode;

// decode the data to a descriptor_table
bc::descriptor_table t = bc::decode_view("l3:fooi2ee");
// get the bview to the root element (ie the list) 
bc::bview b = t.get_root();
// get a std::string reference to the first value in the lsit
auto& v1 = get_string(b[0]);
// get the integer value of the second value in the list
auto v2 = get_integer(b[1]);
```

Serialize to bencode using `bvalue`.
```cpp
#include <iostream>
#include <bencode/bvalue.hpp>
#include <bencode/encode.hpp>

bc::bvalue b {
  {"foo", 1},
  {"bar", 2},
  {"baz", bc::bvalue(bc::btype::list, {1, 2, 3})},
};

// prints "d3:bari2e3:bazli1ei2ei3ee3:fooi1ee";
bc::encode_to(std::cout, b);
```

Serialize to bencode using `encoder`

```cpp
#include <iostream>
#include <vector>
#include <bencode/encode.hpp>           // bc::encoder
#include <bencode/traits/vector.hpp>    // support for serializating std::vector

bc::encoder es(std::cout);

es << bc::begin_dict
   << "foo" << 1
   << "bar" << 2
   << "baz" << std::vector{1, 2, 3}
   << end_dict;

// prints "d3:bari2e3:bazli1ei2ei3ee3:fooi1ee";
```

For more examples see the [documentation](https://fbdtemme.github.io/bencode/) 

## Building

This project requires C++20. 
Currently only GCC 10 is supported.

This library depends on following projects:
*  [Catch2](https://github.com/catchorg/Catch2)
*  [fmt](https://github.com/fmtlib/fmt)
*  [Microsoft GSL](https://github.com/microsoft/GSL)
*  [expected-lite](https://github.com/martinmoene/expected-lite)

All dependencies can be fetched from github during configure time or can be installed manually.

The tests can be built as every other project which makes use of the CMake build system.

```{bash}
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make 
```

## Integration

You can use the `bencode::bencode` interface target in CMake.
The library can be located with `find_package`.

```cmake
# CMakeLists.txt
find_package(bencode REQUIRED)
...
add_library(foo ...)
...
target_link_libraries(foo INTERFACE bencode::bencode)
```

The source tree can be included in your project and added to your build with `add_subdirectory`.

```cmake
# CMakeLists.txt
# Disable building tests and benchmarks.
set(BENCODE_BUILD_TESTS OFF)
set(BENCODE_BUILD_BENCHMARKS OFF)

add_subdirectory(bencode)
...
add_library(foo ...)
...
target_link_libraries(foo INTERFACE bencode::bencode)
```

You can also use `FetchContent` to download the code from github.
    
```cmake
# CMakeLists.txt
include(FetchContent)

FetchContent_Declare(bencode
  GIT_REPOSITORY https://github.com/fbdtemme/bencode.git
  GIT_TAG "master")

FetchContent_MakeAvailable(bencode)
...
add_library(foo ...)
...
target_link_libraries(foo INTERFACE bencode::bencode)
```

## License

Distributed under the MIT license. See `LICENSE` for more information.