/*
 * Authors: Manu Evans
 * Email:   turkeyman@gmail.com
 * License: BSD, go for broke!
 *
 * What is:
 * Array is a memory-managed array, similar to std::vector, except more convenient.
 * There is an optional template argument which reserves some number of elements in the
 * array structure which will be used for small arrays before overflowing into an
 * allocation. This is convenient for use as over-flow safe stack-buffers, or just to
 * efficiently manage small arrays without performing many small allocations.
 * Array interacts seamlessly with Slice.
 */

#pragma once

#include <slice.h>

#include <type_traits>
#include <cstdarg>

#if !defined(SLICE_ALLOC)
# define SLICE_ALLOC(bytes) malloc(bytes)
#endif
#if !defined(SLICE_FREE)
# define SLICE_FREE(ptr) free(ptr)
#endif

namespace beautifulcode
{
	enum Reserve_T { Reserve };
	enum Alloc_T { Alloc };
	enum Concat_T { Concat };
	enum Sprintf_T { Sprintf };

	template <typename T, size_t Count = 0, bool IsString = detail::IsSomeChar<T>::value>
	struct Array : public Slice<T>
	{
		using value_type = typename Slice<T>::value_type;
		using reference = typename Slice<T>::reference;
		using const_reference = typename Slice<T>::const_reference;
		using size_type = typename Slice<T>::size_type;
		using difference_type = typename Slice<T>::difference_type;
		using iterator = typename Slice<T>::iterator;
		using const_iterator = typename Slice<T>::const_iterator;

		Array() noexcept;
		Array(nullptr_t) noexcept;
		Array(std::initializer_list<T> list);
		Array(const Array<T, Count, IsString> &val);
		template <typename U, size_t N, bool S> Array(Array<U, N, S> &&rval);
		template <typename U> Array(U *ptr, size_t length);
		template <typename U, bool S> Array(Slice<U, S> slice);
		template <typename U, size_t N> Array(U(&arr)[N]);
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Array(const std::vector<_Ty, _Alloc> &vec);
		template <class _Ty, class _Alloc> Array(std::vector<_Ty, _Alloc> &&vec);
#endif
		Array(Alloc_T, size_t count);
		Array(Reserve_T, size_t count);
		template <typename... Items> Array(Concat_T, Items&&... items);
		~Array();

		Array<T, Count, IsString>& operator=(const Array<T, Count, IsString> &arr);
		template <typename U, size_t N, bool S> Array<T, Count, IsString>& operator=(Array<U, N, S> &&rval);
		template <typename U, bool S> Array<T, Count, IsString>& operator=(Slice<U, S> arr);

		void reserve(size_t count);
		void alloc(size_t count);
		void resize(size_t count);
		void clear();

		template <typename... Items> Array<T, Count, IsString>& append(Items&&... items);

		template <typename U> reference push_back(U &&item);
		template <typename... Args> reference emplace_back(Args&&... args);
		value_type pop_back();
		void pop_back(size_t n);

		reference pop_front() noexcept = delete;
		reference pop_front(size_t n) noexcept = delete;

		value_type remove(size_t i);
		void remove(const value_type *item);
		void remove_first(const_reference item) { remove(this->find_first(item)); }

		value_type remove_swap_last(size_t i);
		void remove_swap_last(const value_type *item);
		void remove_first_swap_last(const_reference item) { remove_swap_last(this->find_first(item)); }

		Slice<T> get_buffer() const noexcept;

	private:
		template <typename U, size_t N, bool S>
		friend struct Array;
		template <typename U, bool S>
		friend struct SharedArray;

		template <size_t Len, bool = true>
		struct LocalBuffer
		{
			char local[sizeof(T) * Len];
			T* ptr() const noexcept { return (T*)local; }
			bool is_allocated(T *p) const noexcept { return p != (T*)local && p != nullptr; }
		};
		template <bool MakeWork> struct LocalBuffer<0, MakeWork>
		{
			T* ptr() const noexcept { return nullptr; }
			bool is_allocated(T *p) const noexcept { return p != nullptr; }
		};
		LocalBuffer<Count> local;

		bool is_allocated() const noexcept { return local.is_allocated(this->ptr); }
	};

	// specialisation for strings
	template <typename C, size_t Count>
	struct Array<C, Count, true> : public Array<C, Count, false>
	{
		static_assert(detail::IsSomeChar<C>::value, "Template argument C is not a character type!");

