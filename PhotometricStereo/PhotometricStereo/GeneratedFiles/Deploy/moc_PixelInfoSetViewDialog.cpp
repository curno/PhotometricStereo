/****************************************************************************
** Meta object code from reading C++ file 'PixelInfoSetViewDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.1.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../PixelInfoSetViewDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PixelInfoSetViewDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.1.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PixelInfoSetViewDialog_t {
    QByteArrayData data[4];
    char stringdata[40];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_PixelInfoSetViewDialog_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_PixelInfoSetViewDialog_t qt_meta_stringdata_PixelInfoSetViewDialog = {
    {
QT_MOC_LITERAL(0, 0, 22),
QT_MOC_LITERAL(1, 23, 7),
QT_MOC_LITERAL(2, 31, 0),
QT_MOC_LITERAL(3, 32, 6)
    },
    "PixelInfoSetViewDialog\0showing\0\0hiding\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PixelInfoSetViewDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x05,
       3,    0,   25,    2, 0x05,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PixelInfoSetViewDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PixelInfoSetViewDialog *_t = static_cast<PixelInfoSetViewDialog *>(_o);
        switch (_id) {
        case 0: _t->showing(); break;
        case 1: _t->hiding(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (PixelInfoSetViewDialog::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PixelInfoSetViewDialog::showing)) {
                *result = 0;
            }
        }
        {
            typedef void (PixelInfoSetViewDialog::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PixelInfoSetViewDialog::hiding)) {
                *result = 1;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject PixelInfoSetViewDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PixelInfoSetViewDialog.data,
      qt_meta_data_PixelInfoSetViewDialog,  qt_static_metacall, 0, 0}
};


const QMetaObject *PixelInfoSetViewDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PixelInfoSetViewDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PixelInfoSetViewDialog.stringdata))
        return static_cast<void*>(const_cast< PixelInfoSetViewDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PixelInfoSetViewDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void PixelInfoSetViewDialog::showing()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void PixelInfoSetViewDialog::hiding()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
