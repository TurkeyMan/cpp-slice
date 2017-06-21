/*
 * Authors: Manu Evans
 * Email:   turkeyman@gmail.com
 * License: BSD, go for broke!
 *
 * What is:
 * SharedArray is a reference counted version of Array, useful for array data
 * that will be shared arbitrarily.
 * SharedArray is 'structurally immutable', that is, the shape of the container
 * can not change, push/pop are disabled, eliminating the risk of data races.
 * SharedArray can be efficiently constructed from Array, which may be used to
 * build the initial dataset before it becomes shared.
 * Like Array, SharedArray is fully interactive with Slice.
 */

#pragma once

#include <array.h>

namespace beautifulcode
{
	template <typename T, bool IsString = detail::IsSomeChar<T>::value>
	struct SharedArray : public Slice<T>
	{
		using value_type = typename Slice<T>::value_type;
		using reference = typename Slice<T>::reference;
		using const_reference = typename Slice<T>::const_reference;
		using size_type = typename Slice<T>::size_type;
		using difference_type = typename Slice<T>::difference_type;
		using iterator = typename Slice<T>::iterator;
		using const_iterator = typename Slice<T>::const_iterator;

		SharedArray() noexcept;
		SharedArray(nullptr_t) noexcept;
		SharedArray(std::initializer_list<T> list);
		SharedArray(const SharedArray<T, IsString> &val);
		template <typename U, bool S> SharedArray(const SharedArray<U, S> &val);
		template <typename U, bool S> SharedArray(SharedArray<U, S> &&rval);
		template <typename U, size_t N, bool S> SharedArray(Array<U, N, S> &&rval);
		template <typename U> SharedArray(U *ptr, size_t length);
		template <typename U, bool S> SharedArray(Slice<U, S> slice);
		template <typename U, size_t N> SharedArray(U(&arr)[N]);
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> SharedArray(const std::vector<_Ty, _Alloc> &vec);
		template <class _Ty, class _Alloc> SharedArray(std::vector<_Ty, _Alloc> &&vec);
#endif
		SharedArray(Alloc_T, size_t count);
		template <typename... Items> SharedArray(Concat_T, Items&&... items);
		~SharedArray();

		size_t use_count() const;
		size_t incRef();
		size_t decRef();
		bool unique() const { return use_count() == 1; }

		void alloc(size_t count);
		void clear();

		Array<T> claim();
		template <size_t Count> Array<T, Count> clone() const { return Array<T, Count>(this->ptr, this->length); }

		bool operator==(SharedArray<T> arr) const;
		bool operator!=(SharedArray<T> arr) const;

		SharedArray<T, IsString>& operator=(const SharedArray<T, IsString> &arr);
		SharedArray<T, IsString>& operator=(SharedArray<T, IsString> &&rval);
		template <typename U, bool S> SharedArray<T, IsString>& operator=(const SharedArray<U, S> &arr);
		template <typename U, bool S> SharedArray<T, IsString>& operator=(SharedArray<U, S> &&rval);
		template <typename U, size_t N, bool S> SharedArray<T, IsString>& operator=(Array<U, N, S> &&rval);
		template <typename U, bool S> SharedArray<T, IsString>& operator=(Slice<U, S> arr);

		// delete unsafe methods
		reference pop_front() noexcept = delete;
		Slice<value_type> pop_front(size_t n) noexcept = delete;
		reference pop_back() noexcept = delete;
		Slice<value_type> pop_back(size_t n) noexcept = delete;

		template <bool SkipEmptyTokens = false>
		Slice<T> pop_token(Slice<const T> delimiters) noexcept = delete;
//		template <bool SkipEmptyTokens = false>
//		Slice<Slice<T>> tokenise(Slice<Slice<T>> tokens, Slice<const T> delimiters) noexcept = delete;
	};

	// specialisation for strings
	template <typename C>
	struct SharedArray<C, true> : public SharedArray<C, false>
	{
		static_assert(detail::IsSomeChar<C>::value, "Template argument C is not a character type!");

		using value_type = typename SharedArray<C, false>::value_type;
		using reference = typename SharedArray<C, false>::reference;
		using const_reference = typename SharedArray<C, false>::const_reference;
		using size_type = typename SharedArray<C, false>::size_type;
		using difference_type = typename SharedArray<C, false>::difference_type;
		using iterator = typename SharedArray<C, false>::iterator;
		using const_iterator = typename SharedArray<C, false>::const_iterator;