		using value_type = typename Array<C, Count, false>::value_type;
		using reference = typename Array<C, Count, false>::reference;
		using const_reference = typename Array<C, Count, false>::const_reference;
		using size_type = typename Array<C, Count, false>::size_type;
		using difference_type = typename Array<C, Count, false>::difference_type;
		using iterator = typename Array<C, Count, false>::iterator;
		using const_iterator = typename Array<C, Count, false>::const_iterator;

		// TODO: all these need to support construction from SomeChar<U>
		Array() noexcept;
		Array(const Array<C, Count, true> &val) noexcept;
		template <typename U, size_t N, bool S> Array(Array<U, N, S> &&rval) noexcept;
		template <typename U> Array(U *ptr, size_t length) noexcept;
		template <typename U, bool S> Array(Slice<U, S> slice) noexcept;
		template <typename U> Array(const U *c_str) noexcept;
//		template <typename U, size_t N> Array(U(&arr)[N]);
#if !defined(NO_STL)
		template <class _Ty, class _Alloc> Array(const std::vector<_Ty, _Alloc> &vec) noexcept;
		template <class _Elem, class _Traits, class _Alloc> Array(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept;
#endif
		Array(Alloc_T, size_t count) noexcept;
		Array(Reserve_T, size_t count) noexcept;
		template <typename... Items> Array(Concat_T, const Items&... items) noexcept;
		template <typename U> Array(Sprintf_T, const U *format, ...) noexcept;

		Array<C, Count, true>& operator=(const Array<C, Count, true> &str) noexcept;
		template <typename U, size_t N, bool S> Array<C, Count, true>& operator=(Array<U, N, S> &&rval) noexcept;
		template <typename U, bool S> Array<C, Count, true>& operator=(Slice<U, S> str) noexcept;
		template <typename U> Array<C, Count, true>& operator=(const U *c_str) noexcept;

		template <typename... Items> Array<C, Count, true>& append(const Items&... items) noexcept;

		template <typename U> Array<C, Count, true>& sprintf(const U *format, ...) noexcept;
		// should there be an appending sprintf?

		Array<C, Count, true>& to_upper() noexcept;
		Array<C, Count, true>& to_lower() noexcept;

		template <typename U, bool S>
		Array<C, Count, true>& url_encode(Slice<U, S> text);
		template <typename U, bool S>
		Array<C, Count, true>& url_decode(Slice<U, S> url);

	private:
		template <typename U, bool S>
		friend struct SharedArray;
		template <typename U> void do_sprintf(const U *format, va_list args) noexcept;
	};

	template <size_t Count = 0>
	using MutableString = Array<char, Count>;
	template <size_t Count = 0>
	using MutableWString = Array<char16_t, Count>;
	template <size_t Count = 0>
	using MutableDString = Array<char32_t, Count>;

	using MutableString64 = MutableString<64 - sizeof(MutableString<0>)>;
	using MutableString128 = MutableString<128 - sizeof(MutableString<0>)>;
	using MutableString256 = MutableString<256 - sizeof(MutableString<0>)>;


	// -------------------------------------------------------------------------------------------------
	// Implementation follows:
	//

	namespace detail
	{
		struct ArrayHeader
		{
			enum Flags
			{
				None = 0,
				RefCounted = 1
			};

			using FreeFunc = void(void*);// noexcept; // TODO: VS2015 doesn't support this?
			FreeFunc *freeFunc;
			uint64_t rc : 21,
			         bytes : 40,
			         flags : 3;
		};

		inline ArrayHeader* get_array_header(const void *buffer) noexcept
		{
			return (ArrayHeader*)buffer - 1;
		}

		template <typename T>
		inline T* alloc_array(size_t bytes, ArrayHeader::Flags flags) noexcept
		{
			ArrayHeader *hdr = (ArrayHeader*)SLICE_ALLOC(sizeof(ArrayHeader) + bytes);
			hdr->freeFunc = [](void *mem) { SLICE_FREE(mem); };
			hdr->rc = 1;
			hdr->bytes = bytes;
			hdr->flags = flags;
			return (T*)(hdr + 1);
		}

		template <typename T>
		inline void free_array(T *pArray) noexcept
		{
			ArrayHeader *hdr = get_array_header(pArray);
			hdr->freeFunc(hdr);
		}

