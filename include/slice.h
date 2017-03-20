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

#include <assert.h>

#if !defined(NO_STL)
# include <vector>
# include <string>
#endif

namespace beautifulcode
{
	namespace detail
	{
		template <typename T> struct SliceElementType		{ using Ty = T; };
		template <> struct SliceElementType<void>			{ using Ty = uint8_t; };
		template <> struct SliceElementType<const void>		{ using Ty = const uint8_t; };

		template <typename T> struct IsSomeChar				{ enum { value = false }; };
		template <> struct IsSomeChar<char>					{ enum { value = true }; };
		template <> struct IsSomeChar<const char>			{ enum { value = true }; };
		template <> struct IsSomeChar<unsigned char>		{ enum { value = true }; };
		template <> struct IsSomeChar<const unsigned char>	{ enum { value = true }; };
		template <> struct IsSomeChar<char16_t>				{ enum { value = true }; };
		template <> struct IsSomeChar<const char16_t>		{ enum { value = true }; };
		template <> struct IsSomeChar<char32_t>				{ enum { value = true }; };
		template <> struct IsSomeChar<const char32_t>		{ enum { value = true }; };
		template <> struct IsSomeChar<wchar_t>				{ enum { value = true }; };
		template <> struct IsSomeChar<const wchar_t>		{ enum { value = true }; };
	}

	// support for C++ iterators
	template <typename T>
	class Iterator
	{
	public:
		Iterator(T *ptr) : ptr(ptr) {}

		T &operator*() const { return *ptr; }
		T *operator->() const { return ptr; }

		bool operator!=(Iterator<T> iter) const { return ptr != iter.ptr; }

		Iterator<T> operator++()
		{
			++ptr;
			return *this;
		}

	private:
		T *ptr;
	};

	template <typename T, bool IsString = detail::IsSomeChar<T>::value>
	struct Slice
	{
	public:
		using ElementType = typename detail::SliceElementType<T>::Ty;

		typedef bool(Predicate)(const ElementType &e);

		size_t length;
		T *ptr;

