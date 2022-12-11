// Copyright(c) Matt Stephanson.
// SPDX - License - Identifier: Apache - 2.0 WITH LLVM - exception

#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <tuple>

namespace std {
    template <class _IndexType, size_t _Rank_dynamic, size_t... _Extents>
    struct _Mdspan_extent_type {
        using index_type = _IndexType;

        index_type _Dynamic_extents[_Rank_dynamic];
        static constexpr size_t _Static_extents[sizeof...(_Extents)] = { _Extents... };
        static constexpr array<size_t, sizeof...(_Extents)> _Dynamic_indexes =
            []() constexpr {
            array<size_t, sizeof...(_Extents)> result;
            size_t _Counter = 0;
            for (size_t i = 0; i < sizeof...(_Extents); ++i) {
                result[i] = _Counter;
                if (_Static_extents[i] == dynamic_extent) {
                    ++_Counter;
                }
            }
            return result;
        }();

        constexpr _Mdspan_extent_type() noexcept = default;

        template <class... _OtherIndexTypes, enable_if_t<sizeof...(_OtherIndexTypes) == _Rank_dynamic
            && (is_same_v<_OtherIndexTypes, index_type> && ...), int> = 0>
        constexpr _Mdspan_extent_type(_OtherIndexTypes... _OtherExtents) noexcept
            : _Dynamic_extents{ _OtherExtents... } {}

        template <class _OtherIndexType, size_t _Size, size_t..._Idx, enable_if_t<_Size == _Rank_dynamic, int> = 0>
        constexpr _Mdspan_extent_type(span<_OtherIndexType, _Size> _Data, index_sequence<_Idx...>) noexcept
            : _Dynamic_extents{ static_cast<index_type>(_STD as_const(_Data[_Idx]))... }
        {}


        template <class... _OtherIndexTypes, enable_if_t<sizeof...(_OtherIndexTypes) == sizeof...(_Extents)
            && sizeof...(_Extents) != _Rank_dynamic && (is_same_v<_OtherIndexTypes, index_type> && ...), int> = 0>
        constexpr _Mdspan_extent_type(_OtherIndexTypes... _OtherExtents) noexcept {
            auto _It = _Dynamic_extents;
            ((_Extents == dynamic_extent ? void(*_It++ = _OtherExtents) : void(_OtherExtents)), ...);
        }

        template <class _OtherIndexType, size_t _Size, size_t..._Idx, enable_if_t<_Size == sizeof...(_Extents)
            && sizeof...(_Extents) != _Rank_dynamic, int> = 0>
        constexpr _Mdspan_extent_type(span<_OtherIndexType, _Size> _Data, index_sequence<_Idx...>) noexcept
            : _Dynamic_extents({ static_cast<index_type>(_STD as_const(_Data[_Dynamic_indexes[_Idx]]))... })
        {}

        constexpr index_type* _Begin_dynamic_extents() noexcept {
            return _Dynamic_extents;
        }

        constexpr const index_type* _Begin_dynamic_extents() const noexcept {
            return _Dynamic_extents;
        }
    };

    template <class _IndexType, size_t... _Extents>
    struct _Mdspan_extent_type<_IndexType, 0, _Extents...> {
        using index_type = _IndexType;

        static constexpr size_t _Static_extents[sizeof...(_Extents)] = { _Extents... };

        constexpr _Mdspan_extent_type() noexcept = default;

        template <class... _IndexTypes, enable_if_t<sizeof...(_IndexTypes) == sizeof...(_Extents), int> = 0>
        constexpr _Mdspan_extent_type(_IndexTypes... /*_OtherExtents*/) noexcept
        {}

        constexpr index_type* _Begin_dynamic_extents() noexcept {
            return nullptr;
        }

        constexpr const index_type* _Begin_dynamic_extents() const noexcept {
            return nullptr;
        }
    };

    template <class _IndexType>
    struct _Mdspan_extent_type<_IndexType, 0> {
        using index_type = _IndexType;

        constexpr index_type* _Begin_dynamic_extents() {
            return nullptr;
        }

        constexpr const index_type* _Begin_dynamic_extents() const noexcept {
            return nullptr;
        }
    };

