/****************************************************************************
** Meta object code from reading C++ file 'PlaybackReader.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../PlaybackReader.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlaybackReader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlaybackReader_t {
    QByteArrayData data[29];
    char stringdata0[357];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlaybackReader_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlaybackReader_t qt_meta_stringdata_PlaybackReader = {
    {
QT_MOC_LITERAL(0, 0, 14), // "PlaybackReader"
QT_MOC_LITERAL(1, 15, 8), // "newFrame"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 5), // "frame"
QT_MOC_LITERAL(4, 31, 16), // "playbackProgress"
QT_MOC_LITERAL(5, 48, 8), // "progress"
QT_MOC_LITERAL(6, 57, 16), // "playbackFinished"
QT_MOC_LITERAL(7, 74, 20), // "batchPlaybackStarted"
QT_MOC_LITERAL(8, 95, 13), // "totalSessions"
QT_MOC_LITERAL(9, 109, 21), // "batchPlaybackProgress"
QT_MOC_LITERAL(10, 131, 14), // "currentSession"
QT_MOC_LITERAL(11, 146, 18), // "currentSessionName"
QT_MOC_LITERAL(12, 165, 21), // "batchPlaybackFinished"
QT_MOC_LITERAL(13, 187, 13), // "nextImagePair"
QT_MOC_LITERAL(14, 201, 2), // "dv"
QT_MOC_LITERAL(15, 204, 3), // "dvs"
QT_MOC_LITERAL(16, 208, 8), // "complete"
QT_MOC_LITERAL(17, 217, 21), // "visualizationProgress"
QT_MOC_LITERAL(18, 239, 7), // "current"
QT_MOC_LITERAL(19, 247, 5), // "total"
QT_MOC_LITERAL(20, 253, 21), // "visualizationComplete"
QT_MOC_LITERAL(21, 275, 9), // "outputDir"
QT_MOC_LITERAL(22, 285, 13), // "startPlayback"
QT_MOC_LITERAL(23, 299, 4), // "path"
QT_MOC_LITERAL(24, 304, 14), // "PlaybackParams"
QT_MOC_LITERAL(25, 319, 6), // "params"
QT_MOC_LITERAL(26, 326, 12), // "stopPlayback"
QT_MOC_LITERAL(27, 339, 12), // "saveYUVFrame"
QT_MOC_LITERAL(28, 352, 4) // "stop"

    },
    "PlaybackReader\0newFrame\0\0frame\0"
    "playbackProgress\0progress\0playbackFinished\0"
    "batchPlaybackStarted\0totalSessions\0"
    "batchPlaybackProgress\0currentSession\0"
    "currentSessionName\0batchPlaybackFinished\0"
    "nextImagePair\0dv\0dvs\0complete\0"
    "visualizationProgress\0current\0total\0"
    "visualizationComplete\0outputDir\0"
    "startPlayback\0path\0PlaybackParams\0"
    "params\0stopPlayback\0saveYUVFrame\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlaybackReader[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x06 /* Public */,
       4,    1,   87,    2, 0x06 /* Public */,
       6,    0,   90,    2, 0x06 /* Public */,
       7,    1,   91,    2, 0x06 /* Public */,
       9,    3,   94,    2, 0x06 /* Public */,
      12,    0,  101,    2, 0x06 /* Public */,
      13,    2,  102,    2, 0x06 /* Public */,
      16,    0,  107,    2, 0x06 /* Public */,
      17,    2,  108,    2, 0x06 /* Public */,
      20,    1,  113,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      22,    1,  116,    2, 0x0a /* Public */,
      22,    1,  119,    2, 0x0a /* Public */,
      26,    1,  122,    2, 0x0a /* Public */,
      28,    0,  125,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void, QMetaType::Double,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::QString,   10,    8,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage, QMetaType::QImage,   14,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   18,   19,
    QMetaType::Void, QMetaType::QString,   21,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   23,
    QMetaType::Void, 0x80000000 | 24,   25,
    QMetaType::Void, QMetaType::Bool,   27,
    QMetaType::Void,

       0        // eod
};

void PlaybackReader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlaybackReader *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->newFrame((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 1: _t->playbackProgress((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->playbackFinished(); break;
        case 3: _t->batchPlaybackStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->batchPlaybackProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 5: _t->batchPlaybackFinished(); break;
        case 6: _t->nextImagePair((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2]))); break;
        case 7: _t->complete(); break;
        case 8: _t->visualizationProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->visualizationComplete((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->startPlayback((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->startPlayback((*reinterpret_cast< const PlaybackParams(*)>(_a[1]))); break;
        case 12: _t->stopPlayback((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlaybackReader::*)(QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::newFrame)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::playbackProgress)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::playbackFinished)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::batchPlaybackStarted)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(int , int , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::batchPlaybackProgress)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::batchPlaybackFinished)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(QImage , QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::nextImagePair)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::complete)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::visualizationProgress)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (PlaybackReader::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackReader::visualizationComplete)) {
                *result = 9;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PlaybackReader::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_PlaybackReader.data,
    qt_meta_data_PlaybackReader,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlaybackReader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlaybackReader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlaybackReader.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlaybackReader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void PlaybackReader::newFrame(QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PlaybackReader::playbackProgress(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PlaybackReader::playbackFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlaybackReader::batchPlaybackStarted(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PlaybackReader::batchPlaybackProgress(int _t1, int _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PlaybackReader::batchPlaybackFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PlaybackReader::nextImagePair(QImage _t1, QImage _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PlaybackReader::complete()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void PlaybackReader::visualizationProgress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void PlaybackReader::visualizationComplete(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