		// set of functions that count elements for appending
		template<typename T>
		constexpr size_t count() noexcept { return 0; }
		template<typename T, typename U, bool S, typename... Args>
		inline auto count(Slice<U, S> slice, Args&&... args) -> decltype(T(slice[0]), size_t());
		template<typename T, typename U, size_t N, bool S, typename... Args>
		inline auto count(Array<U, N, S> &&arr, Args&&... args) -> decltype(T(std::move(arr[0])), size_t());
		template<typename T, typename U, typename... Args>
		inline auto count(U &&item, Args&&... args) -> decltype(T(std::forward<U>(item)), size_t())
		{
			return count<T>(std::forward<Args>(args)...) + 1;
		}
		template<typename T, typename U, size_t N, bool S, typename... Args>
		inline auto count(Array<U, N, S> &&arr, Args&&... args) -> decltype(T(std::move(arr[0])), size_t())
		{
			return count<T>(std::forward<Args>(args)...) + arr.length;
		}
		template<typename T, typename U, bool S, typename... Args>
		inline auto count(Slice<U, S> slice, Args&&... args) -> decltype(T(slice[0]), size_t())
		{
			return count<T>(std::forward<Args>(args)...) + slice.length;
		}

		// set of functions that append elements
		template<typename T> T* return_t_star();
		template<typename T>
		inline T* append(T *buffer) { return buffer; }
		template<typename T, typename U, bool S, typename... Args>
		inline auto append(T *buffer, Slice<U, S> s, Args&&... args) -> decltype(T(s.ptr[0]), return_t_star<T>());
		template<typename T, typename U, size_t N, bool S, typename... Args>
		inline auto append(T *buffer, Array<U, N, S> &&a, Args&&... args) -> decltype(T(std::move(a.ptr[0])), return_t_star<T>());
		template<typename T, typename U, typename... Args>
		inline auto append(T *buffer, U &&a, Args&&... args) -> decltype(T(std::forward<U>(a)), return_t_star<T>())
		{
			new((void*)buffer) T(std::forward<U>(a));
			append(buffer + 1, std::forward<Args>(args)...);
			return buffer;
		}
		template<typename T, typename U, size_t N, bool S, typename... Args>
		inline auto append(T *buffer, Array<U, N, S> &&a, Args&&... args) -> decltype(T(std::move(a.ptr[0])), return_t_star<T>())
		{
			for (size_t i = 0; i < a.length; ++i)
				new((void*)(buffer + i)) T(std::move(a.ptr[i]));
			append(buffer + a.length, std::forward<Args>(args)...);
			return buffer;
		}
		template<typename T, typename U, bool S, typename... Args>
		inline auto append(T *buffer, Slice<U, S> s, Args&&... args) -> decltype(T(s.ptr[0]), return_t_star<T>())
		{
			for (size_t i = 0; i < s.length; ++i)
				new((void*)(buffer + i)) T(s.ptr[i]);
			append(buffer + s.length, std::forward<Args>(args)...);
			return buffer;
		}
	}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array() noexcept {}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array(nullptr_t) noexcept
		: Slice<T>(nullptr) {}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array(Alloc_T, size_t count)
	{
		alloc(count);
	}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array(Reserve_T, size_t count)
	{
		reserve(count);
	}

	template <typename T, size_t Count, bool S>
	template <typename... Items>
	inline Array<T, Count, S>::Array(Concat_T, Items&&... items)
	{
		this->length = detail::count<value_type>(std::forward<Items>(items)...);
		this->ptr = detail::append<value_type>(detail::alloc_array<value_type>(this->length * sizeof(value_type), detail::ArrayHeader::None), std::forward<Items>(items)...);
	}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array(std::initializer_list<T> list)
		: Array<T, Count, S>(list.begin(), list.size()) {}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::Array(const Array<T, Count, S> &val)
		: Array<T, Count, S>(val.ptr, val.length) {}

	template <typename T, size_t Count, bool IsString>
	template <typename U, size_t N, bool S>
	inline Array<T, Count, IsString>::Array(Array<U, N, S> &&rval)
	{
		if (rval.is_allocated())
		{
			// claim allocated buffers
			this->length = rval.length;
			rval.length = 0;
			this->ptr = rval.ptr;
			rval.ptr = nullptr;
		}
		else if (rval.length)
		{
			if (rval.length <= Count)
				this->ptr = (T*)local.ptr();
			else
				reserve(rval.length);
			this->length = rval.length;

			if (std::is_pod<T>::value)
				memcpy((void*)this->ptr, rval.ptr, sizeof(T)*this->length);
			else
			{
				for (size_t i = 0; i < this->length; ++i)
					new((void*)&this->ptr[i]) T(std::move(rval.ptr[i]));
			}
		}
	}

