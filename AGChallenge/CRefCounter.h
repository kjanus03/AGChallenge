#ifndef CREFCOUNTER
#define CREFCOUNTER
#include <vector>

template <typename T>
class CSmartPointer;  // Forward declaration

template <typename T>
class CRefCounter {
public:
    CRefCounter();
    void vAddPointer(CSmartPointer<T>* pSmartPointer);
    void vDeletePointer(CSmartPointer<T>* pSmartPointer);
    int iGetCount();
    std::vector<CSmartPointer<T>*> vpGetSharedPointerList() const;

private:
    int i_count;
    std::vector<CSmartPointer<T>*> pv_pointers;
};

template <typename T>
CRefCounter<T>::CRefCounter() { i_count = 0; }

template <typename T>
void CRefCounter<T>::vAddPointer(CSmartPointer<T>* pSmartPointer) {
    pv_pointers.push_back(pSmartPointer);
    i_count++;
}

template <typename T>
void CRefCounter<T>::vDeletePointer(CSmartPointer<T>* pSmartPointer) {
    auto it = std::remove(pv_pointers.begin(), pv_pointers.end(), pSmartPointer);
    pv_pointers.erase(it, pv_pointers.end());
    i_count = pv_pointers.size();
}

template <typename T>
int CRefCounter<T>::iGetCount() 
{
    return i_count;
}

template <typename T>
std::vector<CSmartPointer<T>*> CRefCounter<T>::vpGetSharedPointerList() const
{
    return pv_pointers;
}

#endif
