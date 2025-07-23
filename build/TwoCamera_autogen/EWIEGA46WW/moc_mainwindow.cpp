/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[52];
    char stringdata0[776];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 13), // "cameraStarted"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 15), // "playbackStarted"
QT_MOC_LITERAL(4, 42, 11), // "startRecord"
QT_MOC_LITERAL(5, 54, 4), // "path"
QT_MOC_LITERAL(6, 59, 10), // "stopRecord"
QT_MOC_LITERAL(7, 70, 10), // "saveRecord"
QT_MOC_LITERAL(8, 81, 14), // "startRecordMax"
QT_MOC_LITERAL(9, 96, 14), // "startRecordMid"
QT_MOC_LITERAL(10, 111, 14), // "startRecordMin"
QT_MOC_LITERAL(11, 126, 5), // "setTH"
QT_MOC_LITERAL(12, 132, 1), // "v"
QT_MOC_LITERAL(13, 134, 18), // "startButtonClicked"
QT_MOC_LITERAL(14, 153, 13), // "updateDVSView"
QT_MOC_LITERAL(15, 167, 3), // "img"
QT_MOC_LITERAL(16, 171, 21), // "updateDVDetectionView"
QT_MOC_LITERAL(17, 193, 22), // "updateDVSDetectionView"
QT_MOC_LITERAL(18, 216, 19), // "onDVDetectionResult"
QT_MOC_LITERAL(19, 236, 6), // "result"
QT_MOC_LITERAL(20, 243, 20), // "onDVSDetectionResult"
QT_MOC_LITERAL(21, 264, 15), // "onFinalDecision"
QT_MOC_LITERAL(22, 280, 8), // "decision"
QT_MOC_LITERAL(23, 289, 9), // "timestamp"
QT_MOC_LITERAL(24, 299, 18), // "onDetectionToggled"
QT_MOC_LITERAL(25, 318, 7), // "enabled"
QT_MOC_LITERAL(26, 326, 17), // "onModelLoadResult"
QT_MOC_LITERAL(27, 344, 7), // "success"
QT_MOC_LITERAL(28, 352, 7), // "message"
QT_MOC_LITERAL(29, 360, 20), // "handleStartMaxRecord"
QT_MOC_LITERAL(30, 381, 19), // "handleStopMaxRecord"
QT_MOC_LITERAL(31, 401, 20), // "handleStartMidRecord"
QT_MOC_LITERAL(32, 422, 19), // "handleStopMidRecord"
QT_MOC_LITERAL(33, 442, 20), // "handleStartMinRecord"
QT_MOC_LITERAL(34, 463, 19), // "handleStopMinRecord"
QT_MOC_LITERAL(35, 483, 18), // "onPlaybackNewFrame"
QT_MOC_LITERAL(36, 502, 5), // "frame"
QT_MOC_LITERAL(37, 508, 18), // "onPlaybackProgress"
QT_MOC_LITERAL(38, 527, 8), // "progress"
QT_MOC_LITERAL(39, 536, 18), // "onPlaybackFinished"
QT_MOC_LITERAL(40, 555, 38), // "on_selectRootPlaybackDirButto..."
QT_MOC_LITERAL(41, 594, 22), // "onBatchPlaybackStarted"
QT_MOC_LITERAL(42, 617, 13), // "totalSessions"
QT_MOC_LITERAL(43, 631, 23), // "onBatchPlaybackProgress"
QT_MOC_LITERAL(44, 655, 14), // "currentSession"
QT_MOC_LITERAL(45, 670, 18), // "currentSessionName"
QT_MOC_LITERAL(46, 689, 23), // "onBatchPlaybackFinished"
QT_MOC_LITERAL(47, 713, 19), // "onPlaybackImagePair"
QT_MOC_LITERAL(48, 733, 2), // "dv"
QT_MOC_LITERAL(49, 736, 3), // "dvs"
QT_MOC_LITERAL(50, 740, 24), // "saveDetectionResultImage"
QT_MOC_LITERAL(51, 765, 10) // "cameraType"

    },
    "MainWindow\0cameraStarted\0\0playbackStarted\0"
    "startRecord\0path\0stopRecord\0saveRecord\0"
    "startRecordMax\0startRecordMid\0"
    "startRecordMin\0setTH\0v\0startButtonClicked\0"
    "updateDVSView\0img\0updateDVDetectionView\0"
    "updateDVSDetectionView\0onDVDetectionResult\0"
    "result\0onDVSDetectionResult\0onFinalDecision\0"
    "decision\0timestamp\0onDetectionToggled\0"
    "enabled\0onModelLoadResult\0success\0"
    "message\0handleStartMaxRecord\0"
    "handleStopMaxRecord\0handleStartMidRecord\0"
    "handleStopMidRecord\0handleStartMinRecord\0"
    "handleStopMinRecord\0onPlaybackNewFrame\0"
    "frame\0onPlaybackProgress\0progress\0"
    "onPlaybackFinished\0"
    "on_selectRootPlaybackDirButton_clicked\0"
    "onBatchPlaybackStarted\0totalSessions\0"
    "onBatchPlaybackProgress\0currentSession\0"
    "currentSessionName\0onBatchPlaybackFinished\0"
    "onPlaybackImagePair\0dv\0dvs\0"
    "saveDetectionResultImage\0cameraType"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  179,    2, 0x06 /* Public */,
       3,    0,  180,    2, 0x06 /* Public */,
       4,    1,  181,    2, 0x06 /* Public */,
       6,    0,  184,    2, 0x06 /* Public */,
       7,    1,  185,    2, 0x06 /* Public */,
       8,    1,  188,    2, 0x06 /* Public */,
       9,    1,  191,    2, 0x06 /* Public */,
      10,    1,  194,    2, 0x06 /* Public */,
      11,    1,  197,    2, 0x06 /* Public */,
      13,    0,  200,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    1,  201,    2, 0x08 /* Private */,
      16,    1,  204,    2, 0x08 /* Private */,
      17,    1,  207,    2, 0x08 /* Private */,
      18,    1,  210,    2, 0x08 /* Private */,
      20,    1,  213,    2, 0x08 /* Private */,
      21,    2,  216,    2, 0x08 /* Private */,
      24,    1,  221,    2, 0x08 /* Private */,
      26,    2,  224,    2, 0x08 /* Private */,
      29,    0,  229,    2, 0x08 /* Private */,
      30,    0,  230,    2, 0x08 /* Private */,
      31,    0,  231,    2, 0x08 /* Private */,
      32,    0,  232,    2, 0x08 /* Private */,
      33,    0,  233,    2, 0x08 /* Private */,
      34,    0,  234,    2, 0x08 /* Private */,
      35,    1,  235,    2, 0x08 /* Private */,
      37,    1,  238,    2, 0x08 /* Private */,
      39,    0,  241,    2, 0x08 /* Private */,
      40,    0,  242,    2, 0x08 /* Private */,
      41,    1,  243,    2, 0x08 /* Private */,
      43,    3,  246,    2, 0x08 /* Private */,
      46,    0,  253,    2, 0x08 /* Private */,
      47,    2,  254,    2, 0x08 /* Private */,
      50,    2,  259,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QImage,   15,
    QMetaType::Void, QMetaType::QImage,   15,
    QMetaType::Void, QMetaType::QImage,   15,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void, QMetaType::QString, QMetaType::QDateTime,   22,   23,
    QMetaType::Void, QMetaType::Bool,   25,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   27,   28,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage,   36,
    QMetaType::Void, QMetaType::Double,   38,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   42,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::QString,   44,   42,   45,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage, QMetaType::QImage,   48,   49,
    QMetaType::Void, QMetaType::QImage, QMetaType::QString,   15,   51,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cameraStarted(); break;
        case 1: _t->playbackStarted(); break;
        case 2: _t->startRecord((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->stopRecord(); break;
        case 4: _t->saveRecord((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->startRecordMax((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->startRecordMid((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->startRecordMin((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->setTH((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->startButtonClicked(); break;
        case 10: _t->updateDVSView((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 11: _t->updateDVDetectionView((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 12: _t->updateDVSDetectionView((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 13: _t->onDVDetectionResult((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 14: _t->onDVSDetectionResult((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 15: _t->onFinalDecision((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QDateTime(*)>(_a[2]))); break;
        case 16: _t->onDetectionToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->onModelLoadResult((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 18: _t->handleStartMaxRecord(); break;
        case 19: _t->handleStopMaxRecord(); break;
        case 20: _t->handleStartMidRecord(); break;
        case 21: _t->handleStopMidRecord(); break;
        case 22: _t->handleStartMinRecord(); break;
        case 23: _t->handleStopMinRecord(); break;
        case 24: _t->onPlaybackNewFrame((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 25: _t->onPlaybackProgress((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 26: _t->onPlaybackFinished(); break;
        case 27: _t->on_selectRootPlaybackDirButton_clicked(); break;
        case 28: _t->onBatchPlaybackStarted((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: _t->onBatchPlaybackProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 30: _t->onBatchPlaybackFinished(); break;
        case 31: _t->onPlaybackImagePair((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< QImage(*)>(_a[2]))); break;
        case 32: _t->saveDetectionResultImage((*reinterpret_cast< const QImage(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::cameraStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::playbackStarted)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::startRecord)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::stopRecord)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::saveRecord)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::startRecordMax)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::startRecordMid)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::startRecordMin)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::setTH)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::startButtonClicked)) {
                *result = 9;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 33;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::cameraStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MainWindow::playbackStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MainWindow::startRecord(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MainWindow::stopRecord()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void MainWindow::saveRecord(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void MainWindow::startRecordMax(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void MainWindow::startRecordMid(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void MainWindow::startRecordMin(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void MainWindow::setTH(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void MainWindow::startButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
