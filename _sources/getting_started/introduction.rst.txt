.. cpp:namespace:: bencode

Introduction
============
Design goals
------------

There are multiple bencode projects available online,
but many are quite limited in scope or have lacking documentation.
The goal of this library is to provide first class support for bencode and to provide all
features expected of a modern serialization/deserialization library.

Features
--------
*  Convenient owning representation of bencoded data with `bvalue`.
*  Fast and memory efficient read-only, non-owning representation into stable buffers
   of bencoded data with `bview`.
*  Build-in serialization/deserializaton for most standard containers.
*  Support for serializing/deserializing to/from user-defined types.
*  Parse directly to custom types by satisfying the `EventConsumer` concept.
*  Throwing and non throwing variants of common functions.


Requirements
------------

Compiler support
++++++++++++++++

This library requires C++20.
Compiler support is limited to GCC10 for the moment.

Dependencies
++++++++++++

This library depends on following external projects.

* `fmt <https://github.com/fmtlib/fmt>`_
* `Microsoft GSL <https://github.com/microsoft/GSL>`_
* `expected-lite <https://github.com/martinmoene/expected-lite>`_
* `Catch2 <https://github.com/catchorg/Catch2>`_, when building tests.

All dependencies are fetched from github using cmake during configuration if
no local installation is found.
