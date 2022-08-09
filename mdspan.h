// Copyright(c) Matt Stephanson.
// SPDX - License - Identifier: Apache - 2.0 WITH LLVM - exception

#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <tuple>

namespace std {
    template <class _SizeType, size_t _Rank_dynamic>
    struct _Extent_dyn_type {
        using size_type = _SizeType;
        size_type _Dynamic_extents[_Rank_dynamic] = {};

        constexpr size_type* _Begin() {
            return _Dynamic_extents;
        }
    };

    template <class _SizeType>
    struct _Extent_dyn_type<_SizeType, 0> {
        using size_type = _SizeType;

        constexpr size_type* _Begin() {
            return nullptr;
        }
    };

    template <class _SizeType, size_t... _Extents>
    class extents : private _Extent_dyn_type<_SizeType, ((_Extents == dynamic_extent) + ... + 0)> {
    public:
        using _Mybase = _Extent_dyn_type<_SizeType, ((_Extents == dynamic_extent) + ... + 0)>;
        using size_type = _Mybase::size_type;
        using rank_type = size_t;

        // [mdspan.extents.cons], Constructors and assignment
        constexpr extents() noexcept = default;
        constexpr extents(const extents&) noexcept = default;
        constexpr extents& operator=(const extents&) noexcept = default;
        constexpr extents(extents&&) noexcept = default;
        constexpr extents& operator=(extents&&) noexcept = default;

        _NODISCARD static constexpr size_t rank() noexcept {
            return sizeof...(_Extents);
        }

        _NODISCARD static constexpr size_t rank_dynamic() noexcept {
            return ((_Extents == dynamic_extent) + ... + 0);
        }

