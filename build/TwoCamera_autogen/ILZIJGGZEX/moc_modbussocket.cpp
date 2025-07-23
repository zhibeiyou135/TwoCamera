/****************************************************************************
** Meta object code from reading C++ file 'modbussocket.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../socket/modbussocket.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modbussocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_modbusSocket_t {
    QByteArrayData data[66];
    char stringdata0[825];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_modbusSocket_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_modbusSocket_t qt_meta_stringdata_modbusSocket = {
    {
QT_MOC_LITERAL(0, 0, 12), // "modbusSocket"
QT_MOC_LITERAL(1, 13, 11), // "startRecord"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 10), // "stopRecord"
QT_MOC_LITERAL(4, 37, 21), // "discRecordingComplete"
QT_MOC_LITERAL(5, 59, 10), // "discNumber"
QT_MOC_LITERAL(6, 70, 21), // "autoRecordingComplete"
QT_MOC_LITERAL(7, 92, 4), // "open"
QT_MOC_LITERAL(8, 97, 6), // "rotate"
QT_MOC_LITERAL(9, 104, 6), // "revent"
QT_MOC_LITERAL(10, 111, 6), // "q_open"
QT_MOC_LITERAL(11, 118, 7), // "q_close"
QT_MOC_LITERAL(12, 126, 8), // "x_mobile"
QT_MOC_LITERAL(13, 135, 8), // "z_mobile"
QT_MOC_LITERAL(14, 144, 5), // "zData"
QT_MOC_LITERAL(15, 150, 9), // "detectRes"
QT_MOC_LITERAL(16, 160, 13), // "detectFinally"
QT_MOC_LITERAL(17, 174, 15), // "rotate_x_mobile"
QT_MOC_LITERAL(18, 190, 15), // "revent_x_mobile"
QT_MOC_LITERAL(19, 206, 7), // "click_x"
QT_MOC_LITERAL(20, 214, 12), // "click_rotate"
QT_MOC_LITERAL(21, 227, 5), // "debug"
QT_MOC_LITERAL(22, 233, 14), // "click_rotate_x"
QT_MOC_LITERAL(23, 248, 17), // "click_rotate_x_45"
QT_MOC_LITERAL(24, 266, 13), // "revent_x_step"
QT_MOC_LITERAL(25, 280, 13), // "rotate_x_step"
QT_MOC_LITERAL(26, 294, 10), // "y_micoreDV"
QT_MOC_LITERAL(27, 305, 8), // "y_micore"
QT_MOC_LITERAL(28, 314, 5), // "yData"
QT_MOC_LITERAL(29, 320, 12), // "basic_rotate"
QT_MOC_LITERAL(30, 333, 5), // "Rdata"
QT_MOC_LITERAL(31, 339, 12), // "x_basic_move"
QT_MOC_LITERAL(32, 352, 5), // "Xdata"
QT_MOC_LITERAL(33, 358, 9), // "emit_path"
QT_MOC_LITERAL(34, 368, 4), // "path"
QT_MOC_LITERAL(35, 373, 9), // "time_path"
QT_MOC_LITERAL(36, 383, 9), // "root_path"
QT_MOC_LITERAL(37, 393, 5), // "reset"
QT_MOC_LITERAL(38, 399, 5), // "begin"
QT_MOC_LITERAL(39, 405, 14), // "stop_emergency"
QT_MOC_LITERAL(40, 420, 17), // "startLoopRotation"
QT_MOC_LITERAL(41, 438, 16), // "stopLoopRotation"
QT_MOC_LITERAL(42, 455, 20), // "executeRotationCycle"
QT_MOC_LITERAL(43, 476, 17), // "testDetectFinally"
QT_MOC_LITERAL(44, 494, 13), // "testDetectRes"
QT_MOC_LITERAL(45, 508, 21), // "testDetectFinallyZero"
QT_MOC_LITERAL(46, 530, 17), // "testDetectResZero"
QT_MOC_LITERAL(47, 548, 21), // "startTrainingRotation"
QT_MOC_LITERAL(48, 570, 20), // "stopTrainingRotation"
QT_MOC_LITERAL(49, 591, 28), // "executeTrainingRotationCycle"
QT_MOC_LITERAL(50, 620, 13), // "clearMaterial"
QT_MOC_LITERAL(51, 634, 14), // "changeMaterial"
QT_MOC_LITERAL(52, 649, 20), // "performAutoRecording"
QT_MOC_LITERAL(53, 670, 16), // "recordSingleDisc"
QT_MOC_LITERAL(54, 687, 16), // "recordAtPosition"
QT_MOC_LITERAL(55, 704, 9), // "xPosition"
QT_MOC_LITERAL(56, 714, 6), // "suffix"
QT_MOC_LITERAL(57, 721, 23), // "loadAutoRecordingConfig"
QT_MOC_LITERAL(58, 745, 10), // "recordAtX1"
QT_MOC_LITERAL(59, 756, 10), // "recordAtX2"
QT_MOC_LITERAL(60, 767, 10), // "recordAtX3"
QT_MOC_LITERAL(61, 778, 10), // "recordAtX4"
QT_MOC_LITERAL(62, 789, 8), // "moveToX1"
QT_MOC_LITERAL(63, 798, 8), // "moveToX2"
QT_MOC_LITERAL(64, 807, 8), // "moveToX3"
QT_MOC_LITERAL(65, 816, 8) // "moveToX4"

    },
    "modbusSocket\0startRecord\0\0stopRecord\0"
    "discRecordingComplete\0discNumber\0"
    "autoRecordingComplete\0open\0rotate\0"
    "revent\0q_open\0q_close\0x_mobile\0z_mobile\0"
    "zData\0detectRes\0detectFinally\0"
    "rotate_x_mobile\0revent_x_mobile\0click_x\0"
    "click_rotate\0debug\0click_rotate_x\0"
    "click_rotate_x_45\0revent_x_step\0"
    "rotate_x_step\0y_micoreDV\0y_micore\0"
    "yData\0basic_rotate\0Rdata\0x_basic_move\0"
    "Xdata\0emit_path\0path\0time_path\0root_path\0"
    "reset\0begin\0stop_emergency\0startLoopRotation\0"
    "stopLoopRotation\0executeRotationCycle\0"
    "testDetectFinally\0testDetectRes\0"
    "testDetectFinallyZero\0testDetectResZero\0"
    "startTrainingRotation\0stopTrainingRotation\0"
    "executeTrainingRotationCycle\0clearMaterial\0"
    "changeMaterial\0performAutoRecording\0"
    "recordSingleDisc\0recordAtPosition\0"
    "xPosition\0suffix\0loadAutoRecordingConfig\0"
    "recordAtX1\0recordAtX2\0recordAtX3\0"
    "recordAtX4\0moveToX1\0moveToX2\0moveToX3\0"
    "moveToX4"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_modbusSocket[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      55,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  289,    2, 0x06 /* Public */,
       3,    0,  292,    2, 0x06 /* Public */,
       4,    1,  293,    2, 0x06 /* Public */,
       6,    0,  296,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,  297,    2, 0x0a /* Public */,
       8,    0,  298,    2, 0x0a /* Public */,
       9,    0,  299,    2, 0x0a /* Public */,
      10,    0,  300,    2, 0x0a /* Public */,
      11,    0,  301,    2, 0x0a /* Public */,
      12,    0,  302,    2, 0x0a /* Public */,
      13,    0,  303,    2, 0x0a /* Public */,
      13,    1,  304,    2, 0x0a /* Public */,
      15,    0,  307,    2, 0x0a /* Public */,
      16,    0,  308,    2, 0x0a /* Public */,
      17,    0,  309,    2, 0x0a /* Public */,
      18,    0,  310,    2, 0x0a /* Public */,
      19,    0,  311,    2, 0x0a /* Public */,
      20,    0,  312,    2, 0x0a /* Public */,
      21,    0,  313,    2, 0x0a /* Public */,
      22,    0,  314,    2, 0x0a /* Public */,
      23,    0,  315,    2, 0x0a /* Public */,
      24,    0,  316,    2, 0x0a /* Public */,
      25,    0,  317,    2, 0x0a /* Public */,
      26,    0,  318,    2, 0x0a /* Public */,
      27,    1,  319,    2, 0x0a /* Public */,
      29,    1,  322,    2, 0x0a /* Public */,
      31,    1,  325,    2, 0x0a /* Public */,
      33,    3,  328,    2, 0x0a /* Public */,
      37,    0,  335,    2, 0x0a /* Public */,
      38,    0,  336,    2, 0x0a /* Public */,
      39,    0,  337,    2, 0x0a /* Public */,
      40,    0,  338,    2, 0x0a /* Public */,
      41,    0,  339,    2, 0x0a /* Public */,
      42,    0,  340,    2, 0x0a /* Public */,
      43,    0,  341,    2, 0x0a /* Public */,
      44,    0,  342,    2, 0x0a /* Public */,
      45,    0,  343,    2, 0x0a /* Public */,
      46,    0,  344,    2, 0x0a /* Public */,
      47,    0,  345,    2, 0x0a /* Public */,
      48,    0,  346,    2, 0x0a /* Public */,
      49,    0,  347,    2, 0x0a /* Public */,
      50,    0,  348,    2, 0x0a /* Public */,
      51,    0,  349,    2, 0x0a /* Public */,
      52,    0,  350,    2, 0x0a /* Public */,
      53,    1,  351,    2, 0x0a /* Public */,
      54,    2,  354,    2, 0x0a /* Public */,
      57,    0,  359,    2, 0x0a /* Public */,
      58,    0,  360,    2, 0x0a /* Public */,
      59,    0,  361,    2, 0x0a /* Public */,
      60,    0,  362,    2, 0x0a /* Public */,
      61,    0,  363,    2, 0x0a /* Public */,
      62,    0,  364,    2, 0x0a /* Public */,
      63,    0,  365,    2, 0x0a /* Public */,
      64,    0,  366,    2, 0x0a /* Public */,
      65,    0,  367,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void, QMetaType::Int,   30,
    QMetaType::Void, QMetaType::Int,   32,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,   34,   35,   36,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   55,   56,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void modbusSocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<modbusSocket *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->startRecord((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->stopRecord(); break;
        case 2: _t->discRecordingComplete((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->autoRecordingComplete(); break;
        case 4: _t->open(); break;
        case 5: _t->rotate(); break;
        case 6: _t->revent(); break;
        case 7: _t->q_open(); break;
        case 8: _t->q_close(); break;
        case 9: _t->x_mobile(); break;
        case 10: _t->z_mobile(); break;
        case 11: _t->z_mobile((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->detectRes(); break;
        case 13: _t->detectFinally(); break;
        case 14: _t->rotate_x_mobile(); break;
        case 15: _t->revent_x_mobile(); break;
        case 16: _t->click_x(); break;
        case 17: _t->click_rotate(); break;
        case 18: _t->debug(); break;
        case 19: _t->click_rotate_x(); break;
        case 20: _t->click_rotate_x_45(); break;
        case 21: _t->revent_x_step(); break;
        case 22: _t->rotate_x_step(); break;
        case 23: _t->y_micoreDV(); break;
        case 24: _t->y_micore((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->basic_rotate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->x_basic_move((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->emit_path((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 28: _t->reset(); break;
        case 29: _t->begin(); break;
        case 30: _t->stop_emergency(); break;
        case 31: _t->startLoopRotation(); break;
        case 32: _t->stopLoopRotation(); break;
        case 33: _t->executeRotationCycle(); break;
        case 34: _t->testDetectFinally(); break;
        case 35: _t->testDetectRes(); break;
        case 36: _t->testDetectFinallyZero(); break;
        case 37: _t->testDetectResZero(); break;
        case 38: _t->startTrainingRotation(); break;
        case 39: _t->stopTrainingRotation(); break;
        case 40: _t->executeTrainingRotationCycle(); break;
        case 41: _t->clearMaterial(); break;
        case 42: _t->changeMaterial(); break;
        case 43: _t->performAutoRecording(); break;
        case 44: _t->recordSingleDisc((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 45: _t->recordAtPosition((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 46: _t->loadAutoRecordingConfig(); break;
        case 47: _t->recordAtX1(); break;
        case 48: _t->recordAtX2(); break;
        case 49: _t->recordAtX3(); break;
        case 50: _t->recordAtX4(); break;
        case 51: _t->moveToX1(); break;
        case 52: _t->moveToX2(); break;
        case 53: _t->moveToX3(); break;
        case 54: _t->moveToX4(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (modbusSocket::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modbusSocket::startRecord)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (modbusSocket::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modbusSocket::stopRecord)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (modbusSocket::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modbusSocket::discRecordingComplete)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (modbusSocket::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modbusSocket::autoRecordingComplete)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject modbusSocket::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_modbusSocket.data,
    qt_meta_data_modbusSocket,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *modbusSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *modbusSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_modbusSocket.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int modbusSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 55)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 55;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 55)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 55;
    }
    return _id;
}

// SIGNAL 0
void modbusSocket::startRecord(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void modbusSocket::stopRecord()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void modbusSocket::discRecordingComplete(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void modbusSocket::autoRecordingComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
