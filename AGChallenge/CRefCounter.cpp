#include "CRefCounter.h"

CRefCounter::CRefCounter() { i_count; }
int CRefCounter::iAdd() { return ++i_count; }
int CRefCounter::iDec() { return --i_count; }
int CRefCounter::iGet() { return i_count; }

