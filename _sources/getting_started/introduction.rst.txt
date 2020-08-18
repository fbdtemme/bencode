.. cpp:namespace:: bencode

Introduction
============
.. important::

     This library targets the C++20 standard. Compiler support is limited to GCC 10 for now.

Features
--------
* Convenient owning representation of bencoded data with :cpp:class:`bvalue`.
* Fast, memory efficient, read-only, non-owning representation of bencoded data :cpp:class:`bview`.
* Build-in interoperability with most STL containers.
* Support for user-defined types.
* Parse directly to custom types by implementing the :cpp:concept:`event_consumer` interface.


Dependencies
------------

* `fmt <https://github.com/fmtlib/fmt>`_
* `Microsoft GSL <https://github.com/microsoft/GSL>`_
* `expected-lite <https://github.com/martinmoene/expected-lite>`_
* `Catch2 <https://github.com/catchorg/Catch2>`_ when building tests.


