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
		T* elements;
		size_t size;
		size_t capacity;
		f32 scalingFactor;
	public:
		void Initialize(mem_tag tag = MEM_TAG_DARRAY, u32 reserveCapacity = 1, f32 _scalingFactor = 1.6f)
		{
			GRASSERT(reserveCapacity != 0);

			size = 0;
			capacity = reserveCapacity;
			scalingFactor = _scalingFactor;
			elements = (T*)GetGlobalAllocator()->Alloc(sizeof(T) * capacity, tag);
		}

		void Deinitialize()
		{
			GetGlobalAllocator()->Free(elements);
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
			elements = (T*)GetGlobalAllocator()->ReAlloc(elements, newCapacity * sizeof(T));
		}

		void Pushback(T&& element)
		{
			if (size >= capacity)
			{
				capacity = (size_t)ceil(capacity * scalingFactor);
				elements = (T*)GetGlobalAllocator()->ReAlloc(elements, capacity * sizeof(T));
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