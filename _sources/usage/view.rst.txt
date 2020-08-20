.. cpp:namespace:: bencode

View interface
==============

Introduction
------------

:cpp:class:`bview` is a sum type that provides access to the values stored in a bencoded buffer.
It holds two pointers, one to a :cpp:class:`descriptor` and a pointer to the bencoded buffer.

The :cpp:class:`descriptor` describes the type and content of the bencoded token the
:cpp:class:`bview` points to and where the data can be found in the bencoded buffer.
This information allows :cpp:class:`bview` to navigate through the bencoded buffer and provide access
to the values.

:cpp:class:`bview` is used together with four subclasses:
:cpp:class:`integer_bview`, :cpp:class:`string_bview`, :cpp:class:`list_bview` and
:cpp:class:`dict_bview`.
These satisfy :cpp:concept:`bview_alternative_type`.
Each subclass provides an extra interface over the generic :cpp:class:`bview`
for the corresponding bencode data type.

:cpp:class:`integer_bview` is implicitly convertible to :cpp:class:`std::uint64_t`.
:cpp:class:`string_bview` provides an interface almost equal to that of :cpp:class:`std::string_view`.
:cpp:class:`list_bview` provides the interface similiar to a const :cpp:class:`std::vector<bc::bview>`
:cpp:class:`dict_bview` provides the interface similar to a const :cpp:class:`std::map` with
:cpp:class:`bc::string_bview`
keys and :cpp:class:`bc::bview` values.

Performance
-----------

:cpp:class:`bview` is about 5 times faster then `bvalue` in read-only scenarios.

Construction
-------------

:cpp:class:`bview` should rarely be constructed directly. :cpp:class:`bview` is the result of calling
:cpp:func:`get_root()` on a :cpp:class:`descriptor_table` instance which is the result of parsing
a bencoded string with :cpp:func:`decode_view`.

.. code-block::

    const std::string data = "d3:cow3:moo4:spam4:eggse";
    bc::descriptor_table desc_table = bencode::decode_view(data);
    bc::bview root_element = desc_table.get_root();


Type checking
-------------

Checking the bencode data type described in a :cpp:class:`bview`
can be done using the following functions:

* :cpp:func:`bool is_integer(const bview&)`
* :cpp:func:`bool is_string(const bview&)`
* :cpp:func:`bool is_list(const bview&)`
* :cpp:func:`bool is_dict(const bview&)`
* :cpp:func:`template \<enum bencode_type E> bool holds_alternative(const bview&)`
* :cpp:expr:`template \<bview_alternative_type T> bool holds_alternative(const T&)`

.. code-block::

    is_integer(root_element)    // returns false
    is_dict(root_element)       // returns true

    // type tag based check
    bc::holds_alternative<bc::type::dict>(root_element); // returns true

    // bview type based check
    bc::holds_alternative<bc::dict_bview>(root_element); // returns true


Access
------

Converting the :cpp:class:`bview` instance to the interface specific
for the bencode datatype it points to is done using accessor functions.

Throwing accessor function will throw :cpp:class:`bad_bview_access` when trying to
convert a bview to a bview subclass that does not match the bencode data type.

* :cpp:func:`const integer_bview& get_integer(const bview&)`
* :cpp:func:`const string_bview& get_string(const bview&)`
* :cpp:func:`const list_bview& get_list(const bview&)`
* :cpp:func:`const dict_bview& get_dict(const bview&)`
* :cpp:func:`template \<enum bencode_type E> const bview_alternative_t<E>& get(const bview&)`
* :cpp:code:`template \<bview_alternative_type T> const T& get(const bview&)`

Non throwing accessor function will return a :cpp:class:`nullptr` when trying to convert
a bview to a bview subclass that does not match the bencode data type.

* :cpp:func:`bool get_if_integer(const bview*)`
* :cpp:func:`bool get_if_string(const bview*)`
* :cpp:func:`bool get_if_list(const bview*)`
* :cpp:func:`bool get_if_dict(const bview*)`
* :cpp:func:`template \<enum bencode_type E> const bview_alternative_t<E>* get_if(const bview*)`
* :cpp:code:`template \<bview_alternative_type T> const T* get_if(const bview&)`


.. code-block:: cpp

    auto dict_view = get_dict(root_element);    // return dict_bview instance
    auto list_view = get_list(root_element)     // throws bad_bview_access

    // type tag based check
    auto get<bc::btype::dict>(root_element);    // return dict_bview instance

    // bview type based check
    auto get<bc::dict_bview>(root_element);     // return dict_bview instance


Comparison
----------

Most types can be compared with :cpp:class:`bview` instances.
When the bencode data type of the :cpp:class:`bview` is not
the same as the bencode type of the the type you compare with when serialized,
the fallback order is `integer < string < list < dict`

Conversion to user-defined types can be enabled by implementing
the necessary :ref:`customization point <customization-compare-to-bview>`.

.. code-block:: cpp

    bview b;    // b is a string_bview with text "foo";
    b == "foo"  // return true
    b > "aa"    // returns true
    b > 3       // return true
    b > std::map<std::string, int> {{"foo", 1}}; // return false


Conversion
----------

To copy the content of a :cpp:class:`bview` value to a specific type, generic converters are used.
The throwing converter will throw :cpp:class:`conversion_error` when an error occures.

* :cpp:func:`template \<typename T> T get_as(const bview&)`

The non throwing converter will return an expected type with the converted value
or an as :cpp:enum:`conversion_errc` value.

* :cpp:func:`template \<typename T> nonstd::expected\<T, conversion_errc> try_get_as(const bview&)`

:cpp:class:`bview` values can be converted to any type that satisfies :cpp:concept:`retrievable_from_bview`.
Conversion to user-defined types can be enabled by implementing
the necessary :ref:`customization point <customization-convert-from-bview>`.


.. code-block::

    // copy a view to a std::map
    auto d = get_as<std::map<std::string, bc::bvalue>>(root_element); //

  // copy a view to a std::map
    auto d2 = try_get_as<std::map<std::string, int>>(root_element);
    if (!d2)
        d2.error()  //  returns conversion_errc::dict_mapped_type_construction_error
                    //  (cannot convert one of the dicts values to int)


Standard library types support
------------------------------

Operations described above are defined for most standard library types.
They are not enabled by default however and the right trait header must be included.
The easiest way is to include all traits but this will have a heavy impact on compile times.

.. code-block:: cpp

    // enable interoperability with all supported types.
    #include <bencode/traits/all.hpp"

    // enable interoperability with std::set, std::unordered_set
    #include <bencode/traits/set.hpp>
