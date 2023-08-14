#pragma once
#include "defines.h"
#include "core/gr_memory.h"
#include "core/asserts.h"

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
			elements = (T*)GAlloc(sizeof(T) * capacity, tag);
		}

		void Deinitialize()
		{
			GFree(elements);
		}

		T* GetRawElements()
		{
			return elements;
		}

		size_t Size()
		{
			return size;
		}

		T const& operator[](int index) const
		{
			GRASSERT(index < size)
			return elements[index];
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
}