		constexpr Slice() noexcept;
		constexpr Slice(nullptr_t) noexcept;
		constexpr Slice(T* ptr, size_t length) noexcept;
		template <typename U, bool S> constexpr Slice(Slice<U, S> slice) noexcept;
		template <typename U, size_t N> constexpr Slice(U(&arr)[N]) noexcept;
		explicit Slice(std::initializer_list<ElementType> list) noexcept;
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Slice(std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> Slice(std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif

		template <typename U, bool S> Slice<T, IsString>& operator=(Slice<U, S> slice) noexcept;

		constexpr ElementType& at(size_t i) const;
		constexpr ElementType& operator[](size_t i) const noexcept;

		constexpr Slice<T> slice(size_t first, size_t last) const noexcept;

		constexpr T *data() const noexcept		{ return ptr; }
		constexpr size_t size() const noexcept	{ return length; }
		constexpr bool empty() const noexcept	{ return length == 0; }
		void clear() noexcept					{ ptr = nullptr; length = 0; }

		explicit constexpr operator bool() const noexcept { return length != 0; }

		constexpr bool operator==(Slice<const T> slice) const noexcept { return ptr == slice.ptr && length == slice.length; }
		constexpr bool operator!=(Slice<const T> slice) const noexcept { return ptr != slice.ptr || length != slice.length; }

		template <typename U> bool eq(Slice<U> slice) const noexcept;
		template <typename U> ptrdiff_t cmp(Slice<U> slice) const noexcept;

		template <typename U> bool begins_with(Slice<U> slice) const noexcept;
		template <typename U> bool ends_with(Slice<U> slice) const noexcept;

		Iterator<T> begin() const noexcept { return Iterator<T>(&ptr[0]); }
		Iterator<T> end() const noexcept { return Iterator<T>(&ptr[length]); }

		constexpr ElementType& front() const noexcept				{ return ((ElementType*)ptr)[0]; }
		constexpr Slice<ElementType> front(size_t n) const noexcept	{ return slice(0, n); }
		constexpr ElementType& back() const noexcept				{ return ((ElementType*)ptr)[length - 1]; }
		constexpr Slice<ElementType> back(size_t n) const noexcept	{ return slice(length - n, length); }

		ElementType& pop_front() noexcept;
		Slice<ElementType> pop_front(size_t n) noexcept;
		ElementType& pop_back() noexcept;
		Slice<ElementType> pop_back(size_t n) noexcept;

		constexpr Slice<T> drop_front(size_t n) const noexcept	{ return slice(n, length); }
		constexpr Slice<T> drop_back(size_t n) const noexcept	{ return slice(0, length - n); }

		bool exists(const ElementType &c, size_t *index = nullptr) const noexcept;
		size_t find_first(const ElementType &c) const noexcept;
		template <typename U> size_t find_first(Slice<U> s) const noexcept;
		size_t find_last(const ElementType &c) const noexcept;
		template <typename U> size_t find_last(Slice<U> s) const noexcept;

		T *search(Predicate *predFunc) const noexcept;

		ptrdiff_t index_of_element(const T *c) const noexcept;

		Slice<T> get_left_at_first(const ElementType &c, bool inclusive = false) const noexcept;
		Slice<T> get_left_at_last(const ElementType &c, bool inclusive = false) const noexcept;
		Slice<T> get_right_at_first(const ElementType &c, bool inclusive = true) const noexcept;
		Slice<T> get_right_at_last(const ElementType &c, bool inclusive = true) const noexcept;

		template <typename U> Slice<T> get_left_at_first(Slice<U> s, bool inclusive = false) const noexcept;
		template <typename U> Slice<T> get_left_at_last(Slice<U> s, bool inclusive = false) const noexcept;
		template <typename U> Slice<T> get_right_at_first(Slice<U> s, bool inclusive = true) const noexcept;
		template <typename U> Slice<T> get_right_at_last(Slice<U> s, bool inclusive = true) const noexcept;

		template <typename U>
		size_t copy_to(Slice<U> dest) const noexcept;

		template <bool SkipEmptyTokens = false>
		Slice<T> pop_token(Slice<T> delimiters) noexcept;

		template <bool SkipEmptyTokens = false>
		Slice<Slice<T>> tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters) noexcept;
		template <bool SkipEmptyTokens = false>
		size_t tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters) noexcept;
	};

	// specialisation for strings
	template <typename C>
	struct Slice<C, true> : Slice<C, false>
	{
		constexpr Slice() noexcept;
		constexpr Slice(C* ptr, size_t length) noexcept;
		template <typename U, bool S> constexpr Slice(Slice<U, S> slice) noexcept;
		Slice(C *c_str) noexcept;
//		template <size_t N> constexpr Slice(C(&str_literal)[N]) noexcept;

#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Slice(std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> Slice(std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif

		size_t num_chars() const noexcept;
		char32_t front_char() const noexcept;
		char32_t back_char() const noexcept;
		char32_t pop_front_char() noexcept;
		char32_t pop_back_char() noexcept;

		bool eq_ic(Slice<C> str) const noexcept;
		bool begins_with_ic(Slice<C> str) const noexcept;
		bool ends_with_ic(Slice<C> str) const noexcept;
		ptrdiff_t cmp_ic(Slice<C> str) const noexcept;

		C* to_cstring(C *buffer, size_t bufferLen) const noexcept;
//		CString<C> to_stringz() const noexcept;

		size_t find_first_ic(Slice<C> s) const noexcept;
		size_t find_last_ic(Slice<C> s) const noexcept;
		Slice<C> get_left_at_first_ic(Slice<C> s, bool inclusive = false) const noexcept;
		Slice<C> get_left_at_last_ic(Slice<C> s, bool inclusive = false) const noexcept;
		Slice<C> get_right_at_first_ic(Slice<C> s, bool inclusive = true) const noexcept;
		Slice<C> get_right_at_last_ic(Slice<C> s, bool inclusive = true) const noexcept;

		template<bool Front = true, bool Back = true>
		Slice<C> trim() const noexcept;