        template <class _OtherSizeType, size_t... _OtherExtents,
            enable_if_t<sizeof...(_OtherExtents) == sizeof...(_Extents)
            && ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent
                || _OtherExtents == _Extents)
                && ...),
            int> = 0>
        // explicit((((_Extents != dynamic_extent) && (_OtherExtents == dynamic_extent)) || ...))
        constexpr extents(const extents<_OtherSizeType, _OtherExtents...>& _Other) noexcept {
            auto _Dynamic_it = _Mybase::_Begin();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _STL_VERIFY(_Static_extents[_Dim] == dynamic_extent || _Static_extents[_Dim] == _Other.extent(_Dim),
                    "Cannot construct an extent from another extent with different rank sizes.");
                if (_Static_extents[_Dim] == dynamic_extent) {
                    *_Dynamic_it++ = _Other.extent(_Dim);
                }
            }
        }

        template <class _OtherSizeType, size_t... _OtherExtents,
            enable_if_t<sizeof...(_OtherExtents) == extents::rank()
            && ((_OtherExtents == dynamic_extent || _Extents == dynamic_extent
                || _OtherExtents == _Extents)
                && ...),
            int> = 0>
        constexpr extents& operator=(const extents<_OtherSizeType, _OtherExtents...>& _Other) noexcept {
            auto _Dynamic_it = _Mybase::_Begin();
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                _STL_VERIFY(_Static_extents[_Dim] == dynamic_extent || _Static_extents[_Dim] == _Other.extent(_Dim),
                    "Cannot assign an extent to another extent with different rank sizes.");
                if (_Static_extents[_Dim] == dynamic_extent) {
                    *_Dynamic_it++ = _Other.extent(_Dim);
                }
            }

            return *this;
        }

        template <class... _SizeTypes,
            enable_if_t<((is_convertible_v<_SizeTypes, size_type>) &&...) && (sizeof...(_SizeTypes) == rank_dynamic()),
            int> = 0>
        explicit constexpr extents(_SizeTypes... _Dynamic) noexcept : _Mybase{ static_cast<size_type>(_Dynamic)... } {}

        template <class _SizeType, size_t _Size,
            enable_if_t<is_convertible_v<_SizeType, size_type>&& _Size == extents::rank()
            && extents::rank_dynamic() < extents::rank(),
            int > = 0>
            // explicit(_Size != extents::rank_dynamic())
            constexpr extents(const array<_SizeType, _Size>& _Dynamic) noexcept {
            auto _It = _Mybase::_Begin();

            size_t _Dim = 0;
            ((_Extents == dynamic_extent ? void(*_It++ = _Dynamic[_Dim++]) : void(++_Dim)), ...);

            // for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
            //    if (_Static_extents[_Dim] == dynamic_extent) {
            //        *_It++ = _Dynamic[_Dim];
            //    }
            //}
        }

        template <class _SizeType, size_t _Size,
            enable_if_t<is_convertible_v<_SizeType, size_t>&& _Size == extents::rank_dynamic(), int> = 0>
        // explicit(_Size != extents::rank_dynamic())
        constexpr extents(const array<_SizeType, _Size>& _Dynamic) noexcept {
            auto _It = _Mybase::_Begin();
            for (const auto _Ext : _Dynamic) {
                *_It++ = _Ext;
            }
        }

        // [mdspan.extents.obs], Observers of the domain multidimensional index space
        _NODISCARD static constexpr size_t static_extent(const rank_type _Idx) noexcept {
            return _Static_extents[_Idx];
        }

        _NODISCARD constexpr size_type extent(const rank_type _Idx) const noexcept {
            if constexpr (rank_dynamic() == 0) {
                return _Static_extents[_Idx];
            }
            else if constexpr (rank_dynamic() == rank()) {
                return _Mybase::_Dynamic_extents[_Idx];
            }
            else {
                const auto _Static_extent = _Static_extents[_Idx];
                if (_Static_extent == dynamic_extent) {
                    return _Mybase::_Dynamic_extents[_Dynamic_index(_Idx)];
                }
                else {
                    return _Static_extent;
                }
            }
        }

        // [mdspan.extents.compare], extents comparison operators
        template <class _OtherSizeType, size_t... _OtherExtents>
        _NODISCARD friend constexpr bool operator==(
            const extents& _Lhs, const extents<_OtherSizeType, _OtherExtents...>& _Rhs) noexcept {
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
            size_t _Result = 0;
            size_t _Dim = 0;

            ((void(_Result += (_Dim < _Idx&& _Extents == dynamic_extent)), void(++_Dim)), ...);

            return _Result;
        }

        static constexpr array<size_t, rank()> _Static_extents = { _Extents... };
    };

    template <class _SizeType, size_t _Rank>
    using dextents = decltype([]<size_t... _Seq>(const index_sequence<_Seq...>) constexpr {
        return extents<_SizeType, ((void)_Seq, dynamic_extent)...>{};
    }(make_index_sequence<_Rank>{}));

    //template <class... _Integrals>
    //explicit extents(_Integrals...) -> extents<((void) _Integrals{0}, dynamic_extent)...>;

    struct layout_left {
        template <class _Extents>
        class mapping {
        public:
            using size_type = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type = layout_left;

            constexpr mapping() noexcept = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept = default;

            constexpr mapping(const _Extents& e) noexcept : _Myext{ e } {};

            template <class OtherExtents, enable_if_t<is_constructible_v<_Extents, OtherExtents>, int> = 0>
            explicit(!std::is_convertible_v<OtherExtents, _Extents>) constexpr mapping(
                const mapping<OtherExtents>& _Other) noexcept
                : _Myext{ _Other.extents() } {};

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //    const layout_right::template mapping<OtherExtents>& _Other) noexcept requires(OtherExtents::rank() <=
            //    1) : _Myext{_Other.extents()} {}

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(OtherExtents::rank() > 0) constexpr mapping(
            //    const layout_stride::template mapping<OtherExtents>& other)
            //    : _Myext{_Other.extents()} {}

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

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
                enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, size_type> && ...),
                int> = 0>
            _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
                return _Index_impl<_Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
            }

            _NODISCARD static constexpr bool is_always_unique() noexcept {
                return true;
            }
            _NODISCARD static constexpr bool is_always_contiguous() noexcept {
                return true;
            }
            _NODISCARD static constexpr bool is_always_strided() noexcept {
                return true;
            }

            _NODISCARD constexpr bool is_unique() const noexcept {
                return true;
            }
            _NODISCARD constexpr bool is_contiguous() const noexcept {
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
                return _Extents::rank() > 0 ? ((_Idx * stride(_Seq)) + ... + 0) : 0;
                //size_type _Stride = 1;
                //size_type _Result = 0;
                //(((_Result += _Idx * _Stride), (void) (_Stride *= _Myext.extent(_Seq))), ...);
                //return _Result;
            }
        };
    };

    struct layout_right {
        template <class _Extents>
        class mapping {
        public:
            using size_type = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type = layout_right;

            constexpr mapping() noexcept = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept = default;

            constexpr mapping(const _Extents& e) noexcept : _Myext{ e } {};

            //        template <class OtherExtents, enable_if_t<is_constructible_v<Extents, OtherExtents>, int> = 0>
            //        explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //            const mapping<OtherExtents>& _Other) noexcept
            //            : _Myext{_Other.extents()} {};

            // vvv COPY PASTA vvv

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(!std::is_convertible_v<OtherExtents, Extents>) constexpr mapping(
            //    const layout_right::template mapping<OtherExtents>& _Other) noexcept
            //        requires(OtherExtents::rank() <=
            //    1) : _Myext{_Other.extents()} {}

            // template <class OtherExtents, enable_if<is_constructible_v<Extents, OtherExtents>, int> = 0>
            // explicit(OtherExtents::rank() > 0) constexpr mapping(
            //    const layout_stride::template mapping<OtherExtents>& other)
            //    : _Myext{_Other.extents()} {}

            // ^^^ COPY PASTA ^^^

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

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
                enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, size_type> && ...),
                int> = 0>
            _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
                return _Index_impl<_Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
            }

            _NODISCARD static constexpr bool is_always_unique() noexcept {
                return true;
            }
            _NODISCARD static constexpr bool is_always_contiguous() noexcept {
                return true;
            }
            _NODISCARD static constexpr bool is_always_strided() noexcept {
                return true;
            }

            _NODISCARD constexpr bool is_unique() const noexcept {
                return true;
            }
            _NODISCARD constexpr bool is_contiguous() const noexcept {
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
    };


    struct layout_stride {
        template <class _Extents>
        class mapping {
        public:
            using size_type = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type = layout_stride;

            constexpr mapping() noexcept = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept = default;

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
            constexpr mapping& operator=(mapping&&) noexcept = default;

            _NODISCARD constexpr _Extents extents() const noexcept {
                return _Myext;
            }

            _NODISCARD constexpr array<typename size_type, _Extents::rank()> strides() const noexcept {
                return _Mystrides;
            }

            _NODISCARD constexpr size_type required_span_size() const noexcept {
                if (_Extents::rank() > 0 /*&& _Myext._All_positive()*/) {
                    size_t _Result = 1;
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
                enable_if_t<sizeof...(_Indices) == _Extents::rank() && (is_convertible_v<_Indices, size_type> && ...),
                int> = 0>
            _NODISCARD constexpr size_type operator()(_Indices... _Idx) const noexcept {
                return _Index_impl<_Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
            }

            _NODISCARD static constexpr bool is_always_unique() noexcept {
                return true;
            }

            _NODISCARD static constexpr bool is_always_contiguous() noexcept {
                return false;
            }

            _NODISCARD static constexpr bool is_always_strided() noexcept {
                return true;
            }

            _NODISCARD constexpr bool is_unique() const noexcept {
                return true;
            }

            _NODISCARD constexpr bool is_contiguous() const noexcept {
                array<pair<size_type, size_type>, _Extents::rank()> _SortedStrides;
                for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                    _SortedStrides[_Dim].first = _Mystrides[_Dim];
                    _SortedStrides[_Dim].second = _Myext.extent(_Dim);
                }

                _STD sort(_SortedStrides.begin(), _SortedStrides.end());

                if (_Extents::rank() > 0 && _SortedStrides[0].first != 1) {
                    return false;
                }

                for (size_t _Dim = 1; _Dim < _Extents::rank(); ++_Dim) {
                    if (_SortedStrides[_Dim].first
                        != _SortedStrides[_Dim - 1].first * _SortedStrides[_Dim - 1].second) {
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
            array<size_type, _Extents::rank()> _Mystrides{};

            template <class... _Indices, size_t... _Seq>
            constexpr size_type _Index_impl(_Indices... _Idx, index_sequence<_Seq...>) const noexcept {
                return ((_Idx * _Mystrides[_Seq]) + ...);
            }
        };
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

            static constexpr size_t rank() {
                return _Extents::rank();
            }
            static constexpr size_t rank_dynamic() {
                return _Extents::rank_dynamic();
            }
            static constexpr size_type static_extent(size_t r) {
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

            _NODISCARD static constexpr bool is_always_contiguous() {
                return mapping_type::is_always_contiguous();
            }

            _NODISCARD static constexpr bool is_always_strided() {
                return mapping_type::is_always_strided();
            }

            _NODISCARD constexpr bool is_unique() const {
                return _Map.is_unique();
            }

            _NODISCARD constexpr bool is_contiguous() const {
                return _Map.is_contiguous();
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