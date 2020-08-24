# Changelog

## v0.1.1

*   Fix invalid output of `events::debug_to` for lists and dicts.
*   Replace macro `BENCODE_SERIALIZES_TO_DICT` by `BENCODE_SERIALIZES_TO_DICT_SORTED`
    and `BENCODE SERIALIZED_TO_DICT_UNSORTED`. `serializes_to_dict` 
    now takes an enum non-type template argument `dict_key_order` 
    to indicate if the dict default iteration order is sorted or not.
*   Fix various event producing compilation errors.

## v0.1.0

Initial release.