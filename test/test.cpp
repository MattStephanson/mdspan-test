#include "CppUnitTest.h"
#include "mdspan.h"
#include <type_traits>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace test {

    template <class T>
    using is_semiregular_trivial_nothrow = std::conjunction<is_trivially_copyable<T>,
        is_nothrow_default_constructible<T>, is_nothrow_copy_constructible<T>, is_nothrow_move_constructible<T>,
        is_nothrow_copy_assignable<T>, is_nothrow_move_assignable<T>>;

    template <class T>
    inline constexpr bool is_semiregular_trivial_nothrow_v = is_semiregular_trivial_nothrow<T>::value;

    TEST_CLASS(extent_tests){
    public :
        TEST_METHOD(traits) {
            static_assert(is_semiregular_trivial_nothrow_v<extents<2, 3>>);
            static_assert(is_semiregular_trivial_nothrow_v<extents<dynamic_extent, 3>>);
            static_assert(is_semiregular_trivial_nothrow_v<extents<2, dynamic_extent>>);
            static_assert(is_semiregular_trivial_nothrow_v<extents<dynamic_extent, dynamic_extent>>);
        }

        TEST_METHOD(static_extent) {
            static_assert(extents<2, 3>::static_extent(0) == 2);
            static_assert(extents<2, 3>::static_extent(1) == 3);

            static_assert(extents<2, dynamic_extent>::static_extent(0) == 2);
            static_assert(extents<2, dynamic_extent>::static_extent(1) == dynamic_extent);

            static_assert(extents<dynamic_extent, 3>::static_extent(0) == dynamic_extent);
            static_assert(extents<dynamic_extent, 3>::static_extent(1) == 3);

            static_assert(extents<dynamic_extent, dynamic_extent>::static_extent(0) == dynamic_extent);
            static_assert(extents<dynamic_extent, dynamic_extent>::static_extent(1) == dynamic_extent);
        }

        TEST_METHOD(copy_ctor_sizes) {
            static_assert(!is_constructible_v<extents<dynamic_extent>, int*>);

            extents<2, 3> e0;
            Assert::AreEqual<size_t>(e0.extent(0), 2u);
            Assert::AreEqual<size_t>(e0.extent(1), 3u);

            extents<2, dynamic_extent> e1(5u);
            Assert::AreEqual<size_t>(e1.extent(0), 2u);
            Assert::AreEqual<size_t>(e1.extent(1), 5u);

            extents<dynamic_extent, 3> e2(5u);
            Assert::AreEqual<size_t>(e2.extent(0), 5u);
            Assert::AreEqual<size_t>(e2.extent(1), 3u);

            extents<dynamic_extent, dynamic_extent> e3(5u, 7u);
            Assert::AreEqual<size_t>(e3.extent(0), 5u);
            Assert::AreEqual<size_t>(e3.extent(1), 7u);

            // convertible size type
            extents<2, dynamic_extent> e4(5);
            Assert::AreEqual<size_t>(e4.extent(0), 2u);
            Assert::AreEqual<size_t>(e4.extent(1), 5u);

            extents<dynamic_extent, 3> e5(5);
            Assert::AreEqual<size_t>(e5.extent(0), 5u);
            Assert::AreEqual<size_t>(e5.extent(1), 3u);

            extents<dynamic_extent, dynamic_extent> e6(5, 7);
            Assert::AreEqual<size_t>(e6.extent(0), 5u);
            Assert::AreEqual<size_t>(e6.extent(1), 7u);
        }

        TEST_METHOD(copy_ctor_other) {
            static_assert(!is_constructible_v<extents<2>, extents<2, 3>>);
            static_assert(!is_constructible_v<extents<2, 3>, extents<2>>);
            static_assert(!is_constructible_v<extents<2, 3>, extents<3, 2>>);

            extents<2, 3> e0{extents<2, 3>{}};
            extents<2, 3> e1{extents<dynamic_extent, 3>{2u}};
            extents<2, 3> e2{extents<2, dynamic_extent>{3u}};
            extents<2, 3> e3{extents<dynamic_extent, dynamic_extent>{2u, 3u}};
        }
        TEST_METHOD(copy_ctor_array) {
            static_assert(!is_constructible_v<extents<dynamic_extent>, array<int*,1>&>);

            extents<2, 3> e0;
            Assert::AreEqual<size_t>(e0.extent(0), 2u);
            Assert::AreEqual<size_t>(e0.extent(1), 3u);

            // native extent::size_type
            extents<2, dynamic_extent> e1(array<size_t, 1>{5});
            Assert::AreEqual<size_t>(e1.extent(0), 2u);
            Assert::AreEqual<size_t>(e1.extent(1), 5u);

            extents<dynamic_extent, 3> e2(array<size_t, 1>{5});
            Assert::AreEqual<size_t>(e2.extent(0), 5u);
            Assert::AreEqual<size_t>(e2.extent(1), 3u);

            extents<dynamic_extent, dynamic_extent> e3(array<size_t,2>{5, 7});
            Assert::AreEqual<size_t>(e3.extent(0), 5u);
            Assert::AreEqual<size_t>(e3.extent(1), 7u);

            // convertible size type
            extents<2, dynamic_extent> e4(array{5});
            Assert::AreEqual<size_t>(e4.extent(0), 2u);
            Assert::AreEqual<size_t>(e4.extent(1), 5u);

            extents<dynamic_extent, 3> e5(array{5});
            Assert::AreEqual<size_t>(e5.extent(0), 5u);
            Assert::AreEqual<size_t>(e5.extent(1), 3u);

            extents<dynamic_extent, dynamic_extent> e6(array{5, 7});
            Assert::AreEqual<size_t>(e6.extent(0), 5u);
            Assert::AreEqual<size_t>(e6.extent(1), 7u);
        }

        TEST_METHOD(assign) {
            static_assert(!is_assignable_v<extents<2>, extents<2, 3>>);
            static_assert(!is_assignable_v<extents<2, 3>, extents<2>>);
            static_assert(!is_assignable_v<extents<2, 3>, extents<3, 2>>);

            extents<2, 3> e0;
            e0 = extents<2, 3>{};
            e0 = extents<dynamic_extent, 3>{2u};
            e0 = extents<2, dynamic_extent>{3u};
            e0 = extents<dynamic_extent, dynamic_extent>{2u, 3u};
        }

        TEST_METHOD(equality) {
            static_assert(extents<2, 3>{} == extents<2, 3>{});
            static_assert(extents<2, 3>{} != extents<3, 2>{});
            static_assert(extents<2>{} != extents<2, 3>{});
            Assert::IsTrue(extents<2, 3>{} == extents<2, dynamic_extent>{3});
            Assert::IsTrue(extents<2, 3>{} == extents<dynamic_extent, 3>{2});
            Assert::IsTrue(extents<2, 3>{} == extents<dynamic_extent, dynamic_extent>{2, 3});
            Assert::IsTrue(extents<dynamic_extent, 3>{2} == extents<2, dynamic_extent>{3});
        }
    };

    TEST_CLASS(layout_left_tests){
    public:
        TEST_METHOD(traits) {
            static_assert(is_semiregular_trivial_nothrow_v<layout_left::mapping<extents<2, 3>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_left::mapping<extents<dynamic_extent, 3>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_left::mapping<extents<2, dynamic_extent>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_left::mapping<extents<dynamic_extent, dynamic_extent>>>);
        }
        
        TEST_METHOD(properties) {
            constexpr layout_left::mapping<extents<2, 3>> map{};
            static_assert(map.is_unique() == true);
            static_assert(map.is_contiguous() == true);
            static_assert(map.is_strided() == true);

            static_assert(decltype(map)::is_always_unique() == true);
            static_assert(decltype(map)::is_always_contiguous() == true);
            static_assert(decltype(map)::is_always_strided() == true);
        }

        TEST_METHOD(extents_ctor) {
            constexpr extents<2, 3> e1;
            constexpr extents<5, dynamic_extent> e2{7};
            constexpr extents<dynamic_extent, 13> e3{11};
            constexpr extents<dynamic_extent, dynamic_extent> e4{17, 19};

            constexpr layout_left::mapping m1{e1};
            static_assert(m1.extents() == e1);

            constexpr layout_left::mapping m2{e2};
            static_assert(m2.extents() == e2);

            constexpr layout_left::mapping m3{e3};
            static_assert(m3.extents() == e3);

            constexpr layout_left::mapping m4{e4};
            static_assert(m4.extents() == e4);
        }

        template <class Extents>
        void copy_ctor_helper(const Extents& e) {
            const layout_left::mapping m1{e};
            const layout_left::mapping m2{m1};
            Assert::IsTrue(m1 == m2);
        }

        TEST_METHOD(copy_ctor) {
            copy_ctor_helper(extents<2, 3>{});
            copy_ctor_helper(extents<5, dynamic_extent>{7});
            copy_ctor_helper(extents<dynamic_extent, 13>{11});
            copy_ctor_helper(extents<dynamic_extent, dynamic_extent>{17, 19});
        }

        TEST_METHOD(copy_ctor_other) {
            constexpr extents<2, 3> e1;
            constexpr extents<2, dynamic_extent> e2{3};
            constexpr layout_left::mapping<decltype(e1)> m1(e2);
            constexpr layout_left::mapping<decltype(e2)> m2(e1);

            static_assert(m1.extents() == e1);
            static_assert(m2.extents() == e1);
            static_assert(m1.extents() == e2);
            static_assert(m2.extents() == e2);
        }

         template <class Extents>
        void assign_helper(const Extents& e) {
            const layout_left::mapping m1{e};
            layout_left::mapping<Extents> m2;
            m2 = m1;
            Assert::IsTrue(m1 == m2);
        }

        TEST_METHOD(assign) {
            assign_helper(extents<2, 3>{});
            assign_helper(extents<5, dynamic_extent>{7});
            assign_helper(extents<dynamic_extent, 13>{11});
            assign_helper(extents<dynamic_extent, dynamic_extent>{17, 19});
        }

        TEST_METHOD(indexing_static) {
            constexpr layout_left::mapping<extents<2, 3>> map{};
            static_assert(map(0, 0) == 0);
            static_assert(map(1, 0) == 1);
            static_assert(map(0, 1) == 2);
            static_assert(map(1, 1) == 3);
            static_assert(map(0, 2) == 4);
            static_assert(map(1, 2) == 5);
            static_assert(map.required_span_size() == 6);
        }

        TEST_METHOD(indexing_dynamic) {
            constexpr layout_left::mapping map{extents<dynamic_extent, dynamic_extent>{2, 3}};
            static_assert(map(0, 0) == 0);
            static_assert(map(1, 0) == 1);
            static_assert(map(0, 1) == 2);
            static_assert(map(1, 1) == 3);
            static_assert(map(0, 2) == 4);
            static_assert(map(1, 2) == 5);
            static_assert(map.required_span_size() == 6);
        }

        TEST_METHOD(indexing_mixed) {
            constexpr layout_left::mapping map1{extents<dynamic_extent, 3>{2}};
            static_assert(map1(0, 0) == 0);
            static_assert(map1(1, 0) == 1);
            static_assert(map1(0, 1) == 2);
            static_assert(map1(1, 1) == 3);
            static_assert(map1(0, 2) == 4);
            static_assert(map1(1, 2) == 5);
            static_assert(map1.required_span_size() == 6);

            constexpr layout_left::mapping map2{extents<2, dynamic_extent>{3}};
            static_assert(map2(0, 0) == 0);
            static_assert(map2(1, 0) == 1);
            static_assert(map2(0, 1) == 2);
            static_assert(map2(1, 1) == 3);
            static_assert(map2(0, 2) == 4);
            static_assert(map2(1, 2) == 5);
            static_assert(map2.required_span_size() == 6);
        }
    };

    TEST_CLASS(layout_right_tests){
    public :
        TEST_METHOD(traits){
            static_assert(is_semiregular_trivial_nothrow_v<layout_right::mapping<extents<2, 3>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_right::mapping<extents<dynamic_extent, 3>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_right::mapping<extents<2, dynamic_extent>>>);
            static_assert(is_semiregular_trivial_nothrow_v<layout_right::mapping<extents<dynamic_extent, dynamic_extent>>>);
        }

    TEST_METHOD(properties) {
        constexpr layout_right::mapping<extents<2, 3>> map{};
        static_assert(map.is_unique() == true);
        static_assert(map.is_contiguous() == true);
        static_assert(map.is_strided() == true);

        static_assert(decltype(map)::is_always_unique() == true);
        static_assert(decltype(map)::is_always_contiguous() == true);
        static_assert(decltype(map)::is_always_strided() == true);
    }

    TEST_METHOD(extents_ctor) {
        constexpr extents<2, 3> e1;
        constexpr extents<5, dynamic_extent> e2{7};
        constexpr extents<dynamic_extent, 13> e3{11};
        constexpr extents<dynamic_extent, dynamic_extent> e4{17, 19};

        constexpr layout_right::mapping m1{e1};
        static_assert(m1.extents() == e1);

        constexpr layout_right::mapping m2{e2};
        static_assert(m2.extents() == e2);

        constexpr layout_right::mapping m3{e3};
        static_assert(m3.extents() == e3);

        constexpr layout_right::mapping m4{e4};
        static_assert(m4.extents() == e4);
    }

    template <class Extents>
    void copy_ctor_helper(const Extents& e) {
        const layout_right::mapping m1{e};
        const layout_right::mapping m2{m1};
        Assert::IsTrue(m1 == m2);
    }

    TEST_METHOD(copy_ctor) {
        copy_ctor_helper(extents<2, 3>{});
        copy_ctor_helper(extents<5, dynamic_extent>{7});
        copy_ctor_helper(extents<dynamic_extent, 13>{11});
        copy_ctor_helper(extents<dynamic_extent, dynamic_extent>{17, 19});
    }

    TEST_METHOD(copy_ctor_other) {
        constexpr extents<2, 3> e1;
        constexpr extents<2, dynamic_extent> e2{3};
        constexpr layout_right::mapping<decltype(e1)> m1(e2);
        constexpr layout_right::mapping<decltype(e2)> m2(e1);

        static_assert(m1.extents() == e1);
        static_assert(m2.extents() == e1);
        static_assert(m1.extents() == e2);
        static_assert(m2.extents() == e2);
    }

    template <class Extents>
    void assign_helper(const Extents& e) {
        const layout_right::mapping m1{e};
        layout_right::mapping<Extents> m2;
        m2 = m1;
        Assert::IsTrue(m1 == m2);
    }

    TEST_METHOD(assign) {
        assign_helper(extents<2, 3>{});
        assign_helper(extents<5, dynamic_extent>{7});
        assign_helper(extents<dynamic_extent, 13>{11});
        assign_helper(extents<dynamic_extent, dynamic_extent>{17, 19});
    }

    TEST_METHOD(indexing_static) {
        constexpr layout_right::mapping<extents<2, 3>> map{};
        static_assert(map(0, 0) == 0);
        static_assert(map(0, 1) == 1);
        static_assert(map(0, 2) == 2);
        static_assert(map(1, 0) == 3);
        static_assert(map(1, 1) == 4);
        static_assert(map(1, 2) == 5);
        static_assert(map.required_span_size() == 6);
    }

    TEST_METHOD(indexing_dynamic) {
        constexpr layout_right::mapping map{extents<dynamic_extent, dynamic_extent>{2, 3}};
        static_assert(map(0, 0) == 0);
        static_assert(map(0, 1) == 1);
        static_assert(map(0, 2) == 2);
        static_assert(map(1, 0) == 3);
        static_assert(map(1, 1) == 4);
        static_assert(map(1, 2) == 5);
        static_assert(map.required_span_size() == 6);
    }

    TEST_METHOD(indexing_mixed) {
        constexpr layout_right::mapping map1{extents<dynamic_extent, 3>{2}};
        static_assert(map1(0, 0) == 0);
        static_assert(map1(0, 1) == 1);
        static_assert(map1(0, 2) == 2);
        static_assert(map1(1, 0) == 3);
        static_assert(map1(1, 1) == 4);
        static_assert(map1(1, 2) == 5);
        static_assert(map1.required_span_size() == 6);

        constexpr layout_right::mapping map2{extents<2, dynamic_extent>{3}};
        static_assert(map2(0, 0) == 0);
        static_assert(map2(0, 1) == 1);
        static_assert(map2(0, 2) == 2);
        static_assert(map2(1, 0) == 3);
        static_assert(map2(1, 1) == 4);
        static_assert(map2(1, 2) == 5);
        static_assert(map2.required_span_size() == 6);
    }
    }
    ;

} // namespace mdspan
