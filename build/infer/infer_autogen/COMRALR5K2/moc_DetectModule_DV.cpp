/****************************************************************************
** Meta object code from reading C++ file 'DetectModule_DV.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../infer/detect/DetectModule_DV.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DetectModule_DV.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DetectModule_DV_t {
    QByteArrayData data[7];
    char stringdata0[78];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DetectModule_DV_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DetectModule_DV_t qt_meta_stringdata_DetectModule_DV = {
    {
QT_MOC_LITERAL(0, 0, 15), // "DetectModule_DV"
QT_MOC_LITERAL(1, 16, 18), // "newDetectResultImg"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 9), // "newResult"
QT_MOC_LITERAL(4, 46, 15), // "modelLoadResult"
QT_MOC_LITERAL(5, 62, 7), // "success"
QT_MOC_LITERAL(6, 70, 7) // "message"

    },
    "DetectModule_DV\0newDetectResultImg\0\0"
    "newResult\0modelLoadResult\0success\0"
    "message"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DetectModule_DV[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       3,    1,   32,    2, 0x06 /* Public */,
       4,    2,   35,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    5,    6,

       0        // eod
};

void DetectModule_DV::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DetectModule_DV *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newDetectResultImg((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->newResult((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->modelLoadResult((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DetectModule_DV::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DetectModule_DV::newDetectResultImg)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DetectModule_DV::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DetectModule_DV::newResult)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DetectModule_DV::*)(bool , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DetectModule_DV::modelLoadResult)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DetectModule_DV::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_DetectModule_DV.data,
    qt_meta_data_DetectModule_DV,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DetectModule_DV::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DetectModule_DV::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DetectModule_DV.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DetectModule_DV::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void DetectModule_DV::newDetectResultImg(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DetectModule_DV::newResult(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DetectModule_DV::modelLoadResult(bool _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
