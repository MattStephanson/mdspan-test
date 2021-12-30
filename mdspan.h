#pragma once

#include <algorithm>
#include <array>
#include <span>

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

        _NODISCARD static constexpr size_t rank_dynamic() noexcept {
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
        _NODISCARD static constexpr size_t rank() noexcept {
            return sizeof...(_Extents);
        }

        _NODISCARD static constexpr size_type static_extent(const size_t _Idx) noexcept {
            return _Static_extents[_Idx];
        }

        _NODISCARD constexpr size_type extent(const size_t _Idx) const noexcept {
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
        _NODISCARD friend constexpr bool operator==(
            const extents& _Lhs, const extents<_OtherExtents...>& _Rhs) noexcept {
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

        _NODISCARD constexpr bool _All_positive() const noexcept {
            for (size_t _Dim = 0; _Dim < sizeof...(_Extents); ++_Dim) {
                if (_Static_extents[_Dim] != dynamic_extent && _Static_extents[_Dim] <= 0) {
                    return false;
                }
            }

            for (size_t _Dim = 0; _Dim < rank_dynamic(); ++_Dim) {
                if (_Dynamic_extents[_Dim] <= 0) {
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
        template <class _Extents>
        class mapping {
        public:
            using size_type    = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type  = layout_left;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            constexpr mapping(const _Extents& e) noexcept : extents_{e} {};

            template <class OtherExtents, enable_if_t<is_constructible_v<_Extents, OtherExtents>, int> = 0>
            explicit(!std::is_convertible_v<OtherExtents, _Extents>) constexpr mapping(
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

            _NODISCARD constexpr _Extents extents() const noexcept {
                return extents_;
            }

            _NODISCARD constexpr size_type required_span_size() const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class... Indices,
                enable_if_t<sizeof...(Indices) == _Extents::rank() && (is_convertible_v<Indices, size_type> && ...),
                    int> = 0>
            _NODISCARD constexpr size_type operator()(Indices... _Idx) const noexcept {
                return _Index_impl<Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
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
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class OtherExtents>
            _NODISCARD friend constexpr bool operator==(
                const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents();
            }

        private:
            _Extents extents_{}; // exposition only

            template <class... Indices, size_t... I>
            constexpr size_type _Index_impl(Indices... _Idx, index_sequence<I...>) const noexcept {
                return ((_Idx * stride(I)) + ...);
            }
        };
    };

    struct layout_right {
        template <class _Extents>
        class mapping {
        public:
            using size_type    = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type  = layout_right;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            constexpr mapping(const _Extents& e) noexcept : extents_{e} {};

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

            _NODISCARD constexpr _Extents extents() const noexcept {
                return extents_;
            }

            _NODISCARD constexpr size_type required_span_size() const noexcept {
                size_type _Result = 1;
                for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class... Indices,
                enable_if_t<sizeof...(Indices) == _Extents::rank() && (is_convertible_v<Indices, size_type> && ...),
                    int> = 0>
            _NODISCARD constexpr size_type operator()(Indices... _Idx) const noexcept {
                return _Index_impl<Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
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
                    _Result *= extents_.extent(_Dim);
                }

                return _Result;
            }

            template <class OtherExtents>
            _NODISCARD friend constexpr bool operator==(
                const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents();
            }

        private:
            _Extents extents_{}; // exposition only

            template <class... Indices, size_t... I>
            constexpr size_type _Index_impl(Indices... _Idx, index_sequence<I...>) const noexcept {
                return ((_Idx * stride(I)) + ...);
            }
        };
    };


    struct layout_stride {
        template <class _Extents>
        class mapping {
        public:
            using size_type    = typename _Extents::size_type;
            using extents_type = _Extents;
            using layout_type  = layout_stride;

            constexpr mapping() noexcept               = default;
            constexpr mapping(const mapping&) noexcept = default;
            constexpr mapping(mapping&&) noexcept      = default;

            template <class SizeType, size_t N,
                enable_if_t<is_convertible_v<SizeType, size_type> && N == extents_type::rank(), int> = 0>
            constexpr mapping(const _Extents& _E_, const array<SizeType, N>& _S_) noexcept
                : extents_{_E_}, strides_{_S_} {};

            template <class OtherExtents, enable_if_t<is_constructible_v<_Extents, OtherExtents>, int> = 0>
            explicit(!is_convertible_v<OtherExtents, _Extents>) constexpr mapping(
                const mapping<OtherExtents>& _Other) noexcept
                : extents_{_Other.extents()}, strides_{_Other.strides()} {}

            template <class LayoutMapping,
                enable_if_t<
                    is_same_v<LayoutMapping,
                        typename LayoutMapping::layout_type::template mapping<typename LayoutMapping::
                                extents_type>> && is_constructible_v<_Extents, typename LayoutMapping::extents_type> && LayoutMapping::is_always_unique()
                        && LayoutMapping::is_always_strided(),
                    int> = 0>
            explicit(!is_convertible_v<typename LayoutMapping::extents_type, _Extents>) constexpr mapping(
                const LayoutMapping& _Other) noexcept
                : extents_(_Other.extents()) {
                for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                    strides_[_Dim] = _Other.stride(_Dim);
                }
            }

            constexpr mapping& operator=(const mapping&) noexcept = default;
            constexpr mapping& operator=(mapping&&) noexcept = default;

            _NODISCARD constexpr _Extents extents() const noexcept {
                return extents_;
            }

            _NODISCARD constexpr array<typename size_type, _Extents::rank()> strides() const noexcept {
                return strides_;
            }

            _NODISCARD constexpr size_type required_span_size() const noexcept {
                if (_Extents::rank() > 0 && extents_._All_positive()) {
                    size_t _Result = 1;
                    for (size_t _Dim = 0; _Dim < _Extents::rank(); ++_Dim) {
                        _Result += (extents_.extent(_Dim) - 1) * strides_[_Dim];
                    }

                    return _Result;
                } else {
                    return _Extents::rank() == 0 ? 1 : 0;
                }
            }

            template <class... Indices,
                enable_if_t<sizeof...(Indices) == _Extents::rank() && (is_convertible_v<Indices, size_type> && ...),
                    int> = 0>
            _NODISCARD constexpr size_type operator()(Indices... _Idx) const noexcept {
                return _Index_impl<Indices...>(_Idx..., make_index_sequence<_Extents::rank()>{});
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
                struct _StEx {
                    size_t _Stride;
                    size_t _Extent;

                    constexpr bool operator<(const _StEx& _Rhs) const {
                        if (_Stride != _Rhs._Stride) {
                            return _Stride < _Rhs._Stride;
                        } else {
                            return _Extent < _Rhs._Extent;
                        }
                    }
                };

                array<_StEx, _Extents::rank()> _SE;
                for (size_t _I = 0; _I < _Extents::rank(); ++_I) {
                    _SE[_I]._Stride = strides_[_I];
                    _SE[_I]._Extent = extents_.extent(_I);
                }

                _STD sort(_SE.begin(), _SE.end());

                if (_SE[0]._Stride != 1) {
                    return false;
                }

                for (size_t _Dim = 1; _Dim < _Extents::rank(); ++_Dim) {
                    if (_SE[_Dim]._Stride != _SE[_Dim - 1]._Stride * _SE[_Dim - 1]._Extent) {
                        return false;
                    }
                }

                return true;
            }

            _NODISCARD constexpr bool is_strided() const noexcept {
                return true;
            }

            _NODISCARD constexpr size_type stride(const size_t _Idx) const noexcept {
                return strides_[_Idx];
            }

            template <class OtherExtents>
            _NODISCARD friend constexpr bool operator==(
                const mapping& _Lhs, const mapping<OtherExtents>& _Rhs) noexcept {
                return _Lhs.extents() == _Rhs.extents() && _Lhs.strides() == _Rhs.strides();
            }

        private:
            _Extents extents_{}; // exposition only
            array<size_type, _Extents::rank()> strides_{}; // exposition only

            template <class... Indices, size_t... I>
            constexpr size_type _Index_impl(Indices... _Idx, index_sequence<I...>) const noexcept {
                return ((_Idx * strides_[I]) + ...);
            }
        };
    };

    template <class _ElementType>
    struct default_accessor {
        using offset_policy = default_accessor;
        using element_type  = _ElementType;
        using reference     = _ElementType&;
        using pointer       = _ElementType*;

        constexpr default_accessor() noexcept = default;

        template <class _OtherElementType,
            enable_if_t<
                is_convertible_v<typename default_accessor<_OtherElementType>::element_type (*)[], _ElementType (*)[]>,
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
        using extents_type    = _Extents;
        using layout_type     = _LayoutPolicy;
        using accessor_type   = _AccessorPolicy;
        using mapping_type    = typename layout_type::template mapping<extents_type>;
        using element_type    = _ElementType;
        using value_type      = remove_cv_t<element_type>;
        using size_type       = size_t;
        using difference_type = ptrdiff_t;
        using pointer         = typename accessor_type::pointer;
        using reference       = typename accessor_type::reference;

        // static constexpr int rank() {
        //    return _Extents::rank();
        //}
        static constexpr size_t rank_dynamic() {
            return _Extents::rank_dynamic();
        }
        // static constexpr size_type static_extent(size_t r) {
        //    return _Extents::static_extent(r);
        //}

        // [mdspan.mdspan.cons], mdspan constructors, assignment, and destructor
        constexpr mdspan()
#ifdef __cpp_lib_concepts
            requires(rank_dynamic() == 0)
#endif
            = default;
        constexpr mdspan(const mdspan& rhs) = default;
        constexpr mdspan(mdspan&& rhs)      = default;

         template <class... _SizeTypes,
         enable_if_t<(is_convertible_v<_SizeTypes, size_type> && ...)
         && is_constructible_v<_Extents, _SizeTypes...> && is_constructible_v<mapping_type, _Extents>
             && is_default_constructible_v<accessor_type>,int> = 0>
         explicit constexpr mdspan(pointer _Ptr_, _SizeTypes... _Exts)
             : _Ptr{_Ptr_}, _Map{_Extents{_Exts...}} {}

         // template <class SizeType, size_t N>
         // explicit(N != rank_dynamic()) constexpr mdspan(pointer p, const array<SizeType, N>& exts);

         // constexpr mdspan(pointer p, const _Extents& ext);

         // constexpr mdspan(pointer p, const mapping_type& m);

         // constexpr mdspan(pointer p, const mapping_type& m, const accessor_type& a);

         // template <class OtherElementType, class OtherExtents, class OtherLayoutPolicy, class OtherAccessorPolicy>
         // explicit(see below) constexpr mdspan(
         //    const mdspan<OtherElementType, OtherExtents, OtherLayoutPolicy, OtherAccessorPolicy>& other);

         constexpr mdspan& operator=(const mdspan& rhs) = default;
         constexpr mdspan& operator=(mdspan&& rhs) = default;

         //// [mdspan.mdspan.mapping], mdspan mapping domain multidimensional index to access codomain element
         // template <class... SizeTypes>
         // constexpr reference operator[](SizeTypes... indices) const;
         // template <class SizeType, size_t N>
         // constexpr reference operator[](const array<SizeType, N>& indices) const;

         // constexpr accessor_type accessor() const {
         //    return acc_;
         //}

         _NODISCARD constexpr _Extents extents() const {
             return _Map.extents();
         }
         // constexpr size_type extent(size_t r) const {
         //    return extents().extent(r);
         //}
         // constexpr size_type size() const;

         //// [mdspan.basic.codomain], mdspan observers of the codomain
         _NODISCARD constexpr pointer data() const {
             return _Ptr;
         }
         // constexpr mapping_type mapping() const {
         //    return map_;
         //}

         // static constexpr bool is_always_unique() {
         //    return mapping_type::is_always_unique();
         //}
         // static constexpr bool is_always_contiguous() {
         //    return mapping_type::is_always_contiguous();
         //}
         // static constexpr bool is_always_strided() {
         //    return mapping_type::is_always_strided();
         //}

         // constexpr bool is_unique() const {
         //    return map_.is_unique();
         //}
         // constexpr bool is_contiguous() const {
         //    return map_.is_contiguous();
         //}
         // constexpr bool is_strided() const {
         //    return map_.is_strided();
         //}
         // constexpr size_type stride(size_t r) const {
         //    return map_.stride(r);
         //}

     private:
         accessor_type acc_; // exposition only
         mapping_type _Map; // exposition only
         pointer _Ptr{}; // exposition only
    };
} // namespace std