	template <typename T, size_t Count, bool S>
	template <typename U>
	inline Array<T, Count, S>::Array(U *ptr, size_t length)
		: Slice<T>()
	{
		reserve(length);
		this->length = length;
		if (std::is_pod<T>::value &&
			std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<U>::type>::value)
		{
			memcpy((void*)this->ptr, ptr, sizeof(T)*length);
		}
		else
		{
			for (size_t i = 0; i < length; ++i)
				new((void*)&this->ptr[i]) T(ptr[i]);
		}
	}

	template <typename T, size_t Count, bool IsString>
	template <typename U, bool S>
	inline Array<T, Count, IsString>::Array(Slice<U, S> slice)
		: Array<T, Count, S>(slice.ptr, slice.length) {}

	template <typename T, size_t Count, bool S>
	template <typename U, size_t N>
	inline Array<T, Count, S>::Array(U(&arr)[N])
		: Array<T, Count, S>(arr, N) {}

#if !defined(NO_STL)
	template <typename T, size_t Count, bool S>
	template <class _Ty, class _Alloc>
	inline Array<T, Count, S>::Array(const std::vector<_Ty, _Alloc> &vec)
		: Array<T, Count, S>(vec.data(), vec.size()) {}

	template <typename T, size_t Count, bool S>
	template <class _Ty, class _Alloc>
	inline Array<T, Count, S>::Array(std::vector<_Ty, _Alloc> &&vec)
	{
		reserve(vec.size());
		this->length = vec.size();

		if (std::is_pod<T>::value &&
			std::is_same<typename std::remove_cv<T>::type, typename std::remove_cv<_Ty>::type>::value)
		{
			memcpy((void*)this->ptr, vec.data(), sizeof(T)*this->length);
		}
		else
		{
			for (size_t i = 0; i < this->length; ++i)
				new((void*)&this->ptr[i]) T(std::move(vec.data()[i]));
		}
	}
#endif

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>::~Array()
	{
		if (!this->ptr)
			return;
		clear();
		if (is_allocated())
		{
			detail::free_array(this->ptr);
			this->ptr = nullptr;
		}
	}

	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::reserve(size_t count)
	{
		bool hasAlloc = is_allocated();
		// early-out if it's in the fixed allocation
		if (count <= Count)
		{
			if (!hasAlloc)
				this->ptr = local.ptr();
			return;
		}
		// work out how much to allocate
		size_t bytes;
		if (hasAlloc)
		{
			bytes = detail::get_array_header(this->ptr)->bytes;
			count *= sizeof(T);
			// early out if there's already more than the request
			if (count <= bytes)
				return;
			do bytes *= 2; // is this a reasonable growth heuristic?
			while (count > bytes);
		}
		else
		{
			enum { InitialAlloc = Count > 8 ? Count * 2 : 16 };
			bytes = count <= InitialAlloc ? InitialAlloc * sizeof(T) : count * sizeof(T);
		}
		// TODO: support realloc?
//		if (hasAlloc)
//		{
//			// attempt to realloc
//			T *mem = detail::slice_realloc<T>(this->ptr, bytes);
//			if (mem)
//			{
//				this->ptr = mem;
//				return;
//			}
//		}
		// alloc new memory and copy old contents
		T *mem = detail::alloc_array<T>(bytes, detail::ArrayHeader::None);
		if (std::is_pod<T>::value)
			memcpy((void*)mem, this->ptr, sizeof(T)*this->length);
		else
		{
			for (size_t i = 0; i < this->length; ++i)
			{
				new((void*)&(mem[i])) T(std::move(this->ptr[i]));
				this->ptr[i].~T();
			}
		}
		// free old memory
		if (hasAlloc)
			detail::free_array<T>(this->ptr);
		this->ptr = mem;
	}
	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::alloc(size_t count)
	{
		clear();
		reserve(count);
		for (size_t i = 0; i < count; ++i)
			new(&this->ptr[i]) T();
		this->length = count;
	}
	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::resize(size_t count)
	{
		if (count < this->length)
		{
			for (size_t i = count; i < this->length; ++i)
				this->ptr[i].~T();
		}
		else if (this->length < count)
		{
			reserve(count);
			for (size_t i = this->length; i < count; ++i)
				new(&this->ptr[i]) T();
		}
		this->length = count;
	}
	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::clear()
	{
		for (size_t i = 0; i < this->length; ++i)
			this->ptr[i].~T();
		this->length = 0;
	}