		template<bool SkipEmptyTokens = true>
		Slice<C> pop_token(Slice<C> delimiters = " \t\n\r") noexcept															{ return ((Slice<C, false>*)this)->pop_token<SkipEmptyTokens>(delimiters); }
		template<bool SkipEmptyTokens = true>
		Slice<Slice<C>> tokenise(Slice<Slice<C>> tokens, Slice<C> delimiters = " \t\n\r") noexcept								{ return ((Slice<C, false>*)this)->tokenise<SkipEmptyTokens>(tokens, delimiters); }
		template<bool SkipEmptyTokens = true>
		size_t tokenise(std::function<void(Slice<C> token, size_t index)> onToken, Slice<C> delimiters = " \t\n\r") noexcept	{ return ((Slice<C, false>*)this)->tokenise<SkipEmptyTokens>(onToken, delimiters); }

//		int64_t parse_int(bool detectBase = true, int base = 10) const;
//		double parse_float() const;

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

	// Cast from a Slice<T> to a Slice<U> where T and U are compatible types.	Its typical use is for casting from a Slice<void>.
	template <typename Dst, typename Src>
	constexpr Dst slice_cast(Src src) noexcept;


	//
	// Implementation follows:
	//

	template <typename T, bool S>
	constexpr Slice<T, S>::Slice() noexcept
		: length(0), ptr(nullptr) {}

	template <typename T, bool S>
	constexpr Slice<T, S>::Slice(nullptr_t) noexcept
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
	inline Slice<T, S>::Slice(std::initializer_list<ElementType> list) noexcept
		: length(list.size()), ptr(list.begin()) {}

#if !defined(NO_STL)
	template <typename T, bool S>
	template <class _Ty, class _Alloc>
	inline Slice<T, S>::Slice(std::vector<_Ty, _Alloc> &vec) noexcept
		: length(vec.length()), ptr(vec.data()) {}

