#pragma once
namespace data {
	template <class T>
	class DynamicBuffer
	{
	public:
		int initSize;
		int heapsize;
		int length;
		DynamicBuffer(int initSize, int expendStepSize);
		bool expendBuffer(int size);
		bool expendBufferTo(int size);
		bool cutToBufferSizeByFunc(int size, int (*checkFunc)(T*, int));
		//cut objects to size based on "int checkFunc(object,position)"
		//which returns priority of the object.
		//do resize heaps. Inore spaces.
		//Note: any negative int means cannot be destroyed.
		//If unable to cut buffer, return false.
		void cutToBufferSizeByTop(int size);
		//cut out latest added objects.
		//do resize heaps. Inore spaces.
		void cutToBufferSizeByBottom(int size);
		//cut out oldest objects.
		//do resize heaps. Inore spaces.
		void compact();
		//set heapsize to size of data.



	private:
		static unsigned long long int instanceLatest;
		unsigned long long int instanceAccessed;
		int stepSize;
		T** dataArrayP;
		void resizeUp(int size);
	};


	//Implementation
	inline static unsigned long long int instanceLatest = 0;
	template<class T>
	inline DynamicBuffer<T>::DynamicBuffer(int initSize, int expendStepSize)
	{
		length = 0;
		stepSize = expendStepSize;
		resizeUp(initSize);
	}


	template<class T>
	inline void DynamicBuffer<T>::resizeUp(int size)
	{
		T** dataBufferP = new T * [size];
		int j = 0;
		int outLength = length;
		for (int i = 0; i < size; i++)
		{
			if (j < length) {
				T** objPin = dataArrayP + (sizeof(T) * j);
				if (*objPin != nullptr) {
					*(dataBufferP + (sizeof(T) * i)) = *objPin;
				}
				else {
					outLength--;
					i--;
				}
				j++;
			}
			else {
				*(dataBufferP + (sizeof(T) * i)) = nullptr;
			}
		}
		delete[] dataArrayP;
		dataArrayP = dataBufferP;
	}
}