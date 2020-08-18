//#include <catch2/catch.hpp>
//#include <string_view>
//
//#include <bencode/bencode.hpp>
//#include <bencode/traits/all.hpp>
//
//using namespace std::string_view_literals;
//
//
//TEST_CASE("test in_place constuction methods", "[construction]")
//{
//    SECTION("\"bencode::make_value\" factory method") {
//        SECTION("uninitialized") {
//            constexpr auto t = bencode::btype::uninitialized;
//            auto b = bencode::bvalue(t);
//            CHECK(b.type() == t);
//        }
//        SECTION("dict") {
//            constexpr auto t = bencode::btype::dict;
//            bencode::bvalue::dict_type reference_d{{"test1", 1},
//                                                  {"test2", "2"}};
//
//            SECTION("empty") {
//                auto b = bencode::bvalue(t);
//                CHECK(b.type() == t);
//                CHECK(get_dict(b).empty());
//            }
//
//            SECTION("copy construction") {
//                auto b = bencode::bvalue(t, reference_d);
//                CHECK(b.type() == t);
//                CHECK(get_dict(b) == reference_d);
//            }
//
//            SECTION("initializer_list") {
//                auto b = bencode::bvalue(t, {{"test1", 1},
//                                                  {"test2", "2"}});
//                CHECK(b.type() == t);
//                CHECK(get_dict(b) == reference_d);
//            }
//        }
//
//        SECTION("list") {
//            constexpr auto t = bencode::btype::list;
//            bencode::bvalue::list_type reference_l{1, 1u, false, "string", bencode::bvalue()};
//
//            SECTION("empty") {
//                auto b = bencode::bvalue(t);
//                CHECK(b.type() == t);
//            }
//
//            SECTION("copy construction") {
//                auto b = bencode::bvalue(t, reference_l);
//                CHECK(b.type() == t);
//                CHECK(get_list(b) == reference_l);
//            }
//
//            SECTION("initializer_list") {
//                auto b = bencode::bvalue(t, {1, 1u, false, "string", bencode::bvalue{}});
//                CHECK(b.type() == t);
//                CHECK(get_list(b) == reference_l);
//            }
//        }
//
//        SECTION("string") {
//            constexpr auto t = bencode::btype::string;
//            bencode::bvalue::string_type reference_s{'a', 'b', 'c'};
//
//            SECTION("empty") {
//                auto b = bencode::bvalue(t);
//                CHECK(b.type() == t);
//                CHECK(b == "");
//            }
//            SECTION("copy construction") {
//                auto b = bencode::bvalue(t, reference_s);
//                CHECK(b.type() == t);
//                CHECK(b == reference_s);
//            }
//
//            SECTION("initializer_list") {
//                auto b = bencode::bvalue(t, {'a', 'b', 'c'});
//                CHECK(b.type() == t);
//                CHECK(b == reference_s);
//            }
//        }
//
//        SECTION("integer") {
//            constexpr auto t = bencode::btype::integer;
//            bencode::bvalue b(t);
//            CHECK(b.type() == t);
//            CHECK(b == 0);
//        }
//    }
//
////    SECTION("named factory method") {
////        SECTION("dict") {
////            constexpr auto t = bencode::value_type::dict;
////            bencode::bvalue::dict_type reference_d{{"test1", 1},
////                                                  {"test2", "2"}};
////
////            SECTION("empty") {
////                auto b = bencode::make_dict();
////                CHECK(b.type() == t);
////                CHECK(get_dict(b).empty());
////            }
////
////            SECTION("copy construction") {
////                auto b = bencode::make_dict(reference_d);
////                CHECK(b.type() == t);
////                CHECK(get_dict(b) == reference_d);
////            }
////
////            SECTION("initializer_list") {
////                auto b = bencode::make_dict({{"test1", 1},
////                                             {"test2", "2"}});
////                CHECK(b.type() == t);
////                CHECK(get_dict(b) == reference_d);
////            }
////        }
////
////        SECTION("list") {
////            constexpr auto t = bencode::value_type::list;
////            bencode::bvalue::list_type reference_l{1, 1u, false, "string", bencode::bvalue()};
////
////            SECTION("empty") {
////                auto b = bencode::make_list();
////                CHECK(b.type() == t);
////            }
////
////            SECTION("copy construction") {
////                auto b = bencode::make_list(reference_l);
////                CHECK(b.type() == t);
////                CHECK(get_list(b) == reference_l);
////            }
////
////            SECTION("initializer_list") {
////                auto b = bencode::make_list({1, 1u, false, "string", bencode::bvalue{}});
////                CHECK(b.type() == t);
////                CHECK(get_list(b) == reference_l);
////            }
////        }
////
////        SECTION("string") {
////            constexpr auto t = bencode::value_type::string;
////            bencode::bvalue::string_type reference_s{'a', 'b', 'c'};
////
////            SECTION("empty") {
////                auto b = bencode::make_string();
////                CHECK(b.type() == t);
////                CHECK(b == "");
////            }
////            SECTION("copy construction") {
////                auto b = bencode::make_string(reference_s);
////                CHECK(b.type() == t);
////                CHECK(b.get_string() == reference_s);
////            }
////
////            SECTION("initializer_list") {
////                auto b = bencode::make_string({'a', 'b', 'c'});
////                CHECK(b.type() == t);
////                CHECK(b.get_string() == reference_s);
////            }
////        }
////
////        SECTION("string_view") {
////            constexpr auto t = bencode::value_type::string_view;
////            auto b = bencode::make_string_view();
////            CHECK(b.type() == t);
////            CHECK(b == ""sv);
////        }
////
////        SECTION("integer") {
////            constexpr auto t = bencode::value_type::integer;
////            auto b = bencode::make_integer();
////            CHECK(b.type() == t);
////            CHECK(b == 0);
////        }
////    }
//}