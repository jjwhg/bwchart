//------------------------------------------------------------------------------------

#ifndef __HEAPPERCLASS_H
#define __HEAPPERCLASS_H

// Derive from this to create a heap PER INSTANCE and do all allocations
// in that instance using Alloc, Realloc and Free, instead of new/delete
//
class CHeapPerInstance
{
private:
	HANDLE m_heap;
	bool m_multithread;
protected:
	CHeapPerInstance(bool mt=true) : m_heap(0), m_multithread(mt) {}
	~CHeapPerInstance() {if(m_heap!=0) HeapDestroy(m_heap);}

	void *Alloc(size_t size)
	{
		if(m_heap==0) m_heap = HeapCreate(0,8192,0);
		return m_heap==0 ? 0 : HeapAlloc(m_heap,m_multithread?0:HEAP_NO_SERIALIZE,size);
	}
	void *Realloc(void *ptr, size_t size)
	{
		if(m_heap==0) return Alloc(size);
		return HeapReAlloc(m_heap,m_multithread?0:HEAP_NO_SERIALIZE,ptr,size);
	}
	void Free(void *ptr)
	{
		if(m_heap!=0) HeapFree(m_heap,m_multithread?0:HEAP_NO_SERIALIZE,ptr);
	}
};

// To create a heap for a whole CLASS, do this:

// 1 - add this in class definition (.h file)
#define DECLARE_LOCAL_HEAP(initsizeBytes) \
private:\
	static HANDLE m_LHheap;\
	static void *LHAlloc(size_t size)\
	{\
		if(m_LHheap==0) m_LHheap = HeapCreate(0,initsizeBytes,0);\
		return m_LHheap==0 ? 0 : HeapAlloc(m_LHheap,0,size);\
	}\
	static void LHFree(void *ptr)\
	{\
		if(m_LHheap!=0) HeapFree(m_LHheap,0,ptr);\
	}\
public:\
	static void _ExitInstance_() {if(m_LHheap!=0) HeapDestroy(m_LHheap); m_LHheap=0;}\
	void *operator new(size_t size) {return LHAlloc(size);}\
	void operator delete(void *p) {LHFree(p);}


// 2 - add this in class implemention (.cpp file)
#define IMPLEMENT_LOCAL_HEAP(classname)\
HANDLE classname##::m_LHheap = 0;

// 3 - call this when exiting program
#define DESTROY_LOCAL_HEAP(classname)\
classname##::_ExitInstance_();

#endif
