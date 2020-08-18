# bencode

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/5cc3eec94d8a486dab62afeab5130def)](https://app.codacy.com/manual/floriandetemmerman/bencode?utm_source=github.com&utm_medium=referral&utm_content=fbdtemme/bencode&utm_campaign=Badge_Grade_Dashboard)

![build](https://github.com/fbdtemme/bencode/workflows/build/badge.svg?branch=master)

A C++20 bencode serialization/deserialization library.

## Features

* Convenient owning representation of bencoded data with `bvalue` .
* Fast and memory efficient read-only, non-owning representation of bencoded data with `bview`.
* Build-in serialization/deserializaton for most standard containers.
* Support for serializing/deserializing to/from user-defined types. 
* Parse directly to custom types using the `EventConsumer` interface.

## Requirements

This project targets C++20. 
Currently only GCC 10 is tested.

## Dependencies:

* [Catch2](https://github.com/catchorg/Catch2) for testing.
* [fmt](https://github.com/fmtlib/fmt)
* [Microsoft GSL](https://github.com/microsoft/GSL)
* [expected-lite](https://github.com/martinmoene/expected-lite)


## Status

This library is a work in progress.


## Building

```
mkdir build
cd buikd
cmake ..
make 
make install
``` 
## Documentation

[Documentation](https://fbdtemme.github.io/bencode/) is hosted on github pages 

