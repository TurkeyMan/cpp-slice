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

namespace beautifulcode
{
	namespace detail
	{
		template <typename T> struct SliceElementType	{ using Ty = T; };
		template <> struct SliceElementType<void>		{ using Ty = uint8_t; };
		template <> struct SliceElementType<const void>	{ using Ty = const uint8_t; };
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

	template <typename T>
	struct Slice
	{
	public:
		using ElementType = typename detail::SliceElementType<T>::Ty;

		typedef bool(Predicate)(const ElementType &e);

		size_t length;
		T *ptr;

		Slice();
		Slice(nullptr_t);
		Slice(T* ptr, size_t length);
		template <typename U> Slice(Slice<U> slice);
		template <typename U, size_t N> Slice(U(&arr)[N]);
		explicit Slice(std::initializer_list<ElementType> list);

		template <typename U> Slice<T>& operator=(Slice<U> slice);

		ElementType& at(size_t i) const;
		ElementType& operator[](size_t i) const	{ return at(i); }

		Slice<T> slice(size_t first, size_t last) const;

		T *data() const		{ return ptr; }
		size_t size() const	{ return length; }
		bool empty() const	{ return length == 0; }
		void clear()		{ ptr = nullptr; length = 0; }

		explicit operator bool() const { return length != 0; }

		bool operator==(Slice<const T> slice) const { return ptr == slice.ptr && length == slice.length; }
		bool operator!=(Slice<const T> slice) const { return ptr != slice.ptr || length != slice.length; }

		template <typename U> bool eq(Slice<U> slice) const;
		template <typename U> ptrdiff_t cmp(Slice<U> slice) const;

		template <typename U> bool begins_with(Slice<U> slice) const;
		template <typename U> bool ends_with(Slice<U> slice) const;

		Iterator<T> begin() const	{ return Iterator<T>(&ptr[0]); }
		Iterator<T> end() const		{ return Iterator<T>(&ptr[length]); }

		ElementType& front() const					{ return ((ElementType*)ptr)[0]; }
		Slice<ElementType> front(size_t n) const	{ return slice(0, n); }
		ElementType& back() const					{ return ((ElementType*)ptr)[length - 1]; }
		Slice<ElementType> back(size_t n) const		{ return slice(length - n, length); }

		ElementType& pop_front();
		Slice<ElementType> pop_front(size_t n);
		ElementType& pop_back();
		Slice<ElementType> pop_back(size_t n);

		Slice<T> drop_front(size_t n) const	{ return slice(n, length); }
		Slice<T> drop_back(size_t n) const	{ return slice(0, length - n); }

		bool exists(const ElementType &c, size_t *index = nullptr) const;
		size_t find_first(const ElementType &c) const;
		template <typename U> size_t find_first(Slice<U> s) const;
		size_t find_last(const ElementType &c) const;
		template <typename U> size_t find_last(Slice<U> s) const;

		T *search(Predicate *predFunc) const;

		ptrdiff_t index_of_element(const T *c) const;

		Slice<T> get_left_at_first(const ElementType &c, bool inclusive = false) const;
		Slice<T> get_left_at_last(const ElementType &c, bool inclusive = false) const;
		Slice<T> get_right_at_first(const ElementType &c, bool inclusive = true) const;
		Slice<T> get_right_at_last(const ElementType &c, bool inclusive = true) const;

		template <typename U> Slice<T> get_left_at_first(Slice<U> s, bool inclusive = false) const;
		template <typename U> Slice<T> get_left_at_last(Slice<U> s, bool inclusive = false) const;
		template <typename U> Slice<T> get_right_at_first(Slice<U> s, bool inclusive = true) const;
		template <typename U> Slice<T> get_right_at_last(Slice<U> s, bool inclusive = true) const;

		template <typename U>
		size_t copy_to(Slice<U> dest) const;

		template <bool SkipEmptyTokens = false>
		Slice<T> pop_token(Slice<T> delimiters);

