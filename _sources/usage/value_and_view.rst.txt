.. cpp:namespace:: bencode

Value and view types
====================

This library contains two main types to represent bencoded data.
A value types and a view type.
They can be conceptually compared the value type :cpp:class:`std::string`
and the view type :cpp:class:`std::string_view`.

:cpp:class:`bvalue` owns the data it contains. Parsing to :cpp:class:`bvalue` involves copying data to
the internal storage of a :cpp:class:`bvalue`. :cpp:class:`bvalue` can modify the stored values.

:cpp:class:`bview` does not own any data.
Parsing to :cpp:class:`bview` is done by creating an index into a stable buffer with the
bencoded data, a :cpp:class:`descriptor_table`.
Based on this index :cpp:class:`bview` provides read-only access to the bencoded data.

The interfaces of :cpp:class:`bvalue` and :cpp:class:`bview` are very similar and use free functions.
This enable the use of both types in a single generic function.

Performance
-----------

Parsing to :cpp:class:`bview` is about 5 times faster then parsing to :cpp:class:`bvalue`.

.. image:: ../images/benchmark_barplot.svg

Contents
--------

:ref:`value`
:ref:`view`

