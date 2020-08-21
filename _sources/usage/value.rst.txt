.. cpp:namespace:: bencode

Value interface
===============

Introduction
------------

:cpp:class:`bvalue` is class that represents a bencoded value.
It is a sum-type implemented with a :cpp:class:`std::variant` to stores the
different bencode data types.

:cpp:class:`bvalue` is an instantiation of the class template
:cpp:class:`template <typename Policy> basic_bvalue` and the exact types used to store
the different bencode data types can be customized through the Policy template argument.

The default policy uses following storage types:

+----------+----------------------+
| integer  | :code:`std::int64_t` |
+----------+----------------------+
| string   | :code:`std::string`  |
+----------+----------------------+
| list     | :code:`std::vector`  |
+----------+----------------------+
| dict     | :code:`std::map`     |
+----------+----------------------+

The bvalue class can be used to create, and manipulate bencode values.

.. note::

    In most examples :cpp:class:`bvalue` can be replaced by
    :cpp:class:`basic_bvalue` with a user-defined Policy template argument.

Construction
------------

:cpp:class:`bvalue` provides similar constructors and assignment operators as :code:`std::variant`.

Construct an empty bvalue.

.. code-block:: cpp

    auto b = bc::bvalue {};

Arguments matching one of the storage types will be forwarded to the constructor of the
underlying type.

.. code-block:: cpp

    // Copy initialize an integer
    bc::bvalue bi = 3;

    // Copy initialize a bvalue holding a string.
    bc::bvalue b3 = "string";

The type can be passed explicitly using a type tag.
This is necessary to distinguish between initializer-lists for string, list and dict bencode types.

.. code-block:: cpp

    // construct a vector of 5 times 1
    bc::bvalue b4(bc::btype::list, {5, 1});


Converting constructors allow any type that has the required customization points implemented
to be passed to the bvalue constructor.

.. code-block:: cpp

    #include <bencode/traits/list.hpp>

    auto l = std::list{1, 2, 4, 5};
    auto b = bencode::bvalue(l);

Accessors
---------

Check the current activate alternative.

.. code-block::  cpp

    bc::bvalue b = "string";

    // returns true
    is_string(b)

    // return false
    is_dict(b)


Get a reference to the underlying type.

.. code-block:: cpp

    bc::bvalue b = "string";

    using bv = bc::bencode_type;

    // access by enum / type tag
    auto& s1 = get<bc::btype::string>(b); // is equivalent to: get<bc::bencode_type::string>(b);

    // access by exact type
    auto& s1 = get<std::string>(b);

    // or the more succinct version
    auto& s = get_string(b);

    auto& l = get_list(b); // throws bad_bvalue_access

    // move the string out of the bvalue.
    auto s = get_string(std::move(b));

Non throwing accessors.

.. code-block:: cpp

    bc::bvalue b = "string";

    using bv = bc::bencode_type;

    // access by enum / type tag
    auto* s1 = get_if<bc::btype::string>(&b);

    // access by exact type
    auto* s2 = get_if<std::string>(&b);

    // or the more succinct version
    auto* s = get_if_string(&b);

    auto* l = get_if_list(b); // l is nullptr

Converting accessors.

.. code-block:: cpp

    bencode::bvalue b = "string";

    auto bytes = get_as<std::vector<std::byte>>(b);


Modifying operation
-------------------

The emplace family of functions will discard the current value and construct
a new value in place forwarding the arguments to the constructor of the underlying type.

.. code-block::  cpp

    auto b = bc::bvalue(20);
    b.emplace_list({1, 2, 3});
    auto& l = get_list(b);

There are some convenience functions when the underlying type is a list or dict to directly access
the underlying type without first obtaining a reference with accessor functions.

.. code-block::  cpp

    auto blist = bc::bvalue(bc::type::list, {1, 2, 3, 4});
    auto bdict = bc::bdict(bc::type::dict, {
        {"one", 1}.
        {"two", 2}
    });

    // get reference to the first element of an array
    auto& first = blist.at(0);

    // change second item
    blist[1] = 0;

    // get reference to the value for key "one"
    auto& first_val = bdict.at("one");

    first_val["one"] = 2;

    // append new elements
    blist.push_back(5);
    blist.emplace_back(bc::type::list, {"a", "b", "c"});

    // check if there is a key "one"
    auto has_one = bdict.contains("one");

    // clear elements
    blist.clear();
    bdict.clear();


Comparison
----------

Comparison operators will look through the bvalue and compare with the underling type.
If the type you compare with does not match the type of the value contained in bvalue
the fallback ordering is defined by the ordering of the types:
:code:`integer < string < list < dict`.

.. code-block::  cpp

    bc::bvalue b_int(2);

    // true
    auto t = (b == 2);

    bc::bvalue b_string("test");

    // true
    auto t2 = b_string < "zzzz"

    bc::bvalue b_dict(bc::btype::dict, {{"one", 1}, {"two", 2}});
    auto dict = std::map({{"one", 1}, {"two", 2}});

    // returns true
    b_dict == dict;

Policies
========

The types used to store the different alternatives can be modified with the Policy template
argument.