		template <bool SkipEmptyTokens = false>
		Slice<Slice<T>> tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters);
		template <bool SkipEmptyTokens = false>
		size_t tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters);
	};

	template <typename T>
	inline bool operator==(nullptr_t, Slice<T> slice) { return slice == nullptr; }
	template <typename T>
	inline bool operator!=(nullptr_t, Slice<T> slice) { return slice == nullptr; }

	// Cast from a Slice<T> to a Slice<U> where T and U are compatible types.	Its typical use is for casting from a Slice<void>.
	template <typename Dst, typename Src>
	Dst slice_cast(Src src);


	//
	// Implementation follows:
	//

	template <typename T>
	inline Slice<T>::Slice()
		: length(0), ptr(nullptr) {}

	template <typename T>
	inline Slice<T>::Slice(nullptr_t)
		: length(0), ptr(nullptr) {}

	template <typename T>
	inline Slice<T>::Slice(T* ptr, size_t length) : length(length), ptr(ptr) {}

	template <typename T>
	template <typename U>
	inline Slice<T>::Slice(Slice<U> slice)
		: length(slice.length), ptr(slice.ptr) {}

	template <typename T>
	template <typename U, size_t N>
	inline Slice<T>::Slice(U(&arr)[N])
		: length(N), ptr(arr) {}

	template <typename T>
	inline Slice<T>::Slice(std::initializer_list<ElementType> list)
		: length(list.size()), ptr(list.begin()) {}

	template <typename T>
	template <typename U>
	inline Slice<T> &Slice<T>::operator=(Slice<U> slice)
	{
		length = slice.length;
		ptr = slice.ptr;
		return *this;
	}

	template <typename T>
	inline typename Slice<T>::ElementType& Slice<T>::at(size_t i) const
	{
		assert(offset < length);
		return ((ElementType*)ptr)[offset];
	}

	template <typename T>
	inline Slice<T> Slice<T>::slice(size_t first, size_t last) const
	{
		assert(first <= last && last <= length);
		return Slice<T>((ElementType*)ptr + first, last - first);
	}

	template <typename T>
	template <typename U>
	inline bool Slice<T>::eq(Slice<U> slice) const
	{
		if (length != slice.length)
			return false;
		for (size_t i = 0; i < length; ++i)
			if (((ElementType*)ptr)[i] != ((ElementType*)slice.ptr)[i])
				return false;
		return true;
	}

	template <typename T>
	template <typename U>
	inline ptrdiff_t Slice<T>::cmp(Slice<U> slice) const
	{
		size_t len = length < slice.length ? length : slice.length;
		for (size_t i = 0; i < len; ++i)
		{
			if (((ElementType*)ptr)[i] != ((ElementType*)slice.ptr)[i])
				return ((ElementType*)ptr)[i] < ((ElementType*)slice.ptr)[i] ? -1 : 1;
		}
		return length - slice.length;
	}

	template <typename T>
	template <typename U>
	inline bool Slice<T>::begins_with(Slice<U> slice) const
	{
		if (length < slice.length)
			return false;
		return Slice<T>(0, slice.length).eq(slice);
	}
	template <typename T>
	template <typename U>
	inline bool Slice<T>::ends_with(Slice<U> slice) const
	{
		if (length < slice.length)
			return false;
		return Slice<T>(length - slice.length, length).eq(slice);
	}

	template <typename T>
	inline typename Slice<T>::ElementType& Slice<T>::pop_front()
	{
		assert(length > 0);
		++ptr;
		--length;
		return ((ElementType*)ptr)[-1];
	}
	template <typename T>
	inline Slice<typename Slice<T>::ElementType> Slice<T>::pop_front(size_t n)
	{
		assert(length >= n);
		ptr += n;
		length -= n;
		return Slice<T>(ptr - n, n);
	}
	template <typename T>
	inline typename Slice<T>::ElementType& Slice<T>::pop_back()
	{
		assert(length > 0);
		--length;
		return ((ElementType*)ptr)[length];
	}
	template <typename T>
	inline Slice<typename Slice<T>::ElementType> Slice<T>::pop_back(size_t n)
	{
		assert(length >= n);
		length -= n;
		return Slice<T>(ptr + length, n);
	}

	template <typename T>
	inline bool Slice<T>::exists(const typename Slice<T>::ElementType &c, size_t *index) const
	{
		size_t i = find_first(c);
		if (index)
			*index = i;
		return i != length;
	}

	namespace detail
	{
		// hack to handle slices of slices
		template <typename T> struct FindImpl					{ static inline bool eq(const T &a, const T &b) { return a == b; } };
		template <typename U> struct FindImpl<Slice<U>>			{ static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
		template <typename U> struct FindImpl<const Slice<U>>	{ static inline bool eq(const Slice<U> &a, const Slice<U> &b) { return a.eq(b); } };
	}
	template <typename T>
	inline size_t Slice<T>::find_first(const typename Slice<T>::ElementType &c) const
	{
		size_t offset = 0;
		while (offset < length && !detail::FindImpl<typename Slice<T>::ElementType>::eq(ptr[offset], c))
			++offset;
		return offset;
	}
	template <typename T>
	inline size_t Slice<T>::find_last(const typename Slice<T>::ElementType &c) const
	{
		ptrdiff_t last = length - 1;
		while (last >= 0 && !detail::FindImpl<typename Slice<T>::ElementType>::eq(ptr[last], c))
			--last;
		return last < 0 ? length : last;
	}
	template <typename T>
	template <typename U>
	inline size_t Slice<T>::find_first(Slice<U> s) const
	{
		if (s.empty())
			return 0;
		ptrdiff_t len = length - s.length;
		for (ptrdiff_t i = 0; i < len; ++i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T>::ElementType>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}
	template <typename T>
	template <typename U>
	inline size_t Slice<T>::find_last(Slice<U> s) const
	{
		if (s.empty())
			return length;
		for (ptrdiff_t i = length - s.length; i >= 0; --i)
		{
			size_t j = 0;
			for (; j < s.length; ++j)
			{
				if (!detail::FindImpl<typename Slice<T>::ElementType>::eq(ptr[i + j], s.ptr[j]))
					break;
			}
			if (j == s.length)
				return i;
		}
		return length;
	}

	template <typename T>
	inline T* Slice<T>::search(Predicate pred) const
	{
		for (size_t i = 0; i < length ++ i)
		{
			if (pred(ptr[i]))
				return ptr + i;
		}
		return nullptr;
	}

	template <typename T>
	inline ptrdiff_t Slice<T>::index_of_element(const T *c) const
	{
		if (c >= ptr && c < ptr + length)
			return c - ptr;
		return -1;
	}

	template <typename T>
	inline Slice<T> Slice<T>::get_left_at_first(const typename Slice<T>::ElementType &c, bool inclusive) const
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T>
	inline Slice<T> Slice<T>::get_left_at_last(const typename Slice<T>::ElementType &c, bool inclusive) const
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 1 : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T>
	inline Slice<T> Slice<T>::get_right_at_first(const typename Slice<T>::ElementType &c, bool inclusive) const
	{
		size_t offset = find_first(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return Slice<T>(this->ptr + offset, length - offset);
	}
	template <typename T>
	inline Slice<T> Slice<T>::get_right_at_last(const typename Slice<T>::ElementType &c, bool inclusive) const
	{
		size_t offset = find_last(c);
		if (offset != this->length)
			offset += inclusive ? 0 : 1;
		return Slice<T>(this->ptr + offset, length - offset);
	}

	template <typename T>
	template <typename U>
	inline Slice<T> Slice<T>::get_left_at_first(Slice<U> s, bool inclusive) const
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T>
	template <typename U>
	inline Slice<T> Slice<T>::get_left_at_last(Slice<U> s, bool inclusive) const
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? s.length : 0;
		return Slice<T>(this->ptr, offset);
	}
	template <typename T>
	template <typename U>
	inline Slice<T> Slice<T>::get_right_at_first(Slice<U> s, bool inclusive) const
	{
		size_t offset = find_first(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return Slice<T>(this->ptr + offset, length - offset);
	}
	template <typename T>
	template <typename U>
	inline Slice<T> Slice<T>::get_right_at_last(Slice<U> s, bool inclusive) const
	{
		size_t offset = find_last(s);
		if (offset != this->length)
			offset += inclusive ? 0 : s.length;
		return Slice<T>(this->ptr + offset, length - offset);
	}

	template <typename T>
	template <typename U>
	inline size_t Slice<T>::copy_to(Slice<U> dest) const
	{
		assert(dest.length >= length);
		for (size_t i = 0; i < length; ++i)
			dest.ptr[i] = ptr[i];
		return length;
	}

	template <typename T>
	template <bool SkipEmptyTokens>
	inline Slice<T> Slice<T>::pop_token(Slice<T> delimiters)
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

	template <typename T>
	template <bool SkipEmptyTokens>
	inline Slice<Slice<T>> Slice<T>::tokenise(Slice<Slice<T>> tokens, Slice<T> delimiters)
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
	template <typename T>
	template <bool SkipEmptyTokens>
	inline size_t Slice<T>::tokenise(std::function<void(Slice<T> token, size_t index)> onToken, Slice<T> delimiters)
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

	namespace detail {
		template <typename T> struct is_slice					{ enum { value = 0 }; };
		template <typename T> struct is_slice<Slice<T>>			{ enum { value = 1 }; };
		template <typename T> struct is_slice<const Slice<T>>	{ enum { value = 1 }; };
	}

	template <typename Dst, typename Src>
	inline Dst slice_cast(Src src)
	{
		static_assert(detail::is_slice<Src>::value && detail::is_slice<Dst>::value, "Dst and/or Src is not a Slice");
		assert(src.length * sizeof(typename Src::ElementType) % sizeof(typename Dst::ElementType) == 0);
		return Dst((typename Dst::ElementType*)src.ptr, src.length * sizeof(typename Src::ElementType) / sizeof(typename Dst::ElementType));
	}
}
