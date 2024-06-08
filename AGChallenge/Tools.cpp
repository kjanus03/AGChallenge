//#include  "stdafx.h"
#include  "tools.h"
using namespace Tools;

void Tools::show(CString text)
{
	::MessageBox(nullptr, text, text, MB_OK);
}

void Tools::show(int val)
{
	CString textBuf;
	textBuf.Format("%d", val);
	show(textBuf);
}

void Tools::show(double val)
{
	CString textBuf;
	textBuf.Format("%.16lf", val);
	show(textBuf);
}