	template <typename T, bool S>
	template <class _Elem, class _Traits, class _Alloc>
	inline Slice<T, S>::Slice(std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
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
	constexpr typename Slice<T, S>::ElementType& Slice<T, S>::at(size_t i) const
	{
		if (i >= length)
			throw std::out_of_range("Index out of bounds");
		return ((ElementType*)ptr)[i];
	}

	template <typename T, bool S>
	constexpr typename Slice<T, S>::ElementType& Slice<T, S>::operator[](size_t i) const noexcept
	{
		assert(i < length);
		return ((ElementType*)ptr)[i];
	}

	template <typename T, bool S>
	constexpr Slice<T> Slice<T, S>::slice(size_t first, size_t last) const noexcept
	{
		assert(first <= last && last <= length);
		return Slice<T>((ElementType*)ptr + first, last - first);
	}

	template <typename T, bool S>
	template <typename U>
	bool Slice<T, S>::eq(Slice<U> slice) const noexcept
	{
		if (length != slice.length)
			return false;
		for (size_t i = 0; i < length; ++i)
			if (((ElementType*)ptr)[i] != ((ElementType*)slice.ptr)[i])
				return false;
		return true;
	}

	template <typename T, bool S>
	template <typename U>
	inline ptrdiff_t Slice<T, S>::cmp(Slice<U> slice) const noexcept
	{
		size_t len = length < slice.length ? length : slice.length;
		for (size_t i = 0; i < len; ++i)
		{
			if (((ElementType*)ptr)[i] != ((ElementType*)slice.ptr)[i])
				return ((ElementType*)ptr)[i] < ((ElementType*)slice.ptr)[i] ? -1 : 1;
		}
		return length - slice.length;
	}

	template <typename T, bool S>
	template <typename U>
	inline bool Slice<T, S>::begins_with(Slice<U> slice) const noexcept
	{
		if (length < slice.length)
			return false;
		return Slice<T>(0, slice.length).eq(slice);
	}
	template <typename T, bool S>
	template <typename U>
	inline bool Slice<T, S>::ends_with(Slice<U> slice) const noexcept
	{
		if (length < slice.length)
			return false;
		return Slice<T>(length - slice.length, length).eq(slice);
	}

	template <typename T, bool S>
	inline typename Slice<T, S>::ElementType& Slice<T, S>::pop_front() noexcept
	{
		assert(length > 0);
		++ptr;
		--length;
		return ((ElementType*)ptr)[-1];
	}
	template <typename T, bool S>
	inline Slice<typename detail::SliceElementType<T>::Ty> Slice<T, S>::pop_front(size_t n) noexcept
	{
		assert(length >= n);
		ptr += n;
		length -= n;
		return{ ptr - n, n };
	}
	template <typename T, bool S>
	inline typename Slice<T, S>::ElementType& Slice<T, S>::pop_back() noexcept
	{
		assert(length > 0);
		--length;
		return ((ElementType*)ptr)[length];
	}
	template <typename T, bool S>
	inline Slice<typename detail::SliceElementType<T>::Ty> Slice<T, S>::pop_back(size_t n) noexcept
	{
		assert(length >= n);
		length -= n;
		return{ ptr + length, n };
	}

	template <typename T, bool S>
	inline bool Slice<T, S>::exists(const typename Slice<T, S>::ElementType &c, size_t *index) const noexcept
	{
		size_t i = find_first(c);
		if (index)
			*index = i;
		return i != length;
	}

	namespace detail
	{
		// hack to handle slices of slices
		template <typename T> struct FindImpl								{ static inline bool eq(const T &a, const T &b) { return a == b; } };
		template <typename U, bool S> struct FindImpl<Slice<U, S>>			{ static inline bool eq(const Slice<U, S> &a, const Slice<U, S> &b) { return a.eq(b); } };
		template <typename U, bool S> struct FindImpl<const Slice<U, S>>	{ static inline bool eq(const Slice<U, S> &a, const Slice<U, S> &b) { return a.eq(b); } };
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_first(const typename Slice<T, S>::ElementType &c) const noexcept
	{
		size_t offset = 0;
		while (offset < length && !detail::FindImpl<typename Slice<T, S>::ElementType>::eq(ptr[offset], c))
			++offset;
		return offset;
	}
	template <typename T, bool S>
	inline size_t Slice<T, S>::find_last(const typename Slice<T, S>::ElementType &c) const noexcept
	{
		ptrdiff_t last = length - 1;
		while (last >= 0 && !detail::FindImpl<typename Slice<T, S>::ElementType>::eq(ptr[last], c))
			--last;
		return last < 0 ? length : last;
	}
	template <typename T, bool S>
	template <typename U>
	inline size_t Slice<T, S>::find_first(Slice<U> s) const noexcept
	{
		if (s.empty())
			return 0;
		ptrdiff_t len = length - s.length;
		for (ptrdiff_t i = 0; i < len; ++i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T, S>::ElementType>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}
	template <typename T, bool S>
	template <typename U>
	inline size_t Slice<T, S>::find_last(Slice<U> s) const noexcept
	{
		if (s.empty())
			return length;
		for (ptrdiff_t i = length - s.length; i >= 0; --i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T, S>::ElementType>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}

	template <typename T, bool S>
	inline T* Slice<T, S>::search(Predicate pred) const noexcept
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
	inline Slice<T> Slice<T, S>::get_left_at_first(const typename Slice<T, S>::ElementType &c, bool inclusive) const noexcept
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_left_at_last(const typename Slice<T, S>::ElementType &c, bool inclusive) const noexcept
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_first(const typename Slice<T, S>::ElementType &c, bool inclusive) const noexcept
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return Slice<T>(this->ptr + offset, length - offset);
	}
	template <typename T, bool S>
	inline Slice<T> Slice<T, S>::get_right_at_last(const typename Slice<T, S>::ElementType &c, bool inclusive) const noexcept
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return Slice<T>(this->ptr + offset, length - offset);
	}

	template <typename T, bool S>
	template <typename U>
	inline Slice<T> Slice<T, S>::get_left_at_first(Slice<U> s, bool inclusive) const noexcept
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T, bool S>
	template <typename U>
	inline Slice<T> Slice<T, S>::get_left_at_last(Slice<U> s, bool inclusive) const noexcept
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T, bool S>
	template <typename U>
	inline Slice<T> Slice<T, S>::get_right_at_first(Slice<U> s, bool inclusive) const noexcept
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return Slice<T>(this->ptr + offset, length - offset);
	}
	template <typename T, bool S>
	template <typename U>
	inline Slice<T> Slice<T, S>::get_right_at_last(Slice<U> s, bool inclusive) const noexcept
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return Slice<T>(this->ptr + offset, length - offset);
	}

