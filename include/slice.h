/*
 * Authors: Manu Evans
 * Email:   turkeyman@gmail.com
 * License: BSD, go for broke!
 *
 * What is:
 * Slice is a reference type like a C pointer with associated length.
 * Slice is not a container, does not allocate and store memory. It can be used to access or iterate data.
 * Provides std::vector-like functionality, along with a range of additional functions with greater convenience.
 * Slice is convenient for use as function arguments, or local working data.
 */

#pragma once

#include <functional>
#include <initializer_list>

#if !defined(NO_STL)
# include <vector>
# include <string>
#endif

#if !defined(SLICE_ASSERT)
# include <assert.h>
# define SLICE_ASSERT(condition) assert(condition)
#endif

namespace beautifulcode
{
	namespace detail
	{
		template <typename T> struct SliceElementType		{ using Ty = T; };
		template <> struct SliceElementType<void>			{ using Ty = uint8_t; };
		template <> struct SliceElementType<const void>		{ using Ty = const uint8_t; };

		template <typename T> struct IsSomeChar				{ enum { value = false }; };
		template <> struct IsSomeChar<char>					{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<const char>			{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<unsigned char>		{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<const unsigned char>	{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<char16_t>				{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<const char16_t>		{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<char32_t>				{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<const char32_t>		{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<wchar_t>				{ enum { value = true }; using type = int; };
		template <> struct IsSomeChar<const wchar_t>		{ enum { value = true }; using type = int; };
	}

	template <typename T, bool IsString = detail::IsSomeChar<T>::value>
	struct Slice
	{
	public:
		using value_type = typename detail::SliceElementType<T>::Ty;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using iterator = value_type*;
		using const_iterator = const value_type*;

		size_t length;
		T *ptr;

		constexpr Slice() noexcept;
		inline Slice(nullptr_t) noexcept; // TODO: constexpr fails on VS2017 due to compiler bug!
		constexpr Slice(T* ptr, size_t length) noexcept;
		template <typename U, bool S> constexpr Slice(Slice<U, S> slice) noexcept;
		template <typename U, size_t N> constexpr Slice(U(&arr)[N]) noexcept;
		explicit Slice(std::initializer_list<value_type> list) noexcept;
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Slice(const std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> Slice(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif

		template <typename U, bool S> Slice<T, IsString>& operator=(Slice<U, S> slice) noexcept;

		constexpr reference at(size_t i) const;
		constexpr reference operator[](size_t i) const noexcept;

		constexpr Slice<T> slice(size_t first, size_t last) const noexcept;

		constexpr T *data() const noexcept		{ return ptr; }
		constexpr size_t size() const noexcept	{ return length; }
		constexpr bool empty() const noexcept	{ return length == 0; }
		void clear() noexcept					{ ptr = nullptr; length = 0; }

		explicit constexpr operator bool() const noexcept { return length != 0; }

		constexpr bool operator==(Slice<const T> slice) const noexcept { return ptr == slice.ptr && length == slice.length; }
		constexpr bool operator!=(Slice<const T> slice) const noexcept { return ptr != slice.ptr || length != slice.length; }

		bool eq(Slice<const T> slice) const noexcept;
		ptrdiff_t cmp(Slice<const T> slice) const noexcept;

		bool begins_with(Slice<const T> slice) const noexcept;
		bool ends_with(Slice<const T> slice) const noexcept;

		iterator begin() const noexcept { return iterator(&ptr[0]); }
		iterator end() const noexcept { return iterator(&ptr[length]); }
		iterator cbegin() const noexcept { return const_iterator(&ptr[0]); }
		iterator cend() const noexcept { return const_iterator(&ptr[length]); }

		constexpr reference front() const noexcept				{ return ((value_type*)ptr)[0]; }
		constexpr Slice<value_type> front(size_t n) const noexcept	{ return slice(0, n); }
		constexpr reference back() const noexcept				{ return ((value_type*)ptr)[length - 1]; }
		constexpr Slice<value_type> back(size_t n) const noexcept	{ return slice(length - n, length); }

		reference pop_front() noexcept;
		Slice<value_type> pop_front(size_t n) noexcept;
		reference pop_back() noexcept;
		Slice<value_type> pop_back(size_t n) noexcept;

		constexpr Slice<T> drop_front(size_t n) const noexcept	{ return slice(n, length); }
		constexpr Slice<T> drop_back(size_t n) const noexcept	{ return slice(0, length - n); }

		bool contains(const_reference c, size_t *index = nullptr) const noexcept;
		bool contains(Slice<const T> s, size_t *index = nullptr) const noexcept;

		size_t find_first(const_reference c) const noexcept;
		size_t find_last(const_reference c) const noexcept;

		size_t find_first(Slice<const T> s) const noexcept;
		size_t find_last(Slice<const T> s) const noexcept;

		Slice<T> get_left_at_first(const_reference c, bool inclusive = false) const noexcept;
		Slice<T> get_left_at_last(const_reference c, bool inclusive = false) const noexcept;
		Slice<T> get_right_at_first(const_reference c, bool inclusive = true) const noexcept;
		Slice<T> get_right_at_last(const_reference c, bool inclusive = true) const noexcept;

		Slice<T> get_left_at_first(Slice<const T> s, bool inclusive = false) const noexcept;
		Slice<T> get_left_at_last(Slice<const T> s, bool inclusive = false) const noexcept;
		Slice<T> get_right_at_first(Slice<const T> s, bool inclusive = true) const noexcept;
		Slice<T> get_right_at_last(Slice<const T> s, bool inclusive = true) const noexcept;

		T *search(std::function<bool(const_reference e)> predFunc) const noexcept;

		ptrdiff_t index_of_element(const T *c) const noexcept;

		template <typename U>
		size_t copy_to(Slice<U> dest) const noexcept;

		template <bool SkipEmptyTokens = false>
		Slice<T> pop_token(Slice<const T> delimiters) noexcept;

		template <bool SkipEmptyTokens = false>
		Slice<Slice<T>> tokenise(Slice<Slice<T>> tokens, Slice<const T> delimiters) noexcept;
		template <bool SkipEmptyTokens = false>
		size_t tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<const T> delimiters) noexcept;
	};

	// specialisation for strings
	template <typename C>
	struct Slice<C, true> : Slice<C, false>
	{
		static_assert(detail::IsSomeChar<C>::value, "Template argument C is not a character type!");

		// TODO: why aren't these inherited?
		using value_type = typename Slice<C, false>::value_type;
		using reference = typename Slice<C, false>::reference;
		using const_reference = typename Slice<C, false>::const_reference;
		using size_type = typename Slice<C, false>::size_type;
		using difference_type = typename Slice<C, false>::difference_type;
		using iterator = typename Slice<C, false>::iterator;
		using const_iterator = typename Slice<C, false>::const_iterator;

		constexpr Slice() noexcept;
		constexpr Slice(C* ptr, size_t length) noexcept;
		template <typename U, bool S> constexpr Slice(Slice<U, S> slice) noexcept;
		Slice(C *c_str) noexcept;
//		template <size_t N> constexpr Slice(C(&str_literal)[N]) noexcept;

#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Slice(const std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> Slice(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif

		size_t num_chars() const noexcept;
		char32_t front_char() const noexcept;
		char32_t back_char() const noexcept;
		char32_t pop_front_char() noexcept;
		char32_t pop_back_char() noexcept;

		bool eq_ic(Slice<const C> str) const noexcept;
		bool begins_with_ic(Slice<const C> str) const noexcept;
		bool ends_with_ic(Slice<const C> str) const noexcept;
		ptrdiff_t cmp_ic(Slice<const C> str) const noexcept;

		C* to_cstring(C *buffer, size_t bufferLen) const noexcept;
//		CString<C> to_stringz() const noexcept;

		size_t find_first_ic(C s) const noexcept;
		size_t find_last_ic(C s) const noexcept;

		size_t find_first_ic(Slice<const C> s) const noexcept;
		size_t find_last_ic(Slice<const C> s) const noexcept;

		Slice<C> get_left_at_first_ic(C s, bool inclusive = false) const noexcept;
		Slice<C> get_left_at_last_ic(C s, bool inclusive = false) const noexcept;
		Slice<C> get_right_at_first_ic(C s, bool inclusive = true) const noexcept;
		Slice<C> get_right_at_last_ic(C s, bool inclusive = true) const noexcept;

		Slice<C> get_left_at_first_ic(Slice<const C> s, bool inclusive = false) const noexcept;
		Slice<C> get_left_at_last_ic(Slice<const C> s, bool inclusive = false) const noexcept;
		Slice<C> get_right_at_first_ic(Slice<const C> s, bool inclusive = true) const noexcept;
		Slice<C> get_right_at_last_ic(Slice<const C> s, bool inclusive = true) const noexcept;

		template<bool Front = true, bool Back = true>
		Slice<C> trim() const noexcept;

		template<bool SkipEmptyTokens = true>
		Slice<C> pop_token(Slice<const C> delimiters = " \t\n\r") noexcept															{ return ((Slice<const C, false>*)this)->pop_token<SkipEmptyTokens>(delimiters); }
		template<bool SkipEmptyTokens = true>
		Slice<Slice<C>> tokenise(Slice<Slice<C>> tokens, Slice<const C> delimiters = " \t\n\r") noexcept							{ return ((Slice<const C, false>*)this)->tokenise<SkipEmptyTokens>(tokens, delimiters); }
		template<bool SkipEmptyTokens = true>
		size_t tokenise(std::function<void(Slice<C> token, size_t index)> onToken, Slice<const C> delimiters = " \t\n\r") noexcept	{ return ((Slice<const C, false>*)this)->tokenise<SkipEmptyTokens>(onToken, delimiters); }

		int64_t parse_int(bool detectBase, int base = 10) const noexcept;
		template <int base = 10>
		int64_t parse_int() const noexcept { return parse_int(false, base); }
		double parse_float() const noexcept;

		uint32_t hash(uint32_t hash = 0x811C9DC5) const noexcept;
	};

	// define string types
	using String = Slice<const char>;
	using WString = Slice<const char16_t>;
	using DString = Slice<const char32_t>;

	template <typename T, bool S>
	constexpr bool operator==(nullptr_t, Slice<T, S> slice) noexcept { return slice == nullptr; }
	template <typename T, bool S>
	constexpr bool operator!=(nullptr_t, Slice<T, S> slice) noexcept { return slice == nullptr; }

	// cast from a Slice<T> to a Slice<U> where T and U are compatible types.
	// It's typical use it for casting from a Slice<void>.
	template <typename Dest, typename Src>
	constexpr Dest slice_static_cast(Src src) noexcept;

	template <typename Dest, typename Src>
	constexpr Dest slice_reinterpret_cast(Src src) noexcept;

	template <typename Dest, typename Src>
	constexpr Dest slice_const_cast(Src src) noexcept;


	// -------------------------------------------------------------------------------------------------
	// Implementation follows:
	//

	template <typename T, bool S>
	constexpr Slice<T, S>::Slice() noexcept
		: length(0), ptr(nullptr) {}

	template <typename T, bool S>
	inline Slice<T, S>::Slice(nullptr_t) noexcept
		: length(0), ptr(nullptr) {}

	template <typename T, bool S>
	constexpr Slice<T, S>::Slice(T* ptr, size_t length) noexcept
		: length(length), ptr(ptr) {}

	template <typename T, bool S>
	template <typename U, bool S2>
	constexpr Slice<T, S>::Slice(Slice<U, S2> slice) noexcept
		: length(slice.length), ptr(slice.ptr) {}

	template <typename T, bool S>
	template <typename U, size_t N>
	constexpr Slice<T, S>::Slice(U(&arr)[N]) noexcept
		: length(N), ptr(arr) {}

	template <typename T, bool S>
	inline Slice<T, S>::Slice(std::initializer_list<value_type> list) noexcept
		: length(list.size()), ptr(list.begin()) {}

#if !defined(NO_STL)
	template <typename T, bool S>
	template <class _Ty, class _Alloc>
	inline Slice<T, S>::Slice(const std::vector<_Ty, _Alloc> &vec) noexcept
		: length(vec.length()), ptr(vec.data()) {}

	template <typename T, bool S>
	template <class _Elem, class _Traits, class _Alloc>
	inline Slice<T, S>::Slice(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
		: length(str.length()), ptr(str.data()) {}
#endif

	template <typename T, bool S>
	template <typename U, bool S2>
	inline Slice<T, S> &Slice<T, S>::operator=(Slice<U, S2> slice) noexcept
	{
		length = slice.length;
		ptr = slice.ptr;
		return *this;
	}

	template <typename T, bool S>
	constexpr typename Slice<T, S>::reference Slice<T, S>::at(size_t i) const
	{
		if (i >= length)
			throw std::out_of_range("Index out of bounds");
		return ((value_type*)ptr)[i];
	}

	template <typename T, bool S>
	constexpr typename Slice<T, S>::reference Slice<T, S>::operator[](size_t i) const noexcept
	{
		SLICE_ASSERT(i < length);
		return ((value_type*)ptr)[i];
	}

	template <typename T, bool S>
	constexpr Slice<T> Slice<T, S>::slice(size_t first, size_t last) const noexcept
	{
		SLICE_ASSERT(first <= last && last <= length);
		return{ (value_type*)ptr + first, last - first };
	}

	template <typename T, bool S>
	bool Slice<T, S>::eq(Slice<const T> slice) const noexcept
	{
		if (length != slice.length)
			return false;
		for (size_t i = 0; i < length; ++i)
			if (((value_type*)ptr)[i] != ((value_type*)slice.ptr)[i])
				return false;
		return true;
	}

	template <typename T, bool S>
	inline ptrdiff_t Slice<T, S>::cmp(Slice<const T> slice) const noexcept
	{
		size_t len = length < slice.length ? length : slice.length;
		for (size_t i = 0; i < len; ++i)
		{
			if (((value_type*)ptr)[i] != ((value_type*)slice.ptr)[i])
				return ((value_type*)ptr)[i] < ((value_type*)slice.ptr)[i] ? -1 : 1;
		}
		return length - slice.length;
	}

	template <typename T, bool S>
	inline bool Slice<T, S>::begins_with(Slice<const T> slice) const noexcept
	{
		if (length < slice.length)
			return false;
		return Slice<T>(0, slice.length).eq(slice);
	}
	template <typename T, bool S>
	inline bool Slice<T, S>::ends_with(Slice<const T> slice) const noexcept
	{
		if (length < slice.length)
			return false;
		return Slice<T>(length - slice.length, length).eq(slice);
	}

	template <typename T, bool S>
	inline typename Slice<T, S>::reference Slice<T, S>::pop_front() noexcept
	{
		SLICE_ASSERT(length > 0);
		++ptr;
		--length;
		return ((value_type*)ptr)[-1];
	}
	template <typename T, bool S>
	inline Slice<typename detail::SliceElementType<T>::Ty> Slice<T, S>::pop_front(size_t n) noexcept
	{
		SLICE_ASSERT(length >= n);
		ptr += n;
		length -= n;
		return{ ptr - n, n };
	}
	template <typename T, bool S>
	inline typename Slice<T, S>::reference Slice<T, S>::pop_back() noexcept
	{
		SLICE_ASSERT(length > 0);
		--length;
		return ((value_type*)ptr)[length];
	}
	template <typename T, bool S>
	inline Slice<typename detail::SliceElementType<T>::Ty> Slice<T, S>::pop_back(size_t n) noexcept
	{
		SLICE_ASSERT(length >= n);
		length -= n;
		return{ ptr + length, n };
	}

	template <typename T, bool S>
	inline bool Slice<T, S>::contains(const typename Slice<T, S>::value_type &c, size_t *index) const noexcept
	{
		size_t i = find_first(c);
		if (index)
			*index = i;
		return i != length;
	}

	template <typename T, bool S>
	inline bool Slice<T, S>::contains(Slice<const T> s, size_t *index) const noexcept
	{
		size_t i = find_first(s);
		if (index)
			*index = i;
		return i != length;
	}

	namespace detail
	{
		// hack to handle slices of slices
		template <typename T> struct FindImpl								{ static inline bool eq(const T &a, const T &b) noexcept { return a == b; } };
		template <typename U, bool S> struct FindImpl<Slice<U, S>>			{ static inline bool eq(const Slice<U, S> &a, const Slice<U, S> &b) noexcept { return a.eq(b); } };
		template <typename U, bool S> struct FindImpl<const Slice<U, S>>	{ static inline bool eq(const Slice<U, S> &a, const Slice<U, S> &b) noexcept { return a.eq(b); } };
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_first(const typename Slice<T, S>::value_type &c) const noexcept
	{
		size_t offset = 0;
		while (offset < length && !detail::FindImpl<typename Slice<T, S>::value_type>::eq(ptr[offset], c))
			++offset;
		return offset;
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_last(const typename Slice<T, S>::value_type &c) const noexcept
	{
		ptrdiff_t last = length - 1;
		while (last >= 0 && !detail::FindImpl<typename Slice<T, S>::value_type>::eq(ptr[last], c))
			--last;
		return last < 0 ? length : last;
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_first(Slice<const T> s) const noexcept
	{
		if (s.empty())
			return 0;
		ptrdiff_t len = length - s.length;
		for (ptrdiff_t i = 0; i < len; ++i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T, S>::value_type>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_last(Slice<const T> s) const noexcept
	{
		if (s.empty())
			return length;
		for (ptrdiff_t i = length - s.length; i >= 0; --i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T, S>::value_type>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}

	template <typename T, bool S>
	inline T* Slice<T, S>::search(std::function<bool(const_reference e)> pred) const noexcept
	{
		for (size_t i = 0; i < length; ++i)
		{
			if (pred(ptr[i]))
				return ptr + i;
		}
		return nullptr;
	}

	template <typename T, bool S>
	inline ptrdiff_t Slice<T, S>::index_of_element(const T *c) const noexcept
	{
		if (c >= ptr && c < ptr + length)
			return c - ptr;
		return -1;
	}

	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_left_at_first(const typename Slice<T, S>::value_type &c, bool inclusive) const noexcept
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return{ this->ptr, offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_left_at_last(const typename Slice<T, S>::value_type &c, bool inclusive) const noexcept
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return{ this->ptr, offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_first(const typename Slice<T, S>::value_type &c, bool inclusive) const noexcept
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return{ this->ptr + offset, length - offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_last(const typename Slice<T, S>::value_type &c, bool inclusive) const noexcept
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return{ this->ptr + offset, length - offset };
	}

	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_left_at_first(Slice<const T> s, bool inclusive) const noexcept
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_left_at_last(Slice<const T> s, bool inclusive) const noexcept
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_first(Slice<const T> s, bool inclusive) const noexcept
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return{ this->ptr + offset, length - offset };
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_last(Slice<const T> s, bool inclusive) const noexcept
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return{ this->ptr + offset, length - offset };
	}

	template <typename T, bool S>
	template <typename U>
	inline size_t Slice<T, S>::copy_to(Slice<U> dest) const noexcept
	{
		SLICE_ASSERT(dest.length >= length);
		for (size_t i = 0; i < length; ++i)
			dest.ptr[i] = ptr[i];
		return length;
	}

	template <typename T, bool S>
	template <bool SkipEmptyTokens>
	inline Slice<T> Slice<T, S>::pop_token(Slice<const T> delimiters) noexcept
	{
		size_t offset = 0;
		if (SkipEmptyTokens)
		{
			while (offset < length && delimiters.contains(ptr[offset]))
				++offset;
		}
		size_t end = offset;
		while (end < length && !delimiters.contains(ptr[end]))
			++end;
		Slice<T> token = slice(offset, end);
		if (end < length)
			++end;
		ptr += end;
		length -= end;
		return token;
	}

	template <typename T, bool S>
	template <bool SkipEmptyTokens>
	inline Slice<Slice<T>> Slice<T, S>::tokenise(Slice<Slice<T>> tokens, Slice<const T> delimiters) noexcept
	{
		size_t numTokens = 0;
		size_t offset = 0;
		while (offset < length && numTokens < tokens.length)
		{
			if (!SkipEmptyTokens)
			{
				size_t tokStart = offset;
				while (offset < length && !delimiters.contains(ptr[offset]))
					++offset;
				tokens[numTokens++] = slice(tokStart, offset);
				++offset;
			}
			else
			{
				while (offset < length && delimiters.contains(ptr[offset]))
					++offset;
				if (offset == length)
					break;
				size_t tokStart = offset;
				while (offset < length && !delimiters.contains(ptr[offset]))
					++offset;
				tokens[numTokens++] = slice(tokStart, offset);
			}
		}
		if (!SkipEmptyTokens && offset > length)
			offset = length;
		ptr += offset;
		length -= offset;
		return tokens.front(numTokens);
	}
	template <typename T, bool S>
	template <bool SkipEmptyTokens>
	inline size_t Slice<T, S>::tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<const T> delimiters) noexcept
	{
		size_t numTokens = 0;
		size_t offset = 0;
		while (offset < length)
		{
			if (!SkipEmptyTokens)
			{
				size_t tokStart = offset;
				while (offset < length && !delimiters.contains(ptr[offset]))
					++offset;
				onToken(Slice<T>(ptr + tokStart, offset++), numTokens++);
			}
			else
			{
				while (offset < length && delimiters.contains(ptr[offset]))
					++offset;
				if (offset == length)
					break;
				size_t tokStart = offset;
				while (offset < length && !delimiters.contains(ptr[offset]))
					++offset;
				onToken(Slice<T>(ptr + tokStart, offset), numTokens++);
			}
		}
		return numTokens;
	}

	namespace detail
	{
		// TODO: these are pretty lame! use lookup table? (requires .cpp file)
		constexpr bool is_whitespace(char32_t c) noexcept { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
		constexpr bool is_hex(char32_t c) noexcept { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
		constexpr bool is_number(char32_t c) noexcept { return c >= '0' && c <= '9'; }

		constexpr char32_t to_lower(char32_t c) noexcept { return c >= 'A' && c <= 'Z' ? c | 0x20 : c; }
		constexpr char32_t to_upper(char32_t c) noexcept { return c >= 'a' && c <= 'z' ? c & ~0x20 : c; }

		template <typename C>
		inline size_t strlen(const C *c_str) noexcept
		{
			size_t len = 0;
			while (c_str[len])
				++len;
			return len;
		}
		template <> inline size_t strlen<char>(const char *c_str) noexcept { return ::strlen(c_str); }
		template <> inline size_t strlen<unsigned char>(const unsigned char *c_str) noexcept { return ::strlen((char*)c_str); }
		template <> inline size_t strlen<wchar_t>(const wchar_t *c_str) noexcept { return wcslen(c_str); }

		template<typename C>
		constexpr size_t utf_seq_length(char32_t c) noexcept;
		template<>
		constexpr size_t utf_seq_length<char>(char32_t c) noexcept
		{
			return c < 0x80 ? 1 : (c < 0x800 ? 2 : (c < 0x10000 ? 3 : 4));
		}
		template<>
		constexpr size_t utf_seq_length<unsigned char>(char32_t c) noexcept
		{
			return c < 0x80 ? 1 : (c < 0x800 ? 2 : (c < 0x10000 ? 3 : 4));
		}
		template<>
		constexpr size_t utf_seq_length<char16_t>(char32_t c) noexcept
		{
			return c < 0x10000 ? 1 : 2;
		}
		template<>
		constexpr size_t utf_seq_length<char32_t>(char32_t) noexcept
		{
			return 1;
		}
		template<>
		constexpr size_t utf_seq_length<wchar_t>(char32_t c) noexcept
		{
			static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t), "Unexpected wchar_t size!");
			return sizeof(wchar_t) == sizeof(char16_t) ? (c < 0x10000 ? 1 : 2) : 1;
		}

		inline size_t utf_encode(char32_t c, char *utf8) noexcept
		{
			if (c < 0x80)
			{
				utf8[0] = (char)c;
				return 1;
			}
			else if (c < 0x800)
			{
				utf8[1] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[0] = (c & 0x1f) | 0xc0;
				return 2;
			}
			else if (c < 0x10000)
			{
				utf8[2] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[1] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[0] = (c & 0x0f) | 0xe0;
				return 3;
			}
			else // NOTE: unicode was restricted to 20 bits, so we will stop here. the upper bits of a char32_t are truncated.
			{
				utf8[3] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[2] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[1] = (c & 0x3f) | 0x80; c >>= 6;
				utf8[0] = (c & 0x07) | 0xf0;
				return 4;
			}
		}
		inline size_t utf_encode(char32_t c, unsigned char *utf8) noexcept
		{
			return utf_encode(c, (char*)utf8);
		}
		inline size_t utf_encode(char32_t c, char16_t *utf16) noexcept
		{
			if (c < 0x10000)
			{
				utf16[0] = (char16_t)c;
				return 1;
			}
			else
			{
				c -= 0x10000;
				utf16[0] = (char16_t)(0xD800 | (c >> 10));
				utf16[1] = (char16_t)(0xDC00 | (c & 0x3FF));
				return 2;
			}
		}
		inline size_t utf_encode(char32_t c, char32_t *utf32) noexcept
		{
			*utf32 = c;
			return 1;
		}
		inline size_t utf_encode(char32_t c, wchar_t *wc) noexcept
		{
			static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t), "Unexpected wchar_t size!");
			if (sizeof(wchar_t) == sizeof(char16_t))
				return utf_encode(c, (char16_t*)wc);
			else
			{
				*wc = (wchar_t)c;
				return 1;
			}
		}

		inline size_t utf_decode(const char *utf8, char32_t *c) noexcept
		{
			if ((unsigned char)utf8[0] < 128)
			{
				*c = utf8[0];
				return 1;
			}
			else if ((utf8[0] & 0xE0) == 0xC0)
			{
				*c = ((char32_t)(utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
				return 2;
			}
			else if ((utf8[0] & 0xF0) == 0xE0)
			{
				*c = ((char32_t)(utf8[0] & 0x0F) << 12) | ((char32_t)(utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
				return 3;
			}
			else
			{
				*c = ((char32_t)(utf8[0] & 0x07) << 18) | ((char32_t)(utf8[1] & 0x3F) << 12) | ((char32_t)(utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
				return 4;
			}
		}
		inline size_t utf_decode(const unsigned char *utf8, char32_t *c) noexcept
		{
			return utf_decode((const char*)utf8, c);
		}
		inline size_t utf_decode(const char16_t *utf16, char32_t *c) noexcept
		{
			if (utf16[0] >= 0xD800 && (utf16[0] & 0xFC00) == 0xD800)
			{
				*c = ((char32_t)(utf16[0] & 0x3FF) << 10) | (utf16[1] & 0x3FF);
				return 2;
			}
			else
			{
				*c = utf16[0];
				return 1;
			}
		}
		inline size_t utf_decode(const char32_t *utf32, char32_t *c) noexcept
		{
			*c = *utf32;
			return 1;
		}
		inline size_t utf_decode(const wchar_t *wc, char32_t *c) noexcept
		{
			static_assert(sizeof(wchar_t) == sizeof(char16_t) || sizeof(wchar_t) == sizeof(char32_t), "Unexpected wchar_t size!");
			if (sizeof(wchar_t) == sizeof(char16_t))
				return utf_decode((char16_t*)wc, c);
			else
			{
				*c = (char32_t)*wc;
				return 1;
			}
		}

		template <typename C, bool S, typename D, bool T>
		size_t url_encode(Slice<C, S> buffer, Slice<D, T> text) noexcept
		{
			const D *s = text.ptr;
			const D *end = text.ptr + text.length;

			size_t len = 0;

			unsigned char temp[4];
			size_t tlen = 1;

			while (s < end)
			{
				if (sizeof(D) > 1)
				{
					char32_t t;
					s += utf_decode(s, &t);
					tlen = utf_encode(t, temp);
				}
				else
					temp[0] = (unsigned char)*s++;

				for (size_t i = 0; i < tlen; ++i)
				{
					unsigned char c = temp[i];
					if (c == ' ')
					{
						buffer.ptr[len++] = '+';
					}
					else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
						|| c == '_' || c == '-' || c == '.' || c == '~'
//						|| c == '*' || c == '!' || c == '(' || c == ')' // these sometimes seem to be valid?
						)
					{
						buffer.ptr[len++] = c;
					}
					else
					{
						int h = (unsigned char)c >> 4, l = c & 0xF;
						buffer.ptr[len++] = '%';
						buffer.ptr[len++] = h < 10 ? '0' + h : 'A' + h - 10;
						buffer.ptr[len++] = l < 10 ? '0' + l : 'A' + l - 10;
					}
				}
			}

			return len;
		}

		template <typename C, bool S, typename D, bool T>
		size_t url_decode(Slice<C, S> buffer, Slice<D, T> url) noexcept
		{
			const D *s = url.ptr;
			const D *end = url.ptr + url.length;

			size_t len = 0;

			unsigned char utf[4];
			size_t tlen = 0;

			while (s < end)
			{
				char32_t c;
				s += utf_decode(s, &c);

				if (c == '+')
					c = ' ';

				while (c == '%' && s < end - 1 && is_hex(s[0]) && is_hex(s[1]))
				{
					utf[tlen++] = (unsigned char)Slice<const D>(s, 2).parse_int(false, 16);
					s += 2;

					if (utf[0] < 128)
					{
						c = utf[0];
						tlen = 0;
						break;
					}
					else if (((utf[0] & 0xE0) == 0xC0 && tlen == 2) ||
						((utf[0] & 0xF0) == 0xE0 && tlen == 3) ||
						tlen == 4)
					{
						utf_decode(utf, &c);
						tlen = 0;
						break;
					}

					c = *s++;
				}

				len += utf_encode(c, buffer.ptr + len);
			}
			return len;
		}
	}

	template <typename C>
	constexpr Slice<C, true>::Slice() noexcept
		: Slice<C, false>() {}

	template <typename C>
	constexpr Slice<C, true>::Slice(C* ptr, size_t length) noexcept
		: Slice<C, false>(ptr, length) {}

	template <typename C>
	template <typename U, bool S>
	constexpr Slice<C, true>::Slice(Slice<U, S> slice) noexcept
		: Slice<C, false>(slice.ptr, slice.length) {}

	template <typename C>
	Slice<C, true>::Slice(C *c_str) noexcept
		: Slice<C, false>(c_str, c_str ? detail::strlen(c_str) : 0) {}

//	template <typename C>
//	template <size_t N>
//	constexpr Slice<C, true>::Slice(C(&str_literal)[N]) noexcept
//		: Slice<C, false>(str_literal, N - 1) {} // literals don't work?!

#if !defined(NO_STL)
	template <typename C>
	template <class _Ty, class _Alloc>
	inline Slice<C, true>::Slice(const std::vector<_Ty, _Alloc> &vec) noexcept
		: Slice<C, false>(vec.data(), vec.size()) {}

	template <typename C>
	template <class _Elem, class _Traits, class _Alloc>
	inline Slice<C, true>::Slice(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
		: Slice<C, false>(str.data(), str.length()) {}
#endif

	template<typename C>
	inline size_t Slice<C, true>::num_chars() const noexcept
	{
		Slice<C> t = *this;
		size_t numChars = 0;
		while (t.length)
		{
			t.pop_front_char();
			++numChars;
		}
		return numChars;
	}
	template<>
	inline size_t Slice<char32_t, true>::num_chars() const noexcept { return length; }
	template<>
	inline size_t Slice<const char32_t, true>::num_chars() const noexcept { return length; }

	template<typename C>
	inline char32_t Slice<C, true>::front_char() const noexcept
	{
		SLICE_ASSERT(this->length > 0);
		char32_t r;
		detail::utf_decode(this->ptr, &r);
		return r;
	}
	template<>
	inline char32_t Slice<const char, true>::back_char() const noexcept
	{
		SLICE_ASSERT(this->length > 0);
		const char *last = ptr + length - 1;
		if ((unsigned char)*last < 128)
			return *last;
		do --last;
		while ((*last & 0xC) == 0x80 && last >= this->ptr);
		char32_t r;
		detail::utf_decode(last, &r);
		return r;
	}
	template<>
	inline char32_t Slice<char, true>::back_char() const noexcept { return ((Slice<const char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<const unsigned char, true>::back_char() const noexcept { return ((Slice<const char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<unsigned char, true>::back_char() const noexcept { return ((Slice<const char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<const char16_t, true>::back_char() const noexcept
	{
		SLICE_ASSERT(this->length > 0);
		char16_t back = ptr[length - 1];
		if (back >= 0xD800 && (back & 0xFC00) == 0xDC00 && this->length > 1)
		{
			char32_t r;
			detail::utf_decode(ptr + length - 2, &r);
			return r;
		}
		return back;
	}
	template<>
	inline char32_t Slice<char16_t, true>::back_char() const noexcept { return ((Slice<const char16_t, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<const char32_t, true>::back_char() const noexcept { SLICE_ASSERT(this->length > 0); return ptr[length - 1]; }
	template<>
	inline char32_t Slice<char32_t, true>::back_char() const noexcept { SLICE_ASSERT(this->length > 0); return ptr[length - 1]; }

	template<typename C>
	inline char32_t Slice<C, true>::pop_front_char() noexcept
	{
		SLICE_ASSERT(this->length > 0);
		char32_t r;
		size_t codeUnits = detail::utf_decode(this->ptr, &r);
		this->ptr += codeUnits;
		this->length -= codeUnits;
		return r;
	}
	template<>
	inline char32_t Slice<const char, true>::pop_back_char() noexcept
	{
		SLICE_ASSERT(this->length > 0);
		if ((unsigned char)ptr[length - 1] < 128)
			return ptr[--length];
		size_t numChars = 2;
		while (this->length >= numChars && (ptr[length - numChars] & 0xC) == 0x80)
			++numChars;
		char32_t r;
		detail::utf_decode(ptr + length - numChars, &r);
		length -= numChars;
		return r;
	}
	template<>
	inline char32_t Slice<char, true>::pop_back_char() noexcept { return ((Slice<const char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<const unsigned char, true>::pop_back_char() noexcept { return ((Slice<const char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<unsigned char, true>::pop_back_char() noexcept { return ((Slice<const char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<const char16_t, true>::pop_back_char() noexcept
	{
		SLICE_ASSERT(this->length > 0);
		char16_t back = ptr[--length];
		if (back >= 0xD800 && (back & 0xFC00) == 0xDC00 && this->length > 1)
		{
			char32_t r;
			detail::utf_decode(ptr + --length, &r);
			return r;
		}
		return back;
	}
	template<>
	inline char32_t Slice<char16_t, true>::pop_back_char() noexcept { return ((Slice<const char16_t, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<const char32_t, true>::pop_back_char() noexcept { SLICE_ASSERT(this->length > 0); return ptr[--length]; }
	template<>
	inline char32_t Slice<char32_t, true>::pop_back_char() noexcept { SLICE_ASSERT(this->length > 0); return ptr[--length]; }

	template<typename C>
	inline bool Slice<C, true>::eq_ic(Slice<const C> str) const noexcept
	{
		if (this->length != str.length)
			return false;
		for (size_t i = 0; i < this->length; ++i)
			if (detail::to_lower(this->ptr[i]) != detail::to_lower(str.ptr[i]))
				return false;
		return true;
	}
	template<typename C>
	inline bool Slice<C, true>::begins_with_ic(Slice<const C> str) const noexcept
	{
		if (this->length < str.length)
			return false;
		return Slice<C>(this->ptr, str.length).eq_ic(str);
	}
	template<typename C>
	inline bool Slice<C, true>::ends_with_ic(Slice<const C> str) const noexcept
	{
		if (this->length < str.length)
			return false;
		return Slice<C>(this->ptr + this->length - str.length, str.length).eq_ic(str);
	}
	template<typename C>
	inline ptrdiff_t Slice<C, true>::cmp_ic(Slice<const C> str) const noexcept
	{
		size_t len = this->length < str.length ? this->length : str.length;
		for (size_t i = 0; i < len; ++i)
		{
			C a = detail::to_lower(this->ptr[i]);
			C b = detail::to_lower(str.ptr[i]);
			if (a != b)
				return a < b ? -1 : 1;
		}
		return this->length - str.length;
	}

	template<typename C>
	inline C* Slice<C, true>::to_cstring(C *buffer, size_t bufferLen) const noexcept
	{
		size_t len = this->length < bufferLen - 1 ? this->length : bufferLen - 1;
		memcpy(buffer, this->ptr, len);
		buffer[len] = 0;
		return buffer;
	}

	template <typename C>
	inline size_t Slice<C, true>::find_first_ic(C c) const noexcept
	{
		c = detail::to_lower(c);
		size_t offset = 0;
		while (offset < this->length && detail::to_lower(this->ptr[offset]) != c)
			++offset;
		return offset;
	}
	template <typename C>
	inline size_t Slice<C, true>::find_last_ic(C c) const noexcept
	{
		c = detail::to_lower(c);
		ptrdiff_t last = this->length - 1;
		while (last >= 0 && detail::to_lower(this->ptr[last]) != c)
			--last;
		return last < 0 ? this->length : last;
	}

	template<typename C>
	inline size_t Slice<C, true>::find_first_ic(Slice<const C> s) const noexcept
	{
		if (s.empty())
			return 0;
		ptrdiff_t len = this->length - s.length;
		for (ptrdiff_t i = 0; i < len; ++i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (detail::to_lower(this->ptr[i + j]) != detail::to_lower(s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return this->length;
	}
	template<typename C>
	inline size_t Slice<C, true>::find_last_ic(Slice<const C> s) const noexcept
	{
		if (s.empty())
			return this->length;
		for (ptrdiff_t i = this->length - s.length; i >= 0; --i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (detail::to_lower(this->ptr[i + j]) != detail::to_lower(s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return this->length;
	}

	template <typename C>
	inline Slice<C> Slice<C, true>::get_left_at_first_ic(C c, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return{ this->ptr, offset };
	}
	template <typename C>
	inline Slice<C> Slice<C, true>::get_left_at_last_ic(C c, bool inclusive) const noexcept
	{
		size_t offset = find_last_ic(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return{ this->ptr, offset };
	}
	template <typename C>
	inline Slice<C> Slice<C, true>::get_right_at_first_ic(C c, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return{ this->ptr + offset, this->length - offset };
	}
	template <typename C>
	inline Slice<C> Slice<C, true>::get_right_at_last_ic(C c, bool inclusive) const noexcept
	{
		size_t offset = find_last_ic(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return{ this->ptr + offset, this->length - offset };
	}

	template<typename C>
	inline Slice<C> Slice<C, true>::get_left_at_first_ic(Slice<const C> s, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_left_at_last_ic(Slice<const C> s, bool inclusive) const noexcept
	{
		size_t offset = find_last_ic(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_right_at_first_ic(Slice<const C> s, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return{ this->ptr + offset, this->length - offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_right_at_last_ic(Slice<const C> s, bool inclusive) const noexcept
	{
		size_t offset = find_last_ic(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return{ this->ptr + offset, this->length - offset };
	}

	template<typename C>
	template<bool Front, bool Back>
	inline Slice<C> Slice<C, true>::trim() const noexcept
	{
		size_t first = 0, last = this->length;
		if (Front)
		{
			while (first < this->length && detail::is_whitespace(this->ptr[first]))
				++first;
		}
		if (Back)
		{
			while (last > first && detail::is_whitespace(this->ptr[last - 1]))
				--last;
		}
		return{ this->ptr + first, last - first };
	}

	template<typename C>
	inline int64_t Slice<C, true>::parse_int(bool detectBase, int base) const noexcept
	{
		const C *s = this->ptr;
		const C *end = s + this->length;

		if (this->length == 0)
			return 0; // TODO: perhaps it would be better to throw if the string is not a number?

		if (detectBase)
		{
			if (*s == '$')
			{
				base = 16;
				++s;
			}
			else if (*s == '0' && detail::to_lower(s[1]) == 'x')
			{
				base = 16;
				s += 2;
			}
			else if (detail::to_lower(*s) == 'b')
			{
				base = 2;
				++s;
			}
		}

		switch (base)
		{
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			{
				int64_t number = 0;
				while (s < end && *s >= '0' && *s <= '0' + ((C)base - 1))
				{
					number = number*base + (*s - '0');
					++s;
				}
				return number;
			}
			case 10:
			{
				// decimal number
				bool neg = *s == '-';
				s += *s == '-' || *s == '+';

				int64_t number = 0;
				while (s < end && *s >= '0' && *s <= '9')
				{
					number = number * 10 + *s - '0';
					++s;
				}
				return neg ? -number : number;
			}
			case 16:
			{
				int64_t number = 0;
				while (s < end && detail::is_hex(*s))
				{
					number = (number << 4) | (detail::is_number(*s) ? *s - '0' : 10 + (*s | 0x20) - 'a');
					++s;
				}
				return number;
			}
			default:
				SLICE_ASSERT(false);
		}
		return 0;
	}

	template<typename C>
	inline double Slice<C, true>::parse_float() const noexcept
	{
		const C *s = this->ptr;
		const C *end = s + this->length;

		bool neg = *s == '-';
		s += *s == '-' || *s == '+';

		size_t n = 0;
		while (s < end && *s >= '0' && *s <= '9')
		{
			n = n * 10 + *s - '0';
			++s;
		}

		double r;
		if (*s == '.')
		{
			++s;
			size_t f = 0;
			double fracSize = 1;
			while (s < end && *s >= '0' && *s <= '9')
			{
				f = f * 10 + *s - '0';
				fracSize *= 10;
				++s;
			}
			r = (double)n + (double)f / fracSize;
		}
		else
			r = (double)n;

		if (neg)
			r = -r;

		if (*s == 'e' || *s == 'E')
		{
			++s;
			bool negExp = *s == '-';
			s += *s == '-' || *s == '+';

			int16_t exp = 0;
			while (s < end && *s >= '0' && *s <= '9')
			{
				exp = exp * 10 + *s - '0';
				++s;
			}

			// calculate 10^^exp
			double e = 1;
			double base = 10;
			while (exp)
			{
				if (exp & 1)
					e *= base;
				exp >>= 1;
				base *= base;
			}
			r = !negExp ? r * e : r / e;
		}

		return r;
	}

	template<typename C>
	inline uint32_t Slice<C, true>::hash(uint32_t hash) const noexcept
	{
		// TODO: is there a better hash for utf16/utf32?
		const unsigned char *data = (const unsigned char *)this->ptr;
		size_t len = this->length * sizeof(C);
		for (size_t i = 0; i < len; ++i)
		{
			hash ^= (uint32_t)data[i];
			hash *= 0x01000193;
		}
		return hash;
	}

	namespace detail
	{
		template <typename T> struct is_slice								{ enum { value = 0 }; };
		template <typename T, bool S> struct is_slice<Slice<T, S>>			{ enum { value = 1 }; };
		template <typename T, bool S> struct is_slice<const Slice<T, S>>	{ enum { value = 1 }; };
	}

	template <typename Dest, typename Src>
	constexpr Dest slice_static_cast(Src src) noexcept
	{
		static_assert(detail::is_slice<Src>::value && detail::is_slice<Dest>::value, "Dest and/or Src is not a Slice");
		SLICE_ASSERT(src.length * sizeof(typename Src::value_type) % sizeof(typename Dest::value_type) == 0);
		return Dest(static_cast<typename Dest::value_type*>(src.ptr), src.length * sizeof(typename Src::value_type) / sizeof(typename Dest::value_type));
	}

	template <typename Dest, typename Src>
	constexpr Dest slice_reinterpret_cast(Src src) noexcept
	{
		static_assert(detail::is_slice<Src>::value && detail::is_slice<Dest>::value, "Dest and/or Src is not a Slice");
		SLICE_ASSERT(src.length * sizeof(typename Src::value_type) % sizeof(typename Dest::value_type) == 0);
		return Dest(reinterpret_cast<typename Dest::value_type*>(src.ptr), src.length * sizeof(typename Src::value_type) / sizeof(typename Dest::value_type));
	}

	template <typename Dest, typename Src>
	constexpr Dest slice_const_cast(Src src) noexcept
	{
		static_assert(detail::is_slice<Src>::value && detail::is_slice<Dest>::value, "Dest and/or Src is not a Slice");
		SLICE_ASSERT(src.length * sizeof(typename Src::value_type) % sizeof(typename Dest::value_type) == 0);
		return Dest(const_cast<typename Dest::value_type*>(src.ptr), src.length * sizeof(typename Src::value_type) / sizeof(typename Dest::value_type));
	}
}
