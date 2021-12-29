#pragma once

#include <array>
#include <span>
#include <algorithm>

namespace std {
    template <size_t... _Extents>
    class extents {
    public:
        using size_type = size_t;

        // [mdspan.extents.cons], Constructors and assignment
        constexpr extents() noexcept               = default;
        constexpr extents(const extents&) noexcept = default;
        constexpr extents& operator=(const extents&) noexcept = default;
        constexpr extents(extents&&) noexcept                 = default;
        constexpr extents& operator=(extents&&) noexcept = default;

        template <size_t... _OtherExtents,
            enable_if_t<sizeof...(_OtherExtents) == extents::rank()
                            && ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent
                                    || _OtherExtents == _Extents)
                                && ...),
                int> = 0>
        constexpr extents(const extents<_OtherExtents...>& _Other) noexcept {
            auto _Dynamic_it = _Dynamic_extents.begin();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _STL_VERIFY(_Static_extents[_Dim] == dynamic_extent || _Static_extents[_Dim] == _Other.extent(_Dim),
                    "Cannot construct an extent from another extent with different rank sizes.");
                if (_Static_extents[_Dim] == dynamic_extent) {
                    *_Dynamic_it++ = _Other.extent(_Dim);
                }
            }
        }

        template <size_t... _OtherExtents,
            enable_if_t<sizeof...(_OtherExtents) == extents::rank()
                            && ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent
                                    || _OtherExtents == _Extents)
                                && ...),
                int> = 0>
        constexpr extents& operator=(const extents<_OtherExtents...>& _Other) noexcept {
            auto _Dynamic_it = _Dynamic_extents.begin();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _STL_VERIFY(_Static_extents[_Dim] == dynamic_extent || _Static_extents[_Dim] == _Other.extent(_Dim),
                    "Cannot assign an extent to another extent with different rank sizes.");
                if (_Static_extents[_Dim] == dynamic_extent) {
                    *_Dynamic_it++ = _Other.extent(_Dim);
                }
            }

            return *this;
        }

        static constexpr size_t rank_dynamic() noexcept {
            return ((_Extents == dynamic_extent) + ...);
        }

        template <class... _SizeTypes, enable_if_t<((is_convertible_v<_SizeTypes, size_type>) &&...)
                                                       && (sizeof...(_SizeTypes) == extents::rank_dynamic()),
                                           int> = 0>
        constexpr extents(_SizeTypes... _Dynamic) noexcept : _Dynamic_extents{static_cast<size_type>(_Dynamic)...} {}

        template <class _SizeType, enable_if_t<is_same_v<_SizeType, size_type>, int> = 0>
        constexpr extents(const array<_SizeType, extents::rank_dynamic()>& _Dynamic) noexcept
            : _Dynamic_extents{_Dynamic} {}

        template <class _SizeType,
            enable_if_t<!is_same_v<_SizeType, size_type> && is_convertible_v<_SizeType, size_t>, int> = 0>
        constexpr extents(const array<_SizeType, extents::rank_dynamic()>& _Dynamic) noexcept {
            for (size_t _Dim = 0; _Dim < rank_dynamic(); ++_Dim) {
                _Dynamic_extents[_Dim] = _Dynamic[_Dim];
            }
        }

        // [mdspan.extents.obs], Observers of the domain multidimensional index space
        static constexpr size_t rank() noexcept {
            return sizeof...(_Extents);
        }

        static constexpr size_type static_extent(const size_t _Idx) noexcept {
            return _Static_extents[_Idx];
        }

        constexpr size_type extent(const size_t _Idx) const noexcept {
            if constexpr (rank_dynamic() == 0) {
                return _Static_extents[_Idx];
            } else if constexpr (rank_dynamic() == rank()) {
                return _Dynamic_extents[_Idx];
            } else {
                const auto _Static_extent = _Static_extents[_Idx];
                if (_Static_extent == dynamic_extent) {
                    return _Dynamic_extents[_Dynamic_index(_Idx)];
                } else {
                    return _Static_extent;
                }
            }
        }

        // [mdspan.extents.compare], extents comparison operators
        template <size_t... _OtherExtents>
        friend constexpr bool operator==(const extents& _Lhs, const extents<_OtherExtents...>& _Rhs) noexcept {
            if constexpr (sizeof...(_Extents) != sizeof...(_OtherExtents)) {
                return false;
            }

            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                if (_Lhs.extent(_Dim) != _Rhs.extent(_Dim)) {
                    return false;
                }
            }

            return true;
        }

    private:
        static constexpr size_t _Dynamic_index(const size_t _Idx) noexcept {
            size_t _Result{0};
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _Result += (_Dim < _Idx && _Static_extents[_Dim] == dynamic_extent);
            }

            return _Result;
        }

        array<size_type, rank_dynamic()> _Dynamic_extents = {};

        inline static constexpr size_t _Static_extents[] = {_Extents...};
    };


    struct layout_left {
        template <class Extents>
        class mapping {
        public:
            using size_type    = typename Extents::size_type;
            using extents_type = Extents;
            using layout_type  = layout_left;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            constexpr mapping(const Extents& e) noexcept : extents_{e} {};

            template <class OtherExtents, enable_if_t<is_constructible_v<Extents, OtherExtents>, int> = 0>
            explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
                const mapping<OtherExtents>& _Other) noexcept
                : extents_{_Other.extents()} {};

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //    const layout_right::template mapping<OtherExtents>& _Other) noexcept requires(OtherExtents::rank() <=
            //    1) : extents_{_Other.extents()} {}

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(OtherExtents::rank() > 0) constexpr mapping(
            //    const layout_stride::template mapping<OtherExtents>& other)
            //    : extents_{_Other.extents()} {}

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

            constexpr Extents extents() const noexcept {
                return extents_;
            }

            constexpr size_type required_span_size() const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < Extents::rank(); ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class... Indices,
                enable_if_t<sizeof...(Indices) == Extents::rank() && (is_convertible_v<Indices, size_type> && ...),
                    int> = 0>
            constexpr size_type operator()(Indices... _Idx) const noexcept {
                return _Index_impl<Indices...>(_Idx..., make_index_sequence<Extents::rank()>{});
            }

            static constexpr bool is_always_unique() noexcept {
                return true;
            }
            static constexpr bool is_always_contiguous() noexcept {
                return true;
            }
            static constexpr bool is_always_strided() noexcept {
                return true;
            }

            constexpr bool is_unique() const noexcept {
                return true;
            }
            constexpr bool is_contiguous() const noexcept {
                return true;
            }
            constexpr bool is_strided() const noexcept {
                return true;
            }

            constexpr size_type stride(size_t _Rank) const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < _Rank; ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class OtherExtents>
            friend constexpr bool operator==(const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents();
            }

        private:
            Extents extents_{}; // exposition only

            template <class... Indices, size_t... I>
            constexpr size_type _Index_impl(Indices... _Idx, index_sequence<I...>) const noexcept {
                return ((_Idx * stride(I)) + ...);
            }
        };
    };

    struct layout_right {
        template <class Extents>
        class mapping {
        public:
            using size_type    = typename Extents::size_type;
            using extents_type = Extents;
            using layout_type  = layout_right;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            constexpr mapping(const Extents& e) noexcept : extents_{e} {};

            //        template <class OtherExtents, enable_if_t<is_constructible_v<Extents, OtherExtents>, int> = 0>
            //        explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //            const mapping<OtherExtents>& _Other) noexcept
            //            : extents_{_Other.extents()} {};

            // vvv COPY PASTA vvv

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //    const layout_right::template mapping<OtherExtents>& _Other) noexcept
            //        requires(OtherExtents::rank() <=
            //    1) : extents_{_Other.extents()} {}

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(OtherExtents::rank() > 0) constexpr mapping(
            //    const layout_stride::template mapping<OtherExtents>& other)
            //    : extents_{_Other.extents()} {}

            // ^^^ COPY PASTA ^^^

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

            constexpr Extents extents() const noexcept {
                return extents_;
            }

            constexpr size_type required_span_size() const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < Extents::rank(); ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class... Indices,
                enable_if_t<sizeof...(Indices) == Extents::rank() && (is_convertible_v<Indices, size_type> && ...),
                    int> = 0>
            constexpr size_type operator()(Indices... _Idx) const noexcept {
                return _Index_impl<Indices...>(_Idx..., make_index_sequence<Extents::rank()>{});
            }

            static constexpr bool is_always_unique() noexcept {
                return true;
            }
            static constexpr bool is_always_contiguous() noexcept {
                return true;
            }
            static constexpr bool is_always_strided() noexcept {
                return true;
            }

            constexpr bool is_unique() const noexcept {
                return true;
            }
            constexpr bool is_contiguous() const noexcept {
                return true;
            }
            constexpr bool is_strided() const noexcept {
                return true;
            }

            constexpr size_type stride(size_t _Rank) const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = _Rank + 1; _Dim < Extents::rank(); ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class OtherExtents>
            friend constexpr bool operator==(const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents();
            }

        private:
            Extents extents_{}; // exposition only

            template <class... Indices, size_t... I>
            constexpr size_type _Index_impl(Indices... _Idx, index_sequence<I...>) const noexcept {
                return ((_Idx * stride(I)) + ...);
            }
        };
    };


    struct layout_stride {
        template <class Extents>
        class mapping {
        public:
            using size_type    = typename Extents::size_type;
            using extents_type = Extents;
            using layout_type  = layout_stride;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            template <class SizeType, size_t N>
            constexpr mapping(const Extents&, const array<SizeType, N>&) noexcept;

            template <class OtherExtents>
            explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
                const mapping<OtherExtents>&) noexcept;

            //template <class LayoutMapping>
            //explicit(see below) constexpr mapping(const LayoutMapping&) noexcept;

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

            constexpr Extents extents() const noexcept {
                return extents_;
            }

            constexpr array<typename size_type, Extents::rank()> strides() const noexcept {
                return strides_;
            }

            constexpr size_type required_span_size() const noexcept;

            template <class... Indices>
            constexpr size_type operator()(Indices...) const noexcept;

            static constexpr bool is_always_unique() noexcept {
                return true;
            }

            static constexpr bool is_always_contiguous() noexcept {
                return false;
            }

            static constexpr bool is_always_strided() noexcept {
                return true;
            }

            constexpr bool is_unique() const noexcept {
                return true;
            }

            constexpr bool is_contiguous() const noexcept {

            }

            constexpr bool is_strided() const noexcept {
                return true;
            }

            constexpr size_type stride(const size_t _Idx) const noexcept {
                return strides_[_Idx];
            }

            template <class OtherExtents>
            friend constexpr bool operator==(const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents();
            }

        private:
            Extents extents_{}; // exposition only
            array<size_type, Extents::rank()> strides_{}; // exposition only
        };
    };
} // namespace std