Descriptor table
================

A descriptor_table holds a contiguous sequence of descriptors and a pointer to a bencoded string.

Descriptor
----------

A descriptor is a 16 byte structure that describes the structural elements of a bencoded value.
Each bencoded value has at least one :cpp:class:`descriptor`.
A descriptor stores the type of the bencode data type in a :cpp:enum:`descriptor_type`
value.
It also stores the position in the bencoded string the described structural element starts.

Dictionary



