.. cpp:namespace:: bencode


Events system
================

The events interface allows different parts of the library to communicate and to efficiently
convert between data representations.
It consists of two concepts :cpp:concept:`event_consumer` and :cpp:concept:`event_producer`.

Event consumers
---------------

The event consumer concepts is modeled after the SAX-interface for XML.
The most simple consumer which ignores all events would look as following:

.. code-block:: cpp

    struct discard_consumer
    {
        void integer(std::int64_t value) { }
        void string(std::string_view value) { }
        void begin_list() {}
        void begin_list(std::size_t size) {}
        void end_list() {}
        void end_list(std::size_t size) {}
        void list_item() {}
        void begin_dict() {}
        void begin_dict(std::size_t size) {}
        void end_dict() {}
        void end_dict(std::size_t size) {}
        void dict_key() {}
        void dict_value() {}
    };

Every bencode data structure can be discribed by successive calls to this inteface.
Following bencode data structure (formatted as JSON) would generate these calls.

.. code-block::

    {
      "foo": ["a", "b"],
      "bar": {"one": 1, "two": 2}
    }

    begin_dict(2)
        string("foo")
        dict_key()
        begin_list(2)
            begin_string("a")
            list_item()
            string("b")
            list_item()
        end_list()
        dict_value()
        string("bar")
        dict_key()
        begin_dict(2)
            string("one")
            dict_key()
            integer(1)
            dict_value()
            string("two")
            dict_key()
            integer(2)
            dict_value()
        end_dict()
        dict_value()
    end_dict()

Note that list_item(), dict_key() and dict_value() are called after describing the value.

Event producers
---------------

The :cpp:concept:`event_producer` concept is defined as any object that can generate calls to a
class satisfying the :cpp:concept:`event_consumer` concept through the
:cpp:func:`connect` method.


Connecting consumer and producers
---------------------------------

:cpp:concept:`event_producers` are connected to :cpp:concept:`event_consumers`
with the connect function.

:cpp:func:`template \<event_consumer EC, event_producer U> constexpr void connect(EC& consumer, U&& producer)`