	template <typename T, size_t Count, bool S>
	inline Array<T, Count, S>& Array<T, Count, S>::operator=(const Array<T, Count, S> &arr)
	{
		if (this != &arr)
		{
			this->~Array();
			new(this) Array<T, Count, S>(arr);
		}
		return *this;
	}
	template <typename T, size_t Count, bool IsString>
	template <typename U, size_t N, bool S>
	inline Array<T, Count, IsString>& Array<T, Count, IsString>::operator=(Array<U, N, S> &&rval)
	{
		if (this != &rval)
		{
			this->~Array();
			new(this) Array<T, Count, IsString>(std::move(rval));
		}
		return *this;
	}

	template <typename T, size_t Count, bool IsString>
	template <typename U, bool S>
	inline Array<T, Count, IsString>& Array<T, Count, IsString>::operator=(Slice<U, S> arr)
	{
		this->~Array();
		new(this) Array<T, Count, IsString>(arr.ptr, arr.length);
		return *this;
	}

	template<typename T, size_t Count, bool S>
	template<typename... Items>
	inline Array<T, Count, S>& Array<T, Count, S>::append(Items&&... items)
	{
		size_t len = this->length + detail::count<value_type>(std::forward<Items>(items)...);
		reserve(len);
		detail::append<value_type>((value_type*)this->ptr + this->length, std::forward<Items>(items)...);
		this->length = len;
		return *this;
	}

	template <typename T, size_t Count, bool S>
	template <typename U>
	inline typename Array<T, Count, S>::reference Array<T, Count, S>::push_back(U &&item)
	{
		reserve(this->length + 1);
		new((void*)(this->ptr + this->length)) T(std::forward<U>(item));
		return this->ptr[this->length++];
	}

	template <typename T, size_t Count, bool S>
	template <typename... Args>
	inline typename Array<T, Count, S>::reference Array<T, Count, S>::emplace_back(Args&&... args)
	{
		reserve(this->length + 1);
		new((void*)(this->ptr + this->length)) T(std::forward<Args>(args)...);
		return this->ptr[this->length++];
	}

	template <typename T, size_t Count, bool S>
	inline typename Array<T, Count, S>::value_type Array<T, Count, S>::pop_back()
	{
		SLICE_ASSERT(this->length > 0);
		--this->length;
		T copy(std::move(this->ptr[this->length]));
		this->ptr[this->length].~T();
		return copy;
	}

	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::pop_back(size_t n)
	{
		SLICE_ASSERT(n >= this->length);
		for (size_t i = this->length - n; i < this->length; ++i)
			this->ptr[i].~T();
		this->length -= n;
	}

	template <typename T, size_t Count, bool S>
	inline typename Array<T, Count, S>::value_type Array<T, Count, S>::remove(size_t i)
	{
		--this->length;
		T copy(std::move(this->ptr[i]));
		for (; i < this->length; ++i)
			this->ptr[i] = std::move(this->ptr[i + 1]);
		this->ptr[i].~T();
		return copy;
	}
	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::remove(const value_type *item)
	{
		remove(this->index_of_element(item));
	}
	template <typename T, size_t Count, bool S>
	inline typename Array<T, Count, S>::value_type Array<T, Count, S>::remove_swap_last(size_t i)
	{
		T copy(std::move(this->ptr[i]));
		if (i < this->length - 1)
			this->ptr[i] = std::move(this->ptr[this->length - 1]);
		this->ptr[--this->length].~T();
		return copy;
	}
	template <typename T, size_t Count, bool S>
	inline void Array<T, Count, S>::remove_swap_last(const value_type *item)
	{
		remove_swap_last(this->index_of_element(item));
	}

	template <typename T, size_t Count, bool S>
	inline Slice<T> Array<T, Count, S>::get_buffer() const noexcept
	{
		if (is_allocated())
			return{ this->ptr, detail::get_array_header(this->ptr)->bytes / sizeof(T) };
		return{ local.ptr(), Count };
	}


	// string methods...

	namespace detail
	{
		template <typename T, typename C>
		inline size_t num_code_units(const C *str, size_t len) noexcept
		{
			if (sizeof(C) == sizeof(T))
				return len;
			else
			{
				const C *end = str + len;
				size_t count = 0;
				while (str < end)
				{
					char32_t c;
					str += detail::utf_decode(str, &c);
					count += detail::utf_seq_length<T>(c);
				}
				return count;
			}
		}
		template <typename T, typename C>
		inline size_t num_code_units(const C *c_str) noexcept
		{
			if (sizeof(C) == sizeof(T))
				return strlen(c_str);
			else
			{
				size_t count = 0;
				while (*c_str)
				{
					char32_t c;
					c_str += detail::utf_decode(c_str, &c);
					count += detail::utf_seq_length<T>(c);
				}
				return count;
			}
		}

