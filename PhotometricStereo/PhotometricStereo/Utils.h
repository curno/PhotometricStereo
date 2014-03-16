#pragma once

#include "stdafx.h"

#define readonly(X) __declspec(property(get=Get##X)) X
#define writeonly(X) __declspec(property(put=Set##X)) X
#define readwrite(X) __declspec(property(get=Get##X,put=Set##X)) X

template<typename T>
void Write(ostream &os, const T t)
{
    os.write(reinterpret_cast<const char*>(&t), sizeof(T));
}

template<typename T>
void Read(istream &is, T &t)
{
    is.read(reinterpret_cast<char*>(&t), sizeof(T));
}

inline void Write(ostream &os, const string &s)
{
    Write(os, (int)(s.length()));
    for (int i = 0; i < s.length(); ++i)
        Write(os, s[i]);
}

inline void Read(istream &is, string &s)
{
    s = "";
    int count;
    Read(is, count);
    for (int i = 0; i < count; ++i)
    {
        char c;
        Read(is, c);
        s += c;
    }
}

inline string FromQStringToStdString(const QString &s) { return s.toLocal8Bit().data(); }
inline QString FromStdStringToQString(const string &s) { return QString::fromLocal8Bit(s.c_str()); }