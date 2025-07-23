/****************************************************************************
** Meta object code from reading C++ file 'DVSDataSource.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../dvs/DVSDataSource.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DVSDataSource.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DVSDataSource_t {
    QByteArrayData data[35];
    char stringdata0[409];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DVSDataSource_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DVSDataSource_t qt_meta_stringdata_DVSDataSource = {
    {
QT_MOC_LITERAL(0, 0, 13), // "DVSDataSource"
QT_MOC_LITERAL(1, 14, 8), // "newImage"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 14), // "newBinaryImage"
QT_MOC_LITERAL(4, 39, 12), // "newGrayImage"
QT_MOC_LITERAL(5, 52, 15), // "newAccGrayImage"
QT_MOC_LITERAL(6, 68, 21), // "newConstructGrayImage"
QT_MOC_LITERAL(7, 90, 13), // "cameraStarted"
QT_MOC_LITERAL(8, 104, 13), // "cameraStopped"
QT_MOC_LITERAL(9, 118, 11), // "startCamera"
QT_MOC_LITERAL(10, 130, 10), // "stopCamera"
QT_MOC_LITERAL(11, 141, 10), // "setDenoise"
QT_MOC_LITERAL(12, 152, 6), // "enable"
QT_MOC_LITERAL(13, 159, 8), // "setSpeed"
QT_MOC_LITERAL(14, 168, 1), // "b"
QT_MOC_LITERAL(15, 170, 15), // "setOverlapCount"
QT_MOC_LITERAL(16, 186, 1), // "o"
QT_MOC_LITERAL(17, 188, 9), // "syncImage"
QT_MOC_LITERAL(18, 198, 11), // "startRecord"
QT_MOC_LITERAL(19, 210, 4), // "path"
QT_MOC_LITERAL(20, 215, 10), // "stopRecord"
QT_MOC_LITERAL(21, 226, 16), // "setEnableSaveRaw"
QT_MOC_LITERAL(22, 243, 1), // "f"
QT_MOC_LITERAL(23, 245, 16), // "setEnableSaveImg"
QT_MOC_LITERAL(24, 262, 16), // "getEnableSaveImg"
QT_MOC_LITERAL(25, 279, 16), // "getEnableSaveRaw"
QT_MOC_LITERAL(26, 296, 17), // "setSaveFolderPath"
QT_MOC_LITERAL(27, 314, 17), // "getSaveFolderPath"
QT_MOC_LITERAL(28, 332, 6), // "setFPS"
QT_MOC_LITERAL(29, 339, 3), // "fps"
QT_MOC_LITERAL(30, 343, 6), // "getFPS"
QT_MOC_LITERAL(31, 350, 17), // "applyBiasSettings"
QT_MOC_LITERAL(32, 368, 16), // "loadBiasSettings"
QT_MOC_LITERAL(33, 385, 14), // "saveBiasToFile"
QT_MOC_LITERAL(34, 400, 8) // "filePath"

    },
    "DVSDataSource\0newImage\0\0newBinaryImage\0"
    "newGrayImage\0newAccGrayImage\0"
    "newConstructGrayImage\0cameraStarted\0"
    "cameraStopped\0startCamera\0stopCamera\0"
    "setDenoise\0enable\0setSpeed\0b\0"
    "setOverlapCount\0o\0syncImage\0startRecord\0"
    "path\0stopRecord\0setEnableSaveRaw\0f\0"
    "setEnableSaveImg\0getEnableSaveImg\0"
    "getEnableSaveRaw\0setSaveFolderPath\0"
    "getSaveFolderPath\0setFPS\0fps\0getFPS\0"
    "applyBiasSettings\0loadBiasSettings\0"
    "saveBiasToFile\0filePath"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DVSDataSource[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  144,    2, 0x06 /* Public */,
       3,    1,  147,    2, 0x06 /* Public */,
       4,    1,  150,    2, 0x06 /* Public */,
       5,    1,  153,    2, 0x06 /* Public */,
       6,    1,  156,    2, 0x06 /* Public */,
       7,    0,  159,    2, 0x06 /* Public */,
       8,    0,  160,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,  161,    2, 0x0a /* Public */,
      10,    0,  162,    2, 0x0a /* Public */,
      11,    1,  163,    2, 0x0a /* Public */,
      13,    1,  166,    2, 0x0a /* Public */,
      15,    1,  169,    2, 0x0a /* Public */,
      17,    0,  172,    2, 0x0a /* Public */,
      18,    1,  173,    2, 0x0a /* Public */,
      20,    0,  176,    2, 0x0a /* Public */,
      21,    1,  177,    2, 0x0a /* Public */,
      23,    1,  180,    2, 0x0a /* Public */,
      24,    0,  183,    2, 0x0a /* Public */,
      25,    0,  184,    2, 0x0a /* Public */,
      26,    1,  185,    2, 0x0a /* Public */,
      27,    0,  188,    2, 0x0a /* Public */,
      28,    1,  189,    2, 0x0a /* Public */,
      30,    0,  192,    2, 0x0a /* Public */,
      31,    0,  193,    2, 0x0a /* Public */,
      32,    0,  194,    2, 0x0a /* Public */,
      33,    1,  195,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void, QMetaType::QImage,    2,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   22,
    QMetaType::Void, QMetaType::Bool,   22,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::QString,
    QMetaType::Void, QMetaType::Double,   29,
    QMetaType::Double,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString,   34,

       0        // eod
};

void DVSDataSource::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DVSDataSource *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->newBinaryImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 2: _t->newGrayImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 3: _t->newAccGrayImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 4: _t->newConstructGrayImage((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 5: _t->cameraStarted(); break;
        case 6: _t->cameraStopped(); break;
        case 7: _t->startCamera(); break;
        case 8: _t->stopCamera(); break;
        case 9: _t->setDenoise((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->setSpeed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->setOverlapCount((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->syncImage(); break;
        case 13: _t->startRecord((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->stopRecord(); break;
        case 15: _t->setEnableSaveRaw((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->setEnableSaveImg((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: { bool _r = _t->getEnableSaveImg();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { bool _r = _t->getEnableSaveRaw();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 19: _t->setSaveFolderPath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: { QString _r = _t->getSaveFolderPath();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 21: _t->setFPS((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 22: { double _r = _t->getFPS();
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = std::move(_r); }  break;
        case 23: { bool _r = _t->applyBiasSettings();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 24: _t->loadBiasSettings(); break;
        case 25: { bool _r = _t->saveBiasToFile((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DVSDataSource::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::newImage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::newBinaryImage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::newGrayImage)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::newAccGrayImage)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::newConstructGrayImage)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::cameraStarted)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DVSDataSource::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DVSDataSource::cameraStopped)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DVSDataSource::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_DVSDataSource.data,
    qt_meta_data_DVSDataSource,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DVSDataSource::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DVSDataSource::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DVSDataSource.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DVSDataSource::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void DVSDataSource::newImage(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DVSDataSource::newBinaryImage(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DVSDataSource::newGrayImage(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DVSDataSource::newAccGrayImage(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DVSDataSource::newConstructGrayImage(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DVSDataSource::cameraStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void DVSDataSource::cameraStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
