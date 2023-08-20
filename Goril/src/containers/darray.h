#pragma once
#include "defines.h"
#include "core/gr_memory.h"
#include "core/asserts.h"

// This makes sure arrays are cache line aligned on most consumer hardware
#define DARRAY_MIN_ALIGNMENT 64

namespace GR
{

	template<typename T>
	class Darray
	{
	private:
		T* elements = nullptr;
		size_t size = 0;
		size_t capacity = 0;
		f32 scalingFactor = 0;
	public:
		void Initialize(mem_tag tag = MEM_TAG_DARRAY, u32 reserveCapacity = 1, f32 _scalingFactor = 1.6f)
		{
			GRASSERT(reserveCapacity != 0);

			size = 0;
			capacity = reserveCapacity;
			scalingFactor = _scalingFactor;
			elements = (T*)GRAlignedAlloc(sizeof(T) * capacity, tag, DARRAY_MIN_ALIGNMENT);
			Zero(elements, sizeof(T) * capacity);
		}

		void Deinitialize()
		{
			GRFree(elements);
		}

		T* GetRawElements()
		{
			return elements;
		}

		const T* GetRawElements() const
		{
			return elements;
		}

		size_t& Size()
		{
			return size;
		}

		const size_t& Size() const
		{
			return size;
		}

		T const& operator[](int index) const
		{
			GRASSERT_DEBUG(index < size);
			return elements[index];
		}

		T& operator[](int index)
		{
			GRASSERT_DEBUG(index < size);
			return elements[index];
		}

		void SetSize(u32 newSize)
		{
			GRASSERT_DEBUG(newSize > size);
			if (capacity < newSize)
			{
				elements = (T*)GReAlloc(elements, newSize * sizeof(T));
				capacity = newSize;
			}

			Zero(elements + size, sizeof(T) * (newSize - size));
			size = newSize;
		}

		void SetCapacity(u32 newCapacity)
		{
			if (size > newCapacity)
				size = newCapacity;
			elements = (T*)GReAlloc(elements, newCapacity * sizeof(T));
		}

		void Pushback(T&& element)
		{
			if (size >= capacity)
			{
				capacity = (size_t)ceil(capacity * scalingFactor);
				elements = (T*)GReAlloc(elements, capacity * sizeof(T));
			}
			elements[size] = element;
			size++;
		}

		void Pushback(T& element)
		{
			if (size >= capacity)
			{
				capacity = (size_t)ceil(capacity * scalingFactor);
				elements = (T*)GReAlloc(elements, capacity * sizeof(T));
			}
			elements[size] = element;
			size++;
		}

		b8 Contains(const T& element)
		{
			for (u32 i = 0; i < size; ++i)
			{
				if (0 == memcmp(&element, &elements[i], sizeof(T)))
				{
					return true;
				}
			}
			return false;
		}

		T PopAt(u32 index)
		{
			T popped = elements[index];
			// Copying the buffer inward to fill in the open gap
			size--;
			MemCopy(elements + index, elements + index + 1, sizeof(T) * (size - index));
			return popped;
		}

		T Pop()
		{
			size--;
			return elements[size];
		}
	};

	template<typename T>
	Darray<T> CreateDarrayWithSize(mem_tag tag = MEM_TAG_DARRAY, u32 size = 0, f32 _scalingFactor = 1.6f)
	{
		Darray<T> darray = Darray<T>();

		if (size)
		{
			darray.Initialize(tag, size, _scalingFactor);
			darray.SetSize(size);
		}
		else
		{
			darray.Initialize(tag, 1, _scalingFactor);
		}

		return darray;
	}

	template<typename T>
	Darray<T> CreateDarrayWithCapacity(mem_tag tag = MEM_TAG_DARRAY, u32 reserveCapacity = 1, f32 _scalingFactor = 1.6f)
	{
		Darray<T> darray = Darray<T>();

		darray.Initialize(tag, reserveCapacity, _scalingFactor);

		return darray;
	}
}