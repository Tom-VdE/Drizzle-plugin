/****************************************************************************
** Meta object code from reading C++ file 'Drizzle_GUI.h'
**
** Created: Sun 2. Aug 18:00:07 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../application/PlugIns/src/Drizzle/Drizzle_GUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Drizzle_GUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Drizzle_GUI[] = {

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
      13,   12,   12,   12, 0x0a,
      29,   12,   24,   12, 0x0a,
      46,   12,   12,   12, 0x0a,
      60,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Drizzle_GUI[] = {
    "Drizzle_GUI\0\0closeGUI()\0bool\0"
    "PerformDrizzle()\0updateInfo1()\0"
    "updateInfo2()\0"
};

const QMetaObject Drizzle_GUI::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_Drizzle_GUI,
      qt_meta_data_Drizzle_GUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Drizzle_GUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Drizzle_GUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Drizzle_GUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Drizzle_GUI))
        return static_cast<void*>(const_cast< Drizzle_GUI*>(this));
    return QDialog::qt_metacast(_clname);
}

int Drizzle_GUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: closeGUI(); break;
        case 1: { bool _r = PerformDrizzle();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 2: updateInfo1(); break;
        case 3: updateInfo2(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
