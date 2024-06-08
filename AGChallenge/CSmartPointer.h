#ifndef CSMARTPOINTER
#define CSMARTPOINTER
#include "CRefCounter.h"

template <typename T> class CSmartPointer
{
public:
	CSmartPointer();
	CSmartPointer(T* pcPointer);
	CSmartPointer(const CSmartPointer& pcOther);
	~CSmartPointer();

	CSmartPointer<T> cDuplicate();
	T& operator*();
	T* operator->();
	CSmartPointer<T>& operator=(const CSmartPointer& pcOther);
	std::vector<CSmartPointer<T>*> vpGetSharedPointerList() const;


private:
	CRefCounter<T>* pc_counter;
	T* pc_pointer;
};

template <typename T>
CSmartPointer<T>::CSmartPointer()
{
	pc_pointer = nullptr;
	pc_counter = new CRefCounter<T>();
	pc_counter->vAddPointer(this);
}


template <typename T>
CSmartPointer<T>::CSmartPointer(T* pcPointer)
{
	pc_pointer = pcPointer;
	pc_counter = new CRefCounter<T>();
	pc_counter->vAddPointer(this);
}

template<typename T>
CSmartPointer<T>::CSmartPointer(const CSmartPointer& pcOther)
{
	pc_pointer = pcOther.pc_pointer;
	pc_counter = pcOther.pc_counter;
	pc_counter->vAddPointer(this);
}

template <typename T>
CSmartPointer<T>::~CSmartPointer() 
{
	pc_counter->vDeletePointer(this);
	if (pc_counter->iGetCount() == 0)
	{
		delete pc_pointer;
		delete pc_counter;
	}
}

template <typename T>
CSmartPointer<T> CSmartPointer<T>::cDuplicate()
{
	pc_counter->vAddPointer(this);
	return CSmartPointer<T>(*this);
}

template <typename T>
T& CSmartPointer<T>::operator*() { return(*pc_pointer); }

template <typename T>
T* CSmartPointer<T>::operator->() { return(pc_pointer); }

template <typename T>
CSmartPointer<T>& CSmartPointer<T>::operator=(const CSmartPointer& pcOther) {
	if (this != &pcOther) {
		pc_counter->vDeletePointer(this);
		if (pc_counter->iGetCount() == 0) {
			delete pc_pointer;
			delete pc_counter;
		}

		pc_pointer = pcOther.pc_pointer;
		pc_counter = pcOther.pc_counter;
		pc_counter->vAddPointer(this);
	}
	return *this;
}

template <typename T>
std::vector<CSmartPointer<T>*> CSmartPointer<T>::vpGetSharedPointerList() const {
	return pc_counter->vpGetSharedPointerList();
}

#endif 