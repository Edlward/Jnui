/****************************************************************************
** Meta object code from reading C++ file 'BaiduMap.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../BaiduMap.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BaiduMap.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_JsBridge_t {
    QByteArrayData data[10];
    char stringdata0[101];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_JsBridge_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_JsBridge_t qt_meta_stringdata_JsBridge = {
    {
QT_MOC_LITERAL(0, 0, 8), // "JsBridge"
QT_MOC_LITERAL(1, 9, 13), // "sigUpdateData"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 7), // "GetData"
QT_MOC_LITERAL(4, 32, 11), // "GetWarnings"
QT_MOC_LITERAL(5, 44, 14), // "SwitchDataView"
QT_MOC_LITERAL(6, 59, 13), // "GetCenterName"
QT_MOC_LITERAL(7, 73, 19), // "GetMeasureDataOfPos"
QT_MOC_LITERAL(8, 93, 3), // "lon"
QT_MOC_LITERAL(9, 97, 3) // "lat"

    },
    "JsBridge\0sigUpdateData\0\0GetData\0"
    "GetWarnings\0SwitchDataView\0GetCenterName\0"
    "GetMeasureDataOfPos\0lon\0lat"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_JsBridge[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   45,    2, 0x0a /* Public */,
       4,    0,   46,    2, 0x0a /* Public */,
       5,    0,   47,    2, 0x0a /* Public */,
       6,    0,   48,    2, 0x0a /* Public */,
       7,    2,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::QVariantList,
    QMetaType::QVariantList,
    QMetaType::Void,
    QMetaType::QString,
    QMetaType::QString, QMetaType::QString, QMetaType::QString,    8,    9,

       0        // eod
};

void JsBridge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<JsBridge *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigUpdateData(); break;
        case 1: { QVariantList _r = _t->GetData();
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 2: { QVariantList _r = _t->GetWarnings();
            if (_a[0]) *reinterpret_cast< QVariantList*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->SwitchDataView(); break;
        case 4: { QString _r = _t->GetCenterName();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 5: { QString _r = _t->GetMeasureDataOfPos((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (JsBridge::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&JsBridge::sigUpdateData)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject JsBridge::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_JsBridge.data,
    qt_meta_data_JsBridge,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *JsBridge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *JsBridge::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_JsBridge.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int JsBridge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void JsBridge::sigUpdateData()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