    template <class _IndexType, size_t... _Extents>
    class extents : private _Mdspan_extent_type<_IndexType, ((_Extents == dynamic_extent) + ... + 0), _Extents...> {
    public:
        using _Mybase = _Mdspan_extent_type<_IndexType, ((_Extents == dynamic_extent) + ... + 0), _Extents...>;
        using index_type = _Mybase::index_type;
        using size_type = make_unsigned_t<index_type>;
        using rank_type = size_t;

        static_assert(_Is_any_of_v<remove_cv_t<_IndexType>, signed char, unsigned char, short, unsigned short, int,
            unsigned int, long, unsigned long, long long, unsigned long long>, "NXXXX [mdspan.extents.overview]/2 requires that extents::size_type be a standard integer type.");

        static_assert(((_Extents == dynamic_extent || _Extents <= (numeric_limits<_IndexType>::max)()) && ...));

        // [mdspan.extents.obs], Observers of the domain multidimensional index space
        _NODISCARD static constexpr rank_type rank() noexcept {
            return sizeof...(_Extents);
        }

        _NODISCARD static constexpr rank_type rank_dynamic() noexcept {
            return ((_Extents == dynamic_extent) + ... + 0);
        }

        _NODISCARD static constexpr size_t static_extent(const rank_type _Idx) noexcept {
            return _Mybase::_Static_extents[_Idx];
        }

        _NODISCARD constexpr size_type extent(const rank_type _Idx) const noexcept {
            if constexpr (rank_dynamic() == 0) {
                return static_cast<size_type>(_Mybase::_Static_extents[_Idx]);
            }
            else if constexpr (rank_dynamic() == rank()) {
                return _Mybase::_Dynamic_extents[_Idx];
            }
            else {
                const auto _Static_extent = _Mybase::_Static_extents[_Idx];
                if (_Static_extent == dynamic_extent) {
                    return _Mybase::_Dynamic_extents[_Mybase::_Dynamic_indexes[_Idx]];
                }
                else {
                    return _Static_extent;
                }
            }
        }

        // [mdspan.extents.cons], Constructors
        constexpr extents() noexcept = default;

