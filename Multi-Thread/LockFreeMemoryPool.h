#pragma once
#include <vector>

template<typename T>
class TLockFreeMemoryPool
{
public:
	TLockFreeMemoryPool(int InitMaxNum,float fWarn = 0.6f, float fCore = 0.9f) 
		:_MaxNums(InitMaxNum)
	{
		_WarnNums = int(_MaxNums * fWarn);
		_CoreNums = int(_MaxNums * fCore);
		_data = new T[_MaxNums];

		_BasePtr = _data;
		_EndPtr = _BasePtr + _MaxNums - 1;
	}

	void BeginFrame() {
		_FreePtrIndex = 0;
	}

	void EndFrame() {
		if (_FreePtrIndex > _WarnNums)
		{
			_MaxNums >>= 1;
			_CoreNums >>= 1;
			_WarnNums >>= 1;

			delete[] _data;

			_data = new T[_MaxNums];
		}
		_FreePtrIndex = 0;
		_BasePtr = _data;
		_EndPtr = _BasePtr + _MaxNums - 1;
	}

	int ElementCount() const {
		return _FreePtrIndex;
	}

	LONG AtomicReadFreePtrIndex() {
		return InterlockedCompareExchange(&_FreePtrIndex, 0, 0);
	}

	T* Alloc(int iNum) {

#if _DEBUG
		LONG _curFreePtrIndex = AtomicReadFreePtrIndex();
		if (_curFreePtrIndex > _WarnNums)
		{
			OutputDebugStringA("TLockFreeMemoryPool 已经超过设置的警戒范围了\r\n");
		}

		assert(_curFreePtrIndex < _CoreNums);
#endif
		LONG newFreePtrIndex = -1;
		do 
		{
			//newFreePtrIndex = AtomicReadFreePtrIndex();
			newFreePtrIndex = InterlockedCompareExchange(&_FreePtrIndex, 0, 0);

		} while (InterlockedCompareExchange((LONG*)&_FreePtrIndex, newFreePtrIndex + iNum, newFreePtrIndex) != newFreePtrIndex);
		//} while (InterlockedIncrement((LONG*)&_FreePtrIndex) == newFreePtrIndex + 1);

		return _BasePtr + newFreePtrIndex + iNum;
	}



	T* GetBasePtr() const {
		return _BasePtr;
	}

private:
	T* _data;
	T* _BasePtr = nullptr;
	T* _EndPtr = nullptr;
	LONG _FreePtrIndex = 0;


	int _WarnNums;
	int _CoreNums;
	int _MaxNums;

};