		// TODO: all these need to support construction from SomeChar<U>
		SharedArray() noexcept;
		SharedArray(nullptr_t) noexcept : SharedArray() {}
		SharedArray(const SharedArray<C, true> &val) noexcept;
		template <typename U, bool S> SharedArray(const SharedArray<U, S> &val) noexcept;
		template <typename U, bool S> SharedArray(SharedArray<U, S> &&rval) noexcept;
		template <typename U, size_t N, bool S> SharedArray(Array<U, N, S> &&rval) noexcept;
		template <typename U> SharedArray(U *ptr, size_t length) noexcept;
		template <typename U, bool S> SharedArray(Slice<U, S> slice) noexcept;
		template <typename U> SharedArray(const U *c_str) noexcept;
//		template <typename U, size_t N> SharedArray(U(&arr)[N]) noexcept;
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> SharedArray(const std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> SharedArray(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif
		SharedArray(Alloc_T, size_t count) noexcept;
		template <typename... Items> SharedArray(Concat_T, const Items&... items) noexcept;
		template <typename U> SharedArray(Sprintf_T, const U *format, ...) noexcept;

		SharedArray<C, true>& operator=(const SharedArray<C, true> &str) noexcept;
		template <typename U, bool S> SharedArray<C, true>& operator=(SharedArray<U, S> &&rval) noexcept;
		template <typename U, size_t N, bool S> SharedArray<C, true>& operator=(Array<U, N, S> &&rval) noexcept;
		template <typename U, bool S> SharedArray<C, true>& operator=(Slice<U, S> str) noexcept;
		template <typename U> SharedArray<C, true>& operator=(const U *c_str) noexcept;

		C* c_str() const noexcept { assert(!this->ptr || this->ptr[this->length] == 0); return this->ptr ? this->ptr : ""; }
	};

	using SharedString = SharedArray<const char>;
	using SharedWString = SharedArray<const char16_t>;
	using SharedDString = SharedArray<const char32_t>;

	// -------------------------------------------------------------------------------------------------
	// Implementation follows:
	//

	namespace detail
	{

	}

	template <typename T, bool S>
	inline SharedArray<T, S>::SharedArray() noexcept {}

	template <typename T, bool S>
	inline SharedArray<T, S>::SharedArray(nullptr_t) noexcept
		: Slice<T>(nullptr) {}

	template <typename T, bool S>
	inline SharedArray<T, S>::SharedArray(std::initializer_list<T> list)
		: SharedArray<T, S>(list.begin(), list.size()) {}

	template <typename T, bool S>
	inline SharedArray<T, S>::SharedArray(const SharedArray<T, S> &val)
		: Slice<T>(val.ptr, val.length)
	{
		if (this->ptr)
			++detail::get_array_header(this->ptr)->rc;
	}
	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>::SharedArray(const SharedArray<U, S> &val)
		: Slice<T>(val.ptr, val.length)
	{
		if (this->ptr)
			++detail::get_array_header(this->ptr)->rc;
	}

	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>::SharedArray(SharedArray<U, S> &&rval)
		: Slice<T, S>(rval.ptr, rval.length)
	{
		if (this != &rval)
		{
			rval.ptr = nullptr;
			rval.length = 0;
		}
	}
	template <typename T, bool IsString>
	template <typename U, size_t N, bool S>
	inline SharedArray<T, IsString>::SharedArray(Array<U, N, S> &&rval)
		: Slice<T, IsString>(rval.ptr, rval.length)
	{
		if (rval.ptr && !rval.is_allocated())
		{
			new(this) SharedArray<T, IsString>(Array<T, 0, IsString>(std::move(rval)));
		}
		else
		{
			rval.ptr = nullptr;
			rval.length = 0;
		}
	}

	template <typename T, bool S>
	template <typename U>
	inline SharedArray<T, S>::SharedArray(U *ptr, size_t length)
		: SharedArray<T, S>(Array<T, 0, S>(ptr, length)) {}

	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>::SharedArray(Slice<U, S> slice)
		: SharedArray<T, IsString>(Array<T, 0, IsString>(slice.ptr, slice.length)) {}

	template <typename T, bool S>
	template <typename U, size_t N>
	inline SharedArray<T, S>::SharedArray(U(&arr)[N])
		: SharedArray<T, S>(Array<T, 0, S>(arr, N)) {}

#if !defined(NO_STL)
	template <typename T, bool S>
	template <class _Ty, class _Alloc>
	inline SharedArray<T, S>::SharedArray(const std::vector<_Ty, _Alloc> &vec)
		: SharedArray<T, S>(Array<T, 0, S>(vec)) {}

	template <typename T, bool S>
	template <class _Ty, class _Alloc>
	inline SharedArray<T, S>::SharedArray(std::vector<_Ty, _Alloc> &&vec)
		: SharedArray<T, S>(Array<T, 0, S>(std::move(vec))) {}
#endif

	template <typename T, bool S>
	inline SharedArray<T, S>::SharedArray(Alloc_T, size_t count)
		: SharedArray<T, S>(Array<T, 0>(Alloc, count)) {}

	template <typename T, bool S>
	template <typename... Items>
	inline SharedArray<T, S>::SharedArray(Concat_T, Items&&... items)
		: SharedArray<T, S>(Array<T, 0>(Concat, std::forward<Items>(items)...)) {}

	template <typename T, bool S>
	inline SharedArray<T, S>::~SharedArray()
	{
		if (!this->ptr)
			return;
		clear();
#if !defined(NDEBUG)
		this->ptr = (T*)(size_t)0xFEEEFEEEFEEEFEEEull;
		this->length = (size_t)0xFEEEFEEEFEEEFEEEull;
#endif
	}

	template <typename T, bool S>
	inline size_t SharedArray<T, S>::use_count() const
	{
		if (this->ptr)
			return detail::get_array_header(this->ptr)->rc;
		else
			return 0;
	}
	template <typename T, bool S>
	inline size_t SharedArray<T, S>::incRef()
	{
		if (this->ptr)
			++detail::get_array_header(this->ptr)->rc;
	}
	template <typename T, bool S>
	inline size_t SharedArray<T, S>::decRef()
	{
		if (this->ptr)
		{
			detail::ArrayHeader *header = detail::get_array_header(this->ptr);
			if (header->rc == 1)
				clear();
			else
				--header->rc;
		}
	}

	template <typename T, bool S>
	inline void SharedArray<T, S>::alloc(size_t count)
	{
		clear();
		new(this) SharedArray<T, S>(Array<T, 0, S>(Alloc, count));
	}
	template <typename T, bool S>
	inline void SharedArray<T, S>::clear()
	{
		if (!this->ptr)
			return;
		detail::ArrayHeader *header = detail::get_array_header(this->ptr);
		if (header->rc > 1)
			--header->rc;
		else
		{
			for (size_t i = 0; i < this->length; ++i)
				this->ptr[i].~T();
			detail::free_array(this->ptr);
		}
		this->length = 0;
		this->ptr = nullptr;
	}

	template <typename T, bool S>
	inline Array<T> SharedArray<T, S>::claim()
	{
		if (use_count() > 1)
			throw std::exception();
		Array<T> arr;
		if (this->length > 0)
		{
			arr.ptr = this->ptr;
			this->ptr = nullptr;
			arr.length = this->length;
			this->length = 0;
		}
		return arr;
	}

	template <typename T, bool S>
	inline bool SharedArray<T, S>::operator==(SharedArray<T> arr) const
	{
		return this->ptr == arr.ptr; // note: SharedArray instances all have the same length
	}
	template <typename T, bool S>
	inline bool SharedArray<T, S>::operator!=(SharedArray<T> arr) const
	{
		return this->ptr != arr.ptr; // note: SharedArray instances all have the same length
	}

	template <typename T, bool S>
	inline SharedArray<T, S>& SharedArray<T, S>::operator=(const SharedArray<T, S> &arr)
	{
		if (!arr.ptr)
		{
			clear();
		}
		else if (arr.ptr != this->ptr)
		{
			clear();
			this->ptr = arr.ptr;
			this->length = arr.length;
			++detail::get_array_header(this->ptr)->rc;
		}
		return *this;
	}
	template <typename T, bool S>
	inline SharedArray<T, S>& SharedArray<T, S>::operator=(SharedArray<T, S> &&rval)
	{
		if (this != &rval)
		{
			clear();
			new(this) SharedArray<T, S>(std::move(rval));
		}
		return *this;
	}

	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>& SharedArray<T, IsString>::operator=(const SharedArray<U, S> &arr)
	{
		if (!arr.ptr)
		{
			clear();
		}
		else if (arr.ptr != this->ptr)
		{
			clear();
			this->ptr = arr.ptr;
			this->length = arr.length;
			++detail::get_array_header(this->ptr)->rc;
		}
		return *this;
	}
	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>& SharedArray<T, IsString>::operator=(SharedArray<U, S> &&rval)
	{
		if (this != &rval)
		{
			clear();
			new(this) SharedArray<T, IsString>(std::move(rval));
		}
		return *this;
	}

	template <typename T, bool IsString>
	template <typename U, size_t N, bool S>
	inline SharedArray<T, IsString>& SharedArray<T, IsString>::operator=(Array<U, N, S> &&rval)
	{
		clear();
		new(this) SharedArray<T, IsString>(std::move(rval));
		return *this;
	}

	template <typename T, bool IsString>
	template <typename U, bool S>
	inline SharedArray<T, IsString>& SharedArray<T, IsString>::operator=(Slice<U, S> arr)
	{
		clear();
		new(this) SharedArray<T, IsString>(arr.ptr, arr.length);
		return *this;
	}


	// string methods...

	namespace detail
	{
	}

	template <typename C>
	inline SharedArray<C, true>::SharedArray() noexcept
		: SharedArray<C, false>() {}

	template <typename C>
	inline SharedArray<C, true>::SharedArray(const SharedArray<C, true> &val) noexcept
		: SharedArray<C, false>(val) {}

	template <typename C>
	template <typename U, bool S>
	inline SharedArray<C, true>::SharedArray(SharedArray<U, S> &&rval) noexcept
	{
		this->ptr = rval.ptr;
		this->length = rval.length;
		rval.ptr = nullptr;
		rval.length = 0;
	}

	template <typename C>
	template <typename U, size_t N, bool S>
	inline SharedArray<C, true>::SharedArray(Array<U, N, S> &&rval) noexcept
	{
		if (rval.is_allocated())
		{
			// claim allocation
			this->ptr = rval.ptr;
			this->length = rval.length;
			rval.ptr = nullptr;
			rval.length = 0;
		}
		else if (rval.length)
			new(this) Array<C, 0, true>(rval.ptr, rval.length);
	}

	template <typename C>
	template <typename U>
	inline SharedArray<C, true>::SharedArray(U *ptr, size_t length) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(ptr, length)) {}