		template <typename T, typename C>
		inline size_t transcode_string(T *buffer, const C *str, size_t len) noexcept
		{
			if (sizeof(C) == sizeof(T))
			{
				memcpy(buffer, str, len * sizeof(T));
				return len;
			}
			else
			{
				const C *end = str + len;
				T *buf = buffer;
				while (str < end)
				{
					char32_t c;
					str += detail::utf_decode(str, &c);
					buf += detail::utf_encode(c, buf);
				}
				return buf - buffer;
			}
		}

		template <typename T, typename C>
		inline size_t transcode_string(T *buffer, const C *str) noexcept
		{
			if (sizeof(C) == sizeof(T))
			{
				strcpy(buffer, str);
				return detail::strlen(str); // TODO: we really need to avoid 2 passes over the string!
			}
			else
			{
				T *buf = buffer;
				while (*str)
				{
					char32_t c;
					str += detail::utf_decode(str, &c);
					buf += detail::utf_encode(c, buf);
				}
				return buf - buffer;
			}
		}

		// set of functions that count code units in strings
		template<typename T>
		constexpr size_t count_chars() noexcept { return 0; }
		template<typename T, typename U, typename... Args>
		inline auto count_chars(const U *str, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), size_t());
		template<typename T, typename U, bool S, typename... Args>
		inline auto count_chars(Slice<U, S> slice, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), size_t());
		template<typename T, typename U, typename... Args>
		inline auto count_chars(U item, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), size_t())
		{
			return count_chars<T>(args...) + utf_seq_length<T>(item);
		}
		template<typename T, typename U, bool S, typename... Args>
		inline auto count_chars(Slice<U, S> slice, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), size_t())
		{
			return count_chars<T>(args...) + num_code_units<T>(slice.ptr, slice.length);
		}
		template<typename T, typename U, typename... Args>
		inline auto count_chars(const U *str, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), size_t())
		{
			return count_chars<T>(args...) + num_code_units<T>(str);
		}

		// set of functions that append strings
		template<typename T>
		inline T* append_string(T *buffer) noexcept { return buffer; }
		template<typename T, typename U, typename... Args>
		inline auto append_string(T *buffer, const U *s, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), return_t_star<T>());
		template<typename T, typename U, bool S, typename... Args>
		inline auto append_string(T *buffer, Slice<U, S> s, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), return_t_star<T>());
		template<typename T, typename U, typename... Args>
		inline auto append_string(T *buffer, U c, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), return_t_star<T>())
		{
			size_t len = utf_encode(c, buffer);
			append_string(buffer + len, args...);
			return buffer;
		}
		template<typename T, typename U, bool S, typename... Args>
		inline auto append_string(T *buffer, Slice<U, S> s, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), return_t_star<T>())
		{
			size_t len = transcode_string(buffer, s.ptr, s.length);
			append_string(buffer + len, args...);
			return buffer;
		}
		template<typename T, typename U, typename... Args>
		inline auto append_string(T *buffer, const U *s, const Args&... args) noexcept -> decltype(typename IsSomeChar<U>::type(), return_t_star<T>())
		{
			T *buf = buffer;
			if (sizeof(U) == sizeof(T))
			{
				while (*s)
					*buf++ = *s++;
			}
			else
			{
				while (*s)
				{
					char32_t c;
					s += detail::utf_decode(s, &c);
					buf += detail::utf_encode(c, buf);
				}
			}

			append_string(buf, args...);
			return buffer;
		}

		inline int vscprintf(const char *format, va_list args) noexcept
		{
#if defined(_MSC_VER)
			return _vscprintf(format, args);
#else
			return vsnprintf(nullptr, 0, format, args);
#endif
		}
		inline int vscprintf(const wchar_t *format, va_list args) noexcept
		{
#if defined(_MSC_VER)
			return _vscwprintf(format, args);
#else
			return vswprintf(nullptr, 0, format, args);
#endif
		}
		inline int vsnprintf(char *s, size_t count, const char *format, va_list args) noexcept
		{
#if defined(_MSC_VER)
			return vsnprintf_s(s, count, count, format, args);
#else
			return vsnprintf(s, count, format, args);
#endif
		}
		inline int vsnprintf(wchar_t *s, size_t count, const wchar_t *format, va_list args) noexcept
		{
#if defined(_MSC_VER)
			return _vsnwprintf_s(s, count, count, format, args);
#else
			return vswprintf(s, count, format, args);
#endif
		}
	}

	template <typename C, size_t Count>
	inline Array<C, Count, true>::Array() noexcept
		: Array<C, Count, false>() {}

	template <typename C, size_t Count>
	inline Array<C, Count, true>::Array(const Array<C, Count, true> &val) noexcept
		: Array<C, Count, true>(val.ptr, val.length) {}

	template <typename C, size_t Count>
	template <typename U, size_t N, bool S>
	inline Array<C, Count, true>::Array(Array<U, N, S> &&rval) noexcept
	{
		static_assert(detail::IsSomeChar<U>::value, "Argument is not a string type!");

		if (sizeof(C) == sizeof(U) && rval.is_allocated())
		{
			// claim allocation
			this->ptr = (C*)rval.ptr;
			this->length = rval.length;
			rval.ptr = nullptr;
			rval.length = 0;
		}
		else
			new(this) Array<C, Count, true>(rval.ptr, rval.length);
	}

	template <typename C, size_t Count>
	template <typename U>
	inline Array<C, Count, true>::Array(U *ptr, size_t length) noexcept
	{
		static_assert(detail::IsSomeChar<U>::value, "Argument is not a string type!");

		if (!length)
			return;

		size_t numCodepoints = detail::num_code_units<C>(ptr, length);

		this->reserve(numCodepoints + 1);
		this->length = numCodepoints;

		size_t len = detail::transcode_string(this->ptr, ptr, length);
		assert(len == numCodepoints);
		this->ptr[len] = 0;
	}

	template <typename C, size_t Count>
	template <typename U, bool S>
	inline Array<C, Count, true>::Array(Slice<U, S> slice) noexcept
		: Array<C, Count, true>(slice.ptr, slice.length) {}

	template <typename C, size_t Count>
	template <typename U>
	inline Array<C, Count, true>::Array(const U *c_str) noexcept
	{
		static_assert(detail::IsSomeChar<U>::value, "Argument is not a string type!");

		if (!c_str || *c_str == 0)
			return;
		if (sizeof(C) == sizeof(U))
		{
			size_t len = detail::strlen(c_str);
			this->reserve(len + 1);
			memcpy(this->ptr, c_str, sizeof(C) * len);
			this->ptr[len] = 0;
			this->length = len;
		}
		else
		{
			size_t numCodepoints = 0;
			const U *s = c_str;
			while (*s)
			{
				char32_t c;
				s += detail::utf_decode(s, &c);
				numCodepoints += detail::utf_seq_length<C>(c);
			}

			this->reserve(numCodepoints + 1);
			this->length = numCodepoints;

			C *ptr = this->ptr;
			while (*c_str)
			{
				char32_t c;
				c_str += detail::utf_decode(c_str, &c);
				ptr += detail::utf_encode(c, ptr);
			}
			assert(ptr == this->ptr + this->length);
			*ptr = 0;
		}
	}

