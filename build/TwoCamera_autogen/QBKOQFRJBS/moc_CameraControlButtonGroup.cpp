/****************************************************************************
** Meta object code from reading C++ file 'CameraControlButtonGroup.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../widgets/cameracontrols/CameraControlButtonGroup.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraControlButtonGroup.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraControlButtonGroup_t {
    QByteArrayData data[7];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraControlButtonGroup_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraControlButtonGroup_t qt_meta_stringdata_CameraControlButtonGroup = {
    {
QT_MOC_LITERAL(0, 0, 24), // "CameraControlButtonGroup"
QT_MOC_LITERAL(1, 25, 9), // "dvStarted"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 10), // "dvsStarted"
QT_MOC_LITERAL(4, 47, 15), // "playbackStarted"
QT_MOC_LITERAL(5, 63, 14), // "camerasStarted"
QT_MOC_LITERAL(6, 78, 11) // "startCamera"

    },
    "CameraControlButtonGroup\0dvStarted\0\0"
    "dvsStarted\0playbackStarted\0camerasStarted\0"
    "startCamera"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraControlButtonGroup[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    0,   40,    2, 0x06 /* Public */,
       4,    0,   41,    2, 0x06 /* Public */,
       5,    0,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   43,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CameraControlButtonGroup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraControlButtonGroup *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->dvStarted(); break;
        case 1: _t->dvsStarted(); break;
        case 2: _t->playbackStarted(); break;
        case 3: _t->camerasStarted(); break;
        case 4: _t->startCamera(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraControlButtonGroup::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraControlButtonGroup::dvStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CameraControlButtonGroup::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraControlButtonGroup::dvsStarted)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CameraControlButtonGroup::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraControlButtonGroup::playbackStarted)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CameraControlButtonGroup::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraControlButtonGroup::camerasStarted)) {
                *result = 3;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject CameraControlButtonGroup::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_CameraControlButtonGroup.data,
    qt_meta_data_CameraControlButtonGroup,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraControlButtonGroup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraControlButtonGroup::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraControlButtonGroup.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CameraControlButtonGroup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CameraControlButtonGroup::dvStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CameraControlButtonGroup::dvsStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CameraControlButtonGroup::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CameraControlButtonGroup::camerasStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
