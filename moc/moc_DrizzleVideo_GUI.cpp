/****************************************************************************
** Meta object code from reading C++ file 'DrizzleVideo_GUI.h'
**
** Created: Sun 16. Aug 16:12:59 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../application/PlugIns/src/Drizzle/DrizzleVideo_GUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DrizzleVideo_GUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DrizzleVideo_GUI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      34,   17,   29,   17, 0x0a,
      51,   17,   17,   17, 0x0a,
      60,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DrizzleVideo_GUI[] = {
    "DrizzleVideo_GUI\0\0closeGUI()\0bool\0"
    "PerformDrizzle()\0browse()\0updateInfo()\0"
};

const QMetaObject DrizzleVideo_GUI::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DrizzleVideo_GUI,
      qt_meta_data_DrizzleVideo_GUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DrizzleVideo_GUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DrizzleVideo_GUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DrizzleVideo_GUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DrizzleVideo_GUI))
        return static_cast<void*>(const_cast< DrizzleVideo_GUI*>(this));
    return QDialog::qt_metacast(_clname);
}

int DrizzleVideo_GUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: closeGUI(); break;
        case 1: { bool _r = PerformDrizzle();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: browse(); break;
        case 3: updateInfo(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