#if !defined(NO_STL)
	template <typename C, size_t Count>
	template <class _Ty, class _Alloc>
	inline Array<C, Count, true>::Array(const std::vector<_Ty, _Alloc> &vec) noexcept
		: Array<C, Count, true>(vec.data(), vec.size()) {}

	template <typename C, size_t Count>
	template <class _Elem, class _Traits, class _Alloc>
	inline Array<C, Count, true>::Array(const std::basic_string<_Elem, _Traits, _Alloc> &str) noexcept
		: Array<C, Count, true>(str.data(), str.length()) {}
#endif

	template <typename C, size_t Count>
	inline Array<C, Count, true>::Array(Alloc_T, size_t count) noexcept
		: Array<C, Count, false>(Alloc, count) {}

	template <typename C, size_t Count>
	inline Array<C, Count, true>::Array(Reserve_T, size_t count) noexcept
		: Array<C, Count, false>(Reserve, count) {}

	template <typename C, size_t Count>
	template <typename... Items>
	inline Array<C, Count, true>::Array(Concat_T, const Items&... items) noexcept
	{
		this->length = detail::count_chars<value_type>(items...);
		this->ptr = detail::append_string<value_type>(detail::alloc_array<value_type>((this->length + 1) * sizeof(value_type), detail::ArrayHeader::None), items...);
		this->ptr[this->length] = 0;
	}

	template <typename C, size_t Count>
	template <typename U>
	inline Array<C, Count, true>::Array(Sprintf_T, const U *format, ...) noexcept
	{
		va_list args;
		va_start(args, format);
		do_sprintf(format, args);
		va_end(args);
	}

	template <typename C, size_t Count>
	inline Array<C, Count, true>& Array<C, Count, true>::operator=(const Array<C, Count, true> &str) noexcept
	{
		this->clear();
		new(this) Array<C, Count, true>(str.ptr, str.length);
		return *this;
	}
	template <typename C, size_t Count>
	template <typename U, size_t N, bool S>
	inline Array<C, Count, true>& Array<C, Count, true>::operator=(Array<U, N, S> &&rval) noexcept
	{
		this->clear();
		new(this) Array<C, Count, true>(std::move(rval));
		return *this;
	}
	template <typename C, size_t Count>
	template <typename U, bool S>
	inline Array<C, Count, true>& Array<C, Count, true>::operator=(Slice<U, S> str) noexcept
	{
		this->clear();
		new(this) Array<C, Count, true>(str.ptr, str.length);
		return *this;
	}
	template <typename C, size_t Count>
	template <typename U>
	inline Array<C, Count, true>& Array<C, Count, true>::operator=(const U *c_str) noexcept
	{
		this->clear();
		new(this) Array<C, Count, true>(c_str);
		return *this;
	}

	template<typename C, size_t Count>
	template<typename... Items>
	inline Array<C, Count, true>& Array<C, Count, true>::append(const Items&... items) noexcept
	{
		size_t len = this->length + detail::count_chars<value_type>(items...);
		this->reserve(len + 1);
		detail::append_string<value_type>((value_type*)this->ptr + this->length, items...);
		this->ptr[len] = 0;
		this->length = len;
		return *this;
	}

	template<typename C, size_t Count>
	template <typename U>
	inline Array<C, Count, true>& Array<C, Count, true>::sprintf(const U *format, ...) noexcept
	{
		va_list args;
		va_start(args, format);
		this->clear();
		do_sprintf(format, args);
		va_end(args);
		return *this;
	}

	template<typename C, size_t Count>
	inline Array<C, Count, true>& Array<C, Count, true>::to_upper() noexcept
	{
		for (size_t i = 0; i < this->length; ++i)
			this->ptr[i] = detail::to_upper(this->ptr[i]);
		return *this;
	}

	template<typename C, size_t Count>
	inline Array<C, Count, true>& Array<C, Count, true>::to_lower() noexcept
	{
		for (size_t i = 0; i < this->length; ++i)
			this->ptr[i] = detail::to_lower(this->ptr[i]);
		return *this;
	}

	template<typename C, size_t Count>
	template <typename U, bool S>
	inline Array<C, Count, true>& Array<C, Count, true>::url_encode(Slice<U, S> text)
	{
		this->reserve(text.length*3 + 1);
		this->length = detail::url_encode(Slice<C>(this->ptr, text.length*3), text);
		this->ptr[this->length] = 0;
		return *this;
	}
	template<typename C, size_t Count>
	template <typename U, bool S>
	inline Array<C, Count, true>& Array<C, Count, true>::url_decode(Slice<U, S> url)
	{
		this->reserve(url.length + 1);
		this->length = detail::url_decode(Slice<C>(this->ptr, url.length), url);
		this->ptr[this->length] = 0;
		return *this;
	}

	template<typename C, size_t Count>
	template <typename U>
	inline void Array<C, Count, true>::do_sprintf(const U *format, va_list args) noexcept
	{
		static_assert(detail::IsSomeChar<U>::value, "'format' is not a string type.");
		static_assert(sizeof(U) == sizeof(char) || sizeof(U) == sizeof(wchar_t), "'format' must be char or wchar_t.");

		va_list args2;
		va_copy(args2, args);

		if (sizeof(U) == sizeof(char))
		{
			size_t len = detail::vscprintf(format, args);
			if (sizeof(C) == sizeof(U))
			{
				this->reserve(len + 1);
				this->length = len;
				detail::vsnprintf((char*)this->ptr, len + 1, (const char*)format, args2);
			}
			else
			{
				MutableString<256> temp;
				temp.reserve(len + 1);
				temp.length = len;
				detail::vsnprintf(temp.ptr, len + 1, (const char*)format, args2);
				new(this) Array<C, Count, true>(temp);
			}
		}
		else
		{
			size_t len = detail::vscprintf(format, args);
			if (sizeof(C) == sizeof(U))
			{
				this->reserve(len + 1);
				this->length = len;
				detail::vsnprintf((wchar_t*)this->ptr, len + 1, (const wchar_t*)format, args2);
			}
			else
			{
				Array<wchar_t, 256> temp;
				temp.reserve(len + 1);
				temp.length = len;
				detail::vsnprintf(temp.ptr, len + 1, (const wchar_t*)format, args2);
				new(this) Array<C, Count, true>(temp);
			}
		}

		va_end(args2);
	}
}
