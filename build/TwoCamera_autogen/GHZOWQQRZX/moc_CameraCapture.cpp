/****************************************************************************
** Meta object code from reading C++ file 'CameraCapture.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../camera/CameraCapture.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraCapture.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CameraCapture_t {
    QByteArrayData data[42];
    char stringdata0[442];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CameraCapture_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CameraCapture_t qt_meta_stringdata_CameraCapture = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CameraCapture"
QT_MOC_LITERAL(1, 14, 12), // "captureImage"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 8), // "cv::Mat*"
QT_MOC_LITERAL(4, 37, 5), // "image"
QT_MOC_LITERAL(5, 43, 19), // "captureCroppedImage"
QT_MOC_LITERAL(6, 63, 12), // "initFinished"
QT_MOC_LITERAL(7, 76, 12), // "startCapture"
QT_MOC_LITERAL(8, 89, 5), // "index"
QT_MOC_LITERAL(9, 95, 11), // "startRecord"
QT_MOC_LITERAL(10, 107, 8), // "savePath"
QT_MOC_LITERAL(11, 116, 10), // "stopRecord"
QT_MOC_LITERAL(12, 127, 10), // "saveRecord"
QT_MOC_LITERAL(13, 138, 14), // "setCameraIndex"
QT_MOC_LITERAL(14, 153, 17), // "setAllowRecording"
QT_MOC_LITERAL(15, 171, 4), // "flag"
QT_MOC_LITERAL(16, 176, 17), // "getAllowRecording"
QT_MOC_LITERAL(17, 194, 13), // "setEnableCrop"
QT_MOC_LITERAL(18, 208, 6), // "enable"
QT_MOC_LITERAL(19, 215, 13), // "setCropParams"
QT_MOC_LITERAL(20, 229, 1), // "x"
QT_MOC_LITERAL(21, 231, 1), // "y"
QT_MOC_LITERAL(22, 233, 5), // "width"
QT_MOC_LITERAL(23, 239, 6), // "height"
QT_MOC_LITERAL(24, 246, 10), // "loadConfig"
QT_MOC_LITERAL(25, 257, 8), // "jsonPath"
QT_MOC_LITERAL(26, 266, 11), // "setSavePath"
QT_MOC_LITERAL(27, 278, 19), // "setAutoWhiteBalance"
QT_MOC_LITERAL(28, 298, 20), // "setWhiteBalanceRatio"
QT_MOC_LITERAL(29, 319, 3), // "red"
QT_MOC_LITERAL(30, 323, 5), // "green"
QT_MOC_LITERAL(31, 329, 4), // "blue"
QT_MOC_LITERAL(32, 334, 13), // "setSaturation"
QT_MOC_LITERAL(33, 348, 10), // "saturation"
QT_MOC_LITERAL(34, 359, 8), // "setGamma"
QT_MOC_LITERAL(35, 368, 5), // "gamma"
QT_MOC_LITERAL(36, 374, 15), // "setExposureTime"
QT_MOC_LITERAL(37, 390, 12), // "exposureTime"
QT_MOC_LITERAL(38, 403, 15), // "setExposureAuto"
QT_MOC_LITERAL(39, 419, 4), // "mode"
QT_MOC_LITERAL(40, 424, 8), // "setFlipX"
QT_MOC_LITERAL(41, 433, 8) // "setFlipY"

    },
    "CameraCapture\0captureImage\0\0cv::Mat*\0"
    "image\0captureCroppedImage\0initFinished\0"
    "startCapture\0index\0startRecord\0savePath\0"
    "stopRecord\0saveRecord\0setCameraIndex\0"
    "setAllowRecording\0flag\0getAllowRecording\0"
    "setEnableCrop\0enable\0setCropParams\0x\0"
    "y\0width\0height\0loadConfig\0jsonPath\0"
    "setSavePath\0setAutoWhiteBalance\0"
    "setWhiteBalanceRatio\0red\0green\0blue\0"
    "setSaturation\0saturation\0setGamma\0"
    "gamma\0setExposureTime\0exposureTime\0"
    "setExposureAuto\0mode\0setFlipX\0setFlipY"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CameraCapture[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  134,    2, 0x06 /* Public */,
       5,    1,  137,    2, 0x06 /* Public */,
       6,    0,  140,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,  141,    2, 0x0a /* Public */,
       7,    0,  144,    2, 0x2a /* Public | MethodCloned */,
       9,    1,  145,    2, 0x0a /* Public */,
      11,    0,  148,    2, 0x0a /* Public */,
      12,    1,  149,    2, 0x0a /* Public */,
      13,    1,  152,    2, 0x0a /* Public */,
      14,    1,  155,    2, 0x0a /* Public */,
      16,    0,  158,    2, 0x0a /* Public */,
      17,    1,  159,    2, 0x0a /* Public */,
      19,    4,  162,    2, 0x0a /* Public */,
      24,    1,  171,    2, 0x0a /* Public */,
      24,    0,  174,    2, 0x2a /* Public | MethodCloned */,
      26,    1,  175,    2, 0x0a /* Public */,
      27,    1,  178,    2, 0x0a /* Public */,
      28,    3,  181,    2, 0x0a /* Public */,
      32,    1,  188,    2, 0x0a /* Public */,
      34,    1,  191,    2, 0x0a /* Public */,
      36,    1,  194,    2, 0x0a /* Public */,
      38,    1,  197,    2, 0x0a /* Public */,
      40,    1,  200,    2, 0x0a /* Public */,
      41,    1,  203,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Bool,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   20,   21,   22,   23,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Float, QMetaType::Float, QMetaType::Float,   29,   30,   31,
    QMetaType::Void, QMetaType::Float,   33,
    QMetaType::Void, QMetaType::Float,   35,
    QMetaType::Void, QMetaType::Float,   37,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Bool,   18,

       0        // eod
};

void CameraCapture::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraCapture *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->captureImage((*reinterpret_cast< cv::Mat*(*)>(_a[1]))); break;
        case 1: _t->captureCroppedImage((*reinterpret_cast< cv::Mat*(*)>(_a[1]))); break;
        case 2: _t->initFinished(); break;
        case 3: _t->startCapture((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->startCapture(); break;
        case 5: _t->startRecord((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->stopRecord(); break;
        case 7: _t->saveRecord((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->setCameraIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->setAllowRecording((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: { bool _r = _t->getAllowRecording();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->setEnableCrop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->setCropParams((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 13: _t->loadConfig((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->loadConfig(); break;
        case 15: _t->setSavePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->setAutoWhiteBalance((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->setWhiteBalanceRatio((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2])),(*reinterpret_cast< float(*)>(_a[3]))); break;
        case 18: _t->setSaturation((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 19: _t->setGamma((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 20: _t->setExposureTime((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 21: _t->setExposureAuto((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->setFlipX((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: _t->setFlipY((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraCapture::*)(cv::Mat * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraCapture::captureImage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CameraCapture::*)(cv::Mat * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraCapture::captureCroppedImage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CameraCapture::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CameraCapture::initFinished)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CameraCapture::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_CameraCapture.data,
    qt_meta_data_CameraCapture,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CameraCapture::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraCapture::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CameraCapture.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CameraCapture::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void CameraCapture::captureImage(cv::Mat * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CameraCapture::captureCroppedImage(cv::Mat * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CameraCapture::initFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