        template <class _OtherIndexType, size_t... _OtherExtents,
            enable_if_t<sizeof...(_OtherExtents) == sizeof...(_Extents)
            && ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent
                || _OtherExtents == _Extents)
                && ...),
            int> = 0>
        explicit((((_Extents != dynamic_extent) && (_OtherExtents == dynamic_extent)) || ...)
            || numeric_limits<size_type>::max() < numeric_limits<_OtherIndexType>::max())
            constexpr extents(const extents<_OtherIndexType, _OtherExtents...>& _Other) noexcept {
            auto _Dynamic_it = _Mybase::_Begin_dynamic_extents();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _STL_VERIFY(_Mybase::_Static_extents[_Dim] == dynamic_extent || _Mybase::_Static_extents[_Dim] == _Other.extent(_Dim),
                    "Cannot construct an extent from another extent with different rank sizes.");
                if (_Mybase::_Static_extents[_Dim] == dynamic_extent) {
                    *_Dynamic_it++ = _Other.extent(_Dim);
                }
            }
        }

        template <class... _OtherIndexTypes,
            enable_if_t<(is_convertible_v<_OtherIndexTypes, size_type> && ...)
            && (is_nothrow_constructible_v<size_type, _OtherIndexTypes> && ...)
            && (sizeof...(_OtherIndexTypes) == rank_dynamic() || sizeof...(_OtherIndexTypes) == rank()),
            int> = 0>
        explicit constexpr extents(_OtherIndexTypes... _Exts) noexcept : _Mybase{ static_cast<size_type>(_Exts)... } {}

        template <class _OtherIndexType, size_t _Size,
            enable_if_t<is_convertible_v<const _OtherIndexType&, size_type>
            && is_nothrow_constructible_v<size_type, const _OtherIndexType&>
            && (_Size == rank_dynamic() || _Size == rank()), int > = 0>
        explicit(_Size != rank_dynamic())
            constexpr extents(const array<_OtherIndexType, _Size>& _Exts) noexcept
            : _Mybase{ span{_Exts}, _STD make_index_sequence<rank_dynamic()>{} } {}

        template <class _OtherIndexType, size_t _Size,
            enable_if_t<is_convertible_v<const _OtherIndexType&, size_type>
            && is_nothrow_constructible_v<size_type, const _OtherIndexType&>
            && (_Size == rank_dynamic() || _Size == rank()), int > = 0>
        explicit(_Size != rank_dynamic())
            constexpr extents(span<_OtherIndexType, _Size> _Exts) noexcept
            : _Mybase{ _Exts, _STD make_index_sequence<rank_dynamic()>{} } {}


        // [mdspan.extents.compare], extents comparison operators
        template <class _OtherIndexType, size_t... _OtherExtents>
        _NODISCARD_FRIEND constexpr bool operator==(
            const extents& _Lhs, const extents<_OtherIndexType, _OtherExtents...>& _Rhs) noexcept {
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

        constexpr void _Fill_extents(index_type* _Out) const noexcept {
            auto _Dynamic_it = _Mybase::_Begin_dynamic_extents();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                if (_Mybase::_Static_extents[_Dim] == dynamic_extent) {
                    *_Out++ = *_Dynamic_it++;
                }
                else {
                    *_Out++ = static_cast<index_type>(_Mybase::_Static_extents[_Dim]);
                }
            }
        }
    };

    template <class _IndexType, size_t _Rank>
    using dextents = decltype([]<size_t... _Seq>(const index_sequence<_Seq...>) constexpr {
        return extents<_IndexType, ((void)_Seq, dynamic_extent)...>{};
    }(make_index_sequence<_Rank>{}));

    template <class... _Integrals, enable_if_t<(is_convertible_v<_Integrals, size_t> && ...), int> = 0>
    extents(_Integrals... _Ext) -> extents<size_t, conditional_t<true,
        integral_constant<size_t, dynamic_extent>, _Integrals>::value...>;

    struct layout_left {
        template <class _Extents> class mapping;
    };

    struct layout_right {
        template <class _Extents> class mapping;
    };

    struct layout_stride {
        template <class _Extents> class mapping;
    };

    template <class _Extents>
    class layout_left::mapping {
    public:
        using extents_type = _Extents;
        using index_type = typename _Extents::index_type;
        using size_type = typename _Extents::size_type;
        using rank_type = typename _Extents::rank_type;
        using layout_type = layout_left;

        constexpr mapping() noexcept = default;
        constexpr mapping(const mapping&) noexcept = default;

        constexpr mapping(const _Extents& e) noexcept : _Myext(e) {};

        template <class _OtherExtents, enable_if_t<is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(!is_convertible_v<_OtherExtents, _Extents>) constexpr
            mapping(const mapping<_OtherExtents>& _Other) noexcept
            : _Myext{ _Other.extents() } {};

        template <class _OtherExtents,
        enable_if_t<_Extents::rank() <= 1 && is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(!is_convertible_v<_OtherExtents, _Extents>)
            constexpr mapping(
                const layout_right::mapping<_OtherExtents>& _Other) noexcept : _Myext{ _Other.extents() } {}

        template <class _OtherExtents,
        enable_if_t<is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(_Extents::rank() > 0)
            constexpr mapping(
            const layout_stride::template mapping<_OtherExtents>& _Other)
            : _Myext{ _Other.extents() } {}

        constexpr mapping& operator=(const mapping&) noexcept = default;

        _NODISCARD constexpr _Extents extents() const noexcept {
            return _Myext;
        }

        _NODISCARD constexpr index_type required_span_size() const noexcept {
            size_type _Result = 1;
            for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                _Result *= _Myext.extent(_Dim);
            }

            return _Result;
        }

        template <class... _Indices,
            enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, index_type> && ...)
            && (is_nothrow_constructible_v<index_type, _Indices> && ...),
            int> = 0>
        _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
            return _Index_impl<_Indices...>(static_cast<index_type>(_Idx)..., make_index_sequence<_Extents::rank()>{});
        }

        _NODISCARD static constexpr bool is_always_unique() noexcept {
            return true;
        }
        _NODISCARD static constexpr bool is_always_exhaustive() noexcept {
            return true;
        }
        _NODISCARD static constexpr bool is_always_strided() noexcept {
            return true;
        }

        _NODISCARD constexpr bool is_unique() const noexcept {
            return true;
        }
        _NODISCARD constexpr bool is_exhaustive() const noexcept {
            return true;
        }
        _NODISCARD constexpr bool is_strided() const noexcept {
            return true;
        }

        _NODISCARD constexpr size_type stride(size_t _Rank) const noexcept {
            size_type _Result = 1;
            for (size_t _Dim = 0; _Dim < _Rank; ++_Dim) {
                _Result *= _Myext.extent(_Dim);
            }

            return _Result;
        }

        template <class OtherExtents>
        _NODISCARD friend constexpr bool operator==(
            const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
            return _Lhs.extents() == _Rhs.extents();
        }

    private:
        _Extents _Myext{};

        template <class... _Indices, size_t... _Seq>
        constexpr size_type _Index_impl(_Indices... _Idx, index_sequence<_Seq...>) const noexcept {
            //return _Extents::rank() > 0 ? ((_Idx * stride(_Seq)) + ... + 0) : 0;
            size_type _Stride = 1;
            size_type _Result = 0;
            (((_Result += _Idx * _Stride), (void)(_Stride *= _Myext.extent(_Seq))), ...);
            return _Result;
        }
    };

    template <class _Extents>
    class layout_right::mapping {
    public:
        using extents_type = _Extents;
        using index_type = typename _Extents::index_type;
        using size_type = typename _Extents::size_type;
        using rank_type = typename _Extents::rank_type;
        using layout_type = layout_right;

        constexpr mapping() noexcept = default;
        constexpr mapping(const mapping&) noexcept = default;

        constexpr mapping(const _Extents& e) noexcept : _Myext(e) {};

        template <class _OtherExtents, enable_if_t<is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(!is_convertible_v<_OtherExtents, _Extents>) constexpr
            mapping(const mapping<_OtherExtents>& _Other) noexcept
            : _Myext{ _Other.extents() } {};

        template <class _OtherExtents,
            enable_if_t<_Extents::rank() <= 1 && is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(!is_convertible_v<_OtherExtents, _Extents>)
            constexpr mapping(
                const layout_left::mapping<_OtherExtents>& _Other) noexcept : _Myext{ _Other.extents() } {}

        template <class _OtherExtents,
            enable_if_t<is_constructible_v<_Extents, _OtherExtents>, int> = 0>
        explicit(_Extents::rank() > 0)
            constexpr mapping(
                const layout_stride::template mapping<_OtherExtents>& _Other)
            : _Myext{ _Other.extents() } {}


        constexpr mapping& operator=(const mapping&) noexcept = default;

        _NODISCARD constexpr _Extents extents() const noexcept {
            return _Myext;
        }

        _NODISCARD constexpr size_type required_span_size() const noexcept {
            size_type _Result = 1;
            for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                _Result *= _Myext.extent(_Dim);
            }

            return _Result;
        }

        template <class... _Indices,
            enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, index_type> && ...)
            && (is_nothrow_constructible_v<index_type, _Indices> && ...),
            int> = 0>
        _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
            return _Index_impl<_Indices...>(static_cast<index_type>(_Idx)..., make_index_sequence<_Extents::rank()>{});
        }

        _NODISCARD static constexpr bool is_always_unique() noexcept {
            return true;
        }
        _NODISCARD static constexpr bool is_always_exhaustive() noexcept {
            return true;
        }
        _NODISCARD static constexpr bool is_always_strided() noexcept {
            return true;
        }

        _NODISCARD constexpr bool is_unique() const noexcept {
            return true;
        }
        _NODISCARD constexpr bool is_exhaustive() const noexcept {
            return true;
        }
        _NODISCARD constexpr bool is_strided() const noexcept {
            return true;
        }

        _NODISCARD constexpr size_type stride(size_t _Rank) const noexcept {
            size_type _Result = 1;
            for (size_t _Dim = _Rank + 1; _Dim < _Extents::rank(); ++_Dim) {
                _Result *= _Myext.extent(_Dim);
            }

            return _Result;
        }

        template <class OtherExtents>
        _NODISCARD friend constexpr bool operator==(
            const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
            return _Lhs.extents() == _Rhs.extents();
        }

    private:
        _Extents _Myext{};

        static constexpr size_t _Multiply(size_t _X, size_t _Y) {
            return _X * _Y;
        }

        template <class... _Indices, size_t... _Seq>
        constexpr size_type _Index_impl(_Indices... _Idx, index_sequence<_Seq...>) const noexcept {
            size_type _Accum = 0;
            ((void)(_Accum = _Idx + _Myext.extent(_Seq) * _Accum), ...);
            return _Accum;
        }
    };


    template <class _Extents>
    class layout_stride::mapping {
    public:
        using extents_type = _Extents;
        using index_type = typename _Extents::index_type;
        using size_type = typename _Extents::size_type;
        using rank_type = typename _Extents::rank_type;
        using layout_type = layout_stride;

        constexpr mapping() noexcept = default;
        constexpr mapping(const mapping&) noexcept = default;

        template <class _SizeType, size_t N,
            enable_if_t<is_convertible_v<_SizeType, size_type>&& N == extents_type::rank(), int> = 0>
        constexpr mapping(const _Extents& _E_, const array<_SizeType, N>& _S_) noexcept
            : _Myext{ _E_ }, _Mystrides{ _S_ } {};

        template <class OtherExtents, enable_if_t<is_constructible_v<_Extents, OtherExtents>, int> = 0>
        explicit(!is_convertible_v<OtherExtents, _Extents>) constexpr mapping(
            const mapping<OtherExtents>& _Other) noexcept
            : _Myext{ _Other.extents() }, _Mystrides{ _Other.strides() } {}

        template <class LayoutMapping,
            enable_if_t<
            is_same_v<LayoutMapping,
            typename LayoutMapping::layout_type::template mapping<typename LayoutMapping::
            extents_type>>&& is_constructible_v<_Extents, typename LayoutMapping::extents_type>&& LayoutMapping::is_always_unique()
            && LayoutMapping::is_always_strided(),
            int> = 0>
            explicit(!is_convertible_v<typename LayoutMapping::extents_type, _Extents>) constexpr mapping(
                const LayoutMapping& _Other) noexcept
            : _Myext(_Other.extents()) {
            for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                _Mystrides[_Dim] = _Other.stride(_Dim);
            }
        }

        constexpr mapping& operator=(const mapping&) noexcept = default;

        _NODISCARD constexpr _Extents extents() const noexcept {
            return _Myext;
        }

        _NODISCARD constexpr array<typename size_type, _Extents::rank()> strides() const noexcept {
            return _Mystrides;
        }

        _NODISCARD constexpr size_type required_span_size() const noexcept {
            if (_Extents::rank() > 0 /*&& _Myext._All_positive()*/) {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                    _Result += (_Myext.extent(_Dim) - 1) * _Mystrides[_Dim];
                }

                return _Result;
            }
            else {
                return 1;
            }
        }

        template <class... _Indices,
            enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, index_type> && ...)
            && (is_nothrow_constructible_v<index_type, _Indices> && ...), int> = 0>
            _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
            return _Index_impl<_Indices...>(static_cast<index_type>(_Idx)..., make_index_sequence<_Extents::rank()>{});
        }

        _NODISCARD static constexpr bool is_always_unique() noexcept {
            return true;
        }

        _NODISCARD static constexpr bool is_always_exhaustive() noexcept {
            return false;
        }

        _NODISCARD static constexpr bool is_always_strided() noexcept {
            return true;
        }

        _NODISCARD constexpr bool is_unique() const noexcept {
            return true;
        }

        _NODISCARD constexpr bool is_exhaustive() const noexcept {
            // Look for a permutation of the ranks such that the partial products of the extents equal the strides.
            // The stride of a singleton dimention doesn't matter because its index can only be zero, so they can
            // be ignored.
            index_type _My_extents[_Extents::rank()];
            _Myext._Fill_extents(_My_extents);

            size_t _Singleton_count = 0;
            for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                if (_My_extents[_Dim] == 1) {
                    ++_Singleton_count;
                }
            }

            index_type _Target = 1;
            for (size_t _Dim = _Singleton_count; _Dim < _Extents::rank(); ++_Dim) {
                size_t _Idx = 0;
                for (; _Idx < _Extents::rank(); ++_Idx) {
                    if (_My_extents[_Idx] != 1 && _Mystrides[_Idx] == _Target) {
                        _Target *= _My_extents[_Idx];
                        break;
                    }
                }

                if (_Idx == _Extents::rank()) {
                    return false;
                }
            }

            return true;
        }

        _NODISCARD constexpr bool is_strided() const noexcept {
            return true;
        }

        _NODISCARD constexpr size_type stride(const size_t _Idx) const noexcept {
            return _Mystrides[_Idx];
        }

        template <class OtherExtents>
        _NODISCARD friend constexpr bool operator==(
            const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
            return _Lhs.extents() == _Rhs.extents() && _Lhs.strides() == _Rhs.strides();
        }

    private:
        _Extents _Myext{};
        array<index_type, _Extents::rank()> _Mystrides{};

        template <class... _Indices, size_t... _Seq>
        constexpr size_type _Index_impl(_Indices... _Idx, index_sequence<_Seq...>) const noexcept {
            return ((_Idx * _Mystrides[_Seq]) + ...);
        }
    };

    template <class _ElementType>
    struct default_accessor {
        using offset_policy = default_accessor;
        using element_type = _ElementType;
        using reference = _ElementType&;
        using pointer = _ElementType*;

        constexpr default_accessor() noexcept = default;

        template <class _OtherElementType,
            enable_if_t<
            is_convertible_v<typename default_accessor<_OtherElementType>::element_type(*)[], _ElementType(*)[]>,
            int> = 0>
        constexpr default_accessor(default_accessor<_OtherElementType>) noexcept {}

        _NODISCARD constexpr typename offset_policy::pointer offset(pointer _Ptr, size_t _Idx) const noexcept {
            return _Ptr + _Idx;
        }

        _NODISCARD constexpr reference access(pointer _Ptr, size_t _Idx) const noexcept {
            return _Ptr[_Idx];
        }
    };

    template <class _ElementType, class _Extents, class _LayoutPolicy = layout_right,
        class _AccessorPolicy = default_accessor<_ElementType>>
        class mdspan {
        public:
            // Domain and codomain types
            using extents_type = _Extents;
            using layout_type = _LayoutPolicy;
            using accessor_type = _AccessorPolicy;
            using mapping_type = typename layout_type::template mapping<extents_type>;
            using element_type = _ElementType;
            using value_type = remove_cv_t<element_type>;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using pointer = typename accessor_type::pointer;
            using reference = typename accessor_type::reference;

            _NODISCARD static constexpr size_t rank() {
                return _Extents::rank();
            }
            _NODISCARD static constexpr size_t rank_dynamic() {
                return _Extents::rank_dynamic();
            }
            _NODISCARD static constexpr size_type static_extent(size_t r) {
                return _Extents::static_extent(r);
            }

            // [mdspan.mdspan.cons], mdspan constructors, assignment, and destructor
            constexpr mdspan()
#ifdef __cpp_lib_concepts
                requires(rank_dynamic() == 0)
#endif
            = default;
            constexpr mdspan(const mdspan& rhs) = default;
            constexpr mdspan(mdspan&& rhs) = default;

            template <class... _SizeTypes,
                enable_if_t<
                (is_convertible_v<_SizeTypes, size_type> && ...)
                && is_constructible_v<_Extents,
                _SizeTypes...>&& is_constructible_v<mapping_type, _Extents>&& is_default_constructible_v<accessor_type>,
                int> = 0>
            explicit constexpr mdspan(pointer _Ptr_, _SizeTypes... _Exts) : _Ptr{ _Ptr_ }, _Map{ _Extents{_Exts...} } {}


            template <class _SizeType, size_t _Size,
                enable_if_t<
                is_convertible_v<_SizeType,
                size_type>&& is_constructible_v<_Extents, array<_SizeType, _Size>>&& is_constructible_v<mapping_type, _Extents>&& is_default_constructible_v<accessor_type>,
                int> = 0>
            explicit(_Size != rank_dynamic()) constexpr mdspan(pointer _Ptr_, const array<_SizeType, _Size>& _Exts)
                : _Ptr{ _Ptr_ }, _Map{ _Extents{_Exts} } {}


            template <class _Extents2 = _Extents,
                enable_if_t<is_constructible_v<mapping_type, _Extents2>&& is_default_constructible_v<accessor_type>, int> =
                0>
            constexpr mdspan(pointer _Ptr_, const _Extents& _Ext) : _Ptr{ _Ptr_ }, _Map{ _Ext } {}

            template <class _Accessor = accessor_type, enable_if_t<is_default_constructible_v<_Accessor>, int> = 0>
            constexpr mdspan(pointer _Ptr_, const mapping_type& _Map_) : _Ptr{ _Ptr_ }, _Map{ _Map_ } {}

            constexpr mdspan(pointer _Ptr_, const mapping_type& _Map_, const accessor_type& _Acc_)
                : _Ptr{ _Ptr_ }, _Map{ _Map_ }, _Acc{ _Acc_ } {}

            // template <class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherAccessorPolicy>
            // explicit(see below) constexpr mdspan(
            //    const mdspan<OtherElementType, OtherExtents, OtherLayoutPolicy, OtherAccessorPolicy>& other);

            constexpr mdspan& operator=(const mdspan& rhs) = default;
            constexpr mdspan& operator=(mdspan&& rhs) = default;

            //// [mdspan.mdspan.mapping], mdspan mapping domain multidimensional index to access codomain element
            // TRANSITION operator()
            template <class... _SizeTypes,
                enable_if_t<(is_convertible_v<_SizeTypes, size_type> && ...) && sizeof...(_SizeTypes) == rank(), int> = 0>
            _NODISCARD constexpr reference operator()(_SizeTypes... _Indices) const {
                return _Acc.access(_Ptr, _Map(_Indices...));
            }

            template <class _SizeType, size_t _Size,
                enable_if_t<is_convertible_v<_SizeType, size_type>&& _Size == rank(), int> = 0>
            _NODISCARD constexpr reference operator[](const array<_SizeType, _Size>& _Indices) const {
                return _Index_impl(_Indices, make_index_sequence<_Size>{});
            }

            _NODISCARD constexpr accessor_type accessor() const {
                return _Acc;
            }

            _NODISCARD constexpr _Extents extents() const {
                return _Map.extents();
            }

            _NODISCARD constexpr size_type extent(size_t r) const {
                const auto& _Ext = _Map.extents();
                return _Ext.extent(r);
            }

            _NODISCARD constexpr size_type size() const {
                const auto& _Ext = _Map.extents();
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < rank(); ++_Dim) {
                    _Result *= _Ext.extent(_Dim);
                }
                return _Result;
            }

            //// [mdspan.basic.codomain], mdspan observers of the codomain
            _NODISCARD constexpr pointer data() const {
                return _Ptr;
            }

            _NODISCARD constexpr mapping_type mapping() const {
                return _Map;
            }

            _NODISCARD static constexpr bool is_always_unique() {
                return mapping_type::is_always_unique();
            }

            _NODISCARD static constexpr bool is_always_exhaustive() {
                return mapping_type::is_always_exhaustive();
            }

            _NODISCARD static constexpr bool is_always_strided() {
                return mapping_type::is_always_strided();
            }

            _NODISCARD constexpr bool is_unique() const {
                return _Map.is_unique();
            }

            _NODISCARD constexpr bool is_exhaustive() const {
                return _Map.is_exhaustive();
            }

            _NODISCARD constexpr bool is_strided() const {
                return _Map.is_strided();
            }

            _NODISCARD constexpr size_type stride(size_t r) const {
                return _Map.stride(r);
            }

        private:
            template <class _SizeType, size_t _Size, size_t... _Idx>
            _NODISCARD constexpr reference _Index_impl(
                const array<_SizeType, _Size>& _Indices, index_sequence<_Idx...>) const {
                return _Acc.access(_Ptr, _Map(_Indices[_Idx]...));
            }

            pointer _Ptr{};
            mapping_type _Map;
            accessor_type _Acc;
    };

} // namespace std