	template <typename C>
	template <typename U, bool S>
	inline SharedArray<C, true>::SharedArray(Slice<U, S> slice) noexcept
		: SharedArray<C, true>(slice.ptr, slice.length) {}

	template <typename C>
	template <typename U>
	inline SharedArray<C, true>::SharedArray(const U *c_str) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(c_str)) {}

#if !defined(NO_STL)
	template <typename C>
	template <class _Ty, class _Alloc>
	inline SharedArray<C, true>::SharedArray(const std::vector<_Ty, _Alloc> &vec) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(vec.data(), vec.size())) {}

	template <typename C>
	template <class _Elem, class _Traits, class _Alloc>
	inline SharedArray<C, true>::SharedArray(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(str.data(), str.length())) {}
#endif

	template <typename C>
	inline SharedArray<C, true>::SharedArray(Alloc_T, size_t count) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(Alloc, count)) {}

	template <typename C>
	template <typename... Items>
	inline SharedArray<C, true>::SharedArray(Concat_T, const Items&... items) noexcept
		: SharedArray<C, true>(Array<C, 0, true>(Concat, items...)) {}

	template <typename C>
	template <typename U>
	inline SharedArray<C, true>::SharedArray(Sprintf_T, const U *format, ...) noexcept
	{
		va_list args;
		va_start(args, format);
		Array<C, 0, true> arr;
		arr.do_sprintf(format, args);
		new(this) SharedArray<C, true>(std::move(arr));
		va_end(args);
	}

	template <typename C>
	inline SharedArray<C, true>& SharedArray<C, true>::operator=(const SharedArray<C, true> &str) noexcept
	{
		if (!str.ptr)
		{
			this->clear();
		}
		else if (str.ptr != this->ptr)
		{
			this->clear();
			this->ptr = str.ptr;
			this->length = str.length;
			++detail::get_array_header(this->ptr)->rc;
		}
		return *this;
	}
	template <typename C>
	template <typename U, bool S>
	inline SharedArray<C, true>& SharedArray<C, true>::operator=(SharedArray<U, S> &&rval) noexcept
	{
		if (&rval != this)
		{
			this->clear();
			new(this) SharedArray<C, true>(std::move(rval));
		}
		return *this;
	}
	template <typename C>
	template <typename U, size_t N, bool S>
	inline SharedArray<C, true>& SharedArray<C, true>::operator=(Array<U, N, S> &&rval) noexcept
	{
		this->clear();
		new(this) SharedArray<C, true>(std::move(rval));
		return *this;
	}
	template <typename C>
	template <typename U, bool S>
	inline SharedArray<C, true>& SharedArray<C, true>::operator=(Slice<U, S> str) noexcept
	{
		this->clear();
		new(this) SharedArray<C, true>(str.ptr, str.length);
		return *this;
	}
	template <typename C>
	template <typename U>
	inline SharedArray<C, true>& SharedArray<C, true>::operator=(const U *c_str) noexcept
	{
		this->clear();
		new(this) SharedArray<C, true>(c_str);
		return *this;
	}
}