	template <typename T, bool S>
	template <typename U>
	inline size_t Slice<T, S>::copy_to(Slice<U> dest) const noexcept
	{
		assert(dest.length >= length);
		for (size_t i = 0; i < length; ++i)
			dest.ptr[i] = ptr[i];
		return length;
	}

	template <typename T, bool S>
	template <bool SkipEmptyTokens>
	inline Slice<T> Slice<T, S>::pop_token(Slice<T> delimiters) noexcept
	{
		size_t offset = 0;
		if (SkipEmptyTokens)
		{
			while (offset < length && delimiters.exists(ptr[offset]))
				++offset;
		}
		size_t end = offset;
		while (end < length && !delimiters.exists(ptr[end]))
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
	inline Slice<Slice<T>> Slice<T, S>::tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters) noexcept
	{
		size_t numTokens = 0;
		size_t offset = 0;
		while (offset < length && numTokens < tokens.length)
		{
			if (!SkipEmptyTokens)
			{
				size_t tokStart = offset;
				while (offset < length && !delimiters.exists(ptr[offset]))
					++offset;
				tokens[numTokens++] = slice(tokStart, offset);
				++offset;
			}
			else
			{
				while (offset < length && delimiters.exists(ptr[offset]))
					++offset;
				if (offset == length)
					break;
				size_t tokStart = offset;
				while (offset < length && !delimiters.exists(ptr[offset]))
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
	inline size_t Slice<T, S>::tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters) noexcept
	{
		size_t numTokens = 0;
		size_t offset = 0;
		while (offset < length)
		{
			if (!SkipEmptyTokens)
			{
				size_t tokStart = offset;
				while (offset < length && !delimiters.exists(ptr[offset]))
					++offset;
				onToken(Slice<T>(ptr + tokStart, offset++), numTokens++);
			}
			else
			{
				while (offset < length && delimiters.exists(ptr[offset]))
					++offset;
				if (offset == length)
					break;
				size_t tokStart = offset;
				while (offset < length && !delimiters.exists(ptr[offset]))
					++offset;
				onToken(Slice<T>(ptr + tokStart, offset), numTokens++);
			}
		}
		return numTokens;
	}

	namespace detail
	{
		// TODO: this is pretty lame, use lookup table?
		constexpr bool is_whitespace(char c) noexcept { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

		constexpr char to_lower(char c) noexcept { return c >= 'A' && c <= 'Z' ? c | 0x20 : c; }
		constexpr char to_upper(char c) noexcept { return c >= 'a' && c <= 'z' ? c & ~0x20 : c; }

		inline size_t utf_decode(const unsigned char *utf8, char32_t *c) noexcept
		{
			if (utf8[0] < 128)
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
		: Slice<C, false>(c_str, c_str ? strlen(c_str) : 0) {}

//	template <typename C>
//	template <size_t N>
//	constexpr Slice<C, true>::Slice(C(&str_literal)[N]) noexcept
//		: Slice<C, false>(str_literal, N - 1) {} // literals don't work?!

#if !defined(NO_STL)
	template <typename C>
	template <class _Ty, class _Alloc>
	inline Slice<C, true>::Slice(std::vector<_Ty, _Alloc> &vec) noexcept
		: Slice<C, false>(vec.data(), vec.length()) {}

	template <typename C>
	template <class _Elem, class _Traits, class _Alloc>
	inline Slice<C, true>::Slice(std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
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
		assert(this->length > 0);
		char32_t r;
		detail::utf_decode(this->ptr, &r);
		return r;
	}
	template<>
	inline char32_t Slice<const unsigned char, true>::back_char() const noexcept
	{
		assert(this->length > 0);
		const unsigned char *last = ptr + length - 1;
		if (*last < 128)
			return *last;
		do --last;
		while ((*last & 0xC) == 0x80 && last >= this->ptr);
		char32_t r;
		detail::utf_decode(last, &r);
		return r;
	}
	template<>
	inline char32_t Slice<unsigned char, true>::back_char() const noexcept { return ((Slice<const unsigned char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<const char, true>::back_char() const noexcept { return ((Slice<const unsigned char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<char, true>::back_char() const noexcept { return ((Slice<const unsigned char, true>*)this)->back_char(); }
	template<>
	inline char32_t Slice<const char16_t, true>::back_char() const noexcept
	{
		assert(this->length > 0);
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
	inline char32_t Slice<const char32_t, true>::back_char() const noexcept { assert(this->length > 0); return ptr[length - 1]; }
	template<>
	inline char32_t Slice<char32_t, true>::back_char() const noexcept { assert(this->length > 0); return ptr[length - 1]; }

	template<typename C>
	inline char32_t Slice<C, true>::pop_front_char() noexcept
	{
		assert(this->length > 0);
		char32_t r;
		size_t codeUnits = detail::utf_decode(this->ptr, &r);
		this->ptr += codeUnits;
		this->length -= codeUnits;
		return r;
	}
	template<>
	inline char32_t Slice<const unsigned char, true>::pop_back_char() noexcept
	{
		assert(this->length > 0);
		if (ptr[length - 1] < 128)
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
	inline char32_t Slice<unsigned char, true>::pop_back_char() noexcept { return ((Slice<const unsigned char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<const char, true>::pop_back_char() noexcept { return ((Slice<const unsigned char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<char, true>::pop_back_char() noexcept { return ((Slice<const unsigned char, true>*)this)->pop_back_char(); }
	template<>
	inline char32_t Slice<const char16_t, true>::pop_back_char() noexcept
	{
		assert(this->length > 0);
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
	inline char32_t Slice<const char32_t, true>::pop_back_char() noexcept { assert(this->length > 0); return ptr[--length]; }
	template<>
	inline char32_t Slice<char32_t, true>::pop_back_char() noexcept { assert(this->length > 0); return ptr[--length]; }

	template<typename C>
	inline bool Slice<C, true>::eq_ic(Slice<C> str) const noexcept
	{
		if (this->length != str.length)
			return false;
		for (size_t i = 0; i < this->length; ++i)
			if (detail::to_lower(this->ptr[i]) != detail::to_lower(str.ptr[i]))
				return false;
		return true;
	}
	template<typename C>
	inline bool Slice<C, true>::begins_with_ic(Slice<C> str) const noexcept
	{
		if (this->length < str.length)
			return false;
		return Slice<C>(this->ptr, str.length).eq_ic(str);
	}
	template<typename C>
	inline bool Slice<C, true>::ends_with_ic(Slice<C> str) const noexcept
	{
		if (this->length < str.length)
			return false;
		return Slice<C>(this->ptr + this->length - str.length, str.length).eq_ic(str);
	}
	template<typename C>
	inline ptrdiff_t Slice<C, true>::cmp_ic(Slice<C> str) const noexcept
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

	template<typename C>
	inline size_t Slice<C, true>::find_first_ic(Slice<C> s) const noexcept
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
	inline size_t Slice<C, true>::find_last_ic(Slice<C> s) const noexcept
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

	template<typename C>
	inline Slice<C> Slice<C, true>::get_left_at_first_ic(Slice<C> s, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_left_at_last_ic(Slice<C> s, bool inclusive) const noexcept
	{
		size_t offset = find_last_ic(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return{ this->ptr, offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_right_at_first_ic(Slice<C> s, bool inclusive) const noexcept
	{
		size_t offset = find_first_ic(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return{ this->ptr + offset, this->length - offset };
	}
	template<typename C>
	inline Slice<C> Slice<C, true>::get_right_at_last_ic(Slice<C> s, bool inclusive) const noexcept
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
	inline uint32_t Slice<C, true>::hash(uint32_t hash) const noexcept
	{
		// TODO: is there a better hash for utf16/utf32?
		unsigned char *data = (unsigned char *)this->ptr;
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

	template <typename Dst, typename Src>
	constexpr Dst slice_cast(Src src) noexcept
	{
		static_assert(detail::is_slice<Src>::value && detail::is_slice<Dst>::value, "Dst and/or Src is not a Slice");
		assert(src.length * sizeof(typename Src::ElementType) % sizeof(typename Dst::ElementType) == 0);
		return Dst((typename Dst::ElementType*)src.ptr, src.length * sizeof(typename Src::ElementType) / sizeof(typename Dst::ElementType));
	}
}
