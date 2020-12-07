# Changelog

## v0.3.0

* Add support for std::shared_ptr, std::unique_ptr, std::weak_ptr and raw pointers.
* Add event_connector.
* Add comparison to jimporter/bencode benchmark.

## v0.2.0

*   make `descriptor_table::get_root` not being const qualified.
*   rename `conversion_error` to `bad_conversion`.
*   rename `format_json_to` to `encode_json_to`
*   throw `bencode::out_of_range` instead of `std::out_of_range`
*   add accessor functions to `bview` similar to the accessor functions of `basic_bvalue`:
    *   `at(std::size_t)` 
    *   `at(std::string_view)` 
    *   `operator[](std::size_t)` 
    *   `front()` 
    *   `back()`
*   add `bpointer`: a bencode pointer similar to json pointers.

 
## v0.1.1

*   Fix invalid output of `events::debug_to` for lists and dicts.
*   Replace macro `BENCODE_SERIALIZES_TO_DICT` by `BENCODE_SERIALIZES_TO_DICT_SORTED`
    and `BENCODE SERIALIZED_TO_DICT_UNSORTED`. `serializes_to_dict` 
    now takes an enum non-type template argument `dict_key_order` 
    to indicate if the dict default iteration order is sorted or not.
*   Fix various event producing compilation errors.

## v0.1.0

Initial release.