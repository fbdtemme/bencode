![](docs/images/bencode.svg)

![build](https://github.com/fbdtemme/bencode/workflows/build/badge.svg?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/5cc3eec94d8a486dab62afeab5130def)](https://app.codacy.com/manual/floriandetemmerman/bencode?utm_source=github.com&utm_medium=referral&utm_content=fbdtemme/bencode&utm_campaign=Badge_Grade_Dashboard)
[![codecov](https://codecov.io/gh/fbdtemme/bencode/branch/master/graph/badge.svg)](https://codecov.io/gh/fbdtemme/bencode)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A header-only C++20 bencode serialization/deserialization library.

## Features

*  Convenient owning representation of bencoded data with `bvalue`.
*  Fast and memory efficient read-only, non-owning representation into stable buffers 
   of bencoded data with `bview`.
*  Build-in serialization/deserializaton for most standard containers.
*  Support for serializing/deserializing to/from user-defined types. 
*  Parse directly to custom types by satisfying the `EventConsumer` concept.
*  Throwing and non throwing variants of common functions.

## Documentation

Documentation is available on the [bencode GitHub pages](https://fbdtemme.github.io/bencode/)

## Requirements

This project requires C++20. 
Currently only GCC 10 is supported.

This library depends on following projects:
* [Catch2](https://github.com/catchorg/Catch2)
* [fmt](https://github.com/fmtlib/fmt)
* [Microsoft GSL](https://github.com/microsoft/GSL)
* [expected-lite](https://github.com/martinmoene/expected-lite)

All dependencies can be fetched from github during configure time or can be installed manually.

## Examples

Decode a value to a `bvalue`.

```cpp
// All examples use namespace bc for brevity
namespace bc = bencode;
```

```cpp
#include <bencode/bencode.hpp> 
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
#include <bencode/bencode.hpp> 
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
#include <bencode/bencode.hpp> 
#include <bencode/bvalue.hpp> 

bc::value b {
  {"foo", 1},
  {"bar", 2},
  {"baz", bc::bvalue(bc::btype::list, {1, 2, 3})},
};

// prints "d3:bari2e3:bazli1ei2ei3ee3:fooi1ee";
bc::encode_to(std::cout, b);
```

Serialize to bencode using `encoder`

```cpp
#include <bencode/bencode.hpp> 

bc::encoder es(std::cout);

es << bc::begin_dict
   << "foo" << 1
   << "bar" << 2
   << "baz" << std::vector{1, 2, 3}
   << end_dict 

// prints "d3:bari2e3:bazli1ei2ei3ee3:fooi1ee";
```

For more examples see the [documentation](https://fbdtemme.github.io/bencode/) 
