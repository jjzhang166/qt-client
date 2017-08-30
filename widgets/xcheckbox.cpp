/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xcheckbox.h"

#include <QtScript>

XCheckBox::XCheckBox(QWidget *pParent) :
  QCheckBox(pParent)
{
  connect(this, SIGNAL(objectNameChanged(const QString &)), this, SLOT(init()));
  constructor();
}

XCheckBox::XCheckBox(const QString &pText, QWidget *pParent) :
  QCheckBox(pText, pParent)
{
  connect(this, SIGNAL(objectNameChanged(const QString &)), this, SLOT(init()));
  constructor();
}

// can't make a static QPixmap 'cause Qt complains:
// Must construct a QApplication before a QPaintDevice
QPixmap *XCheckBox::_checkedIcon = 0;

void XCheckBox::constructor()
{
  _default=false;
  setForgetful(false);
  _initialized = false;

  _mapper = new XDataWidgetMapper(this);
}

void XCheckBox::setData()
{
  if (_mapper->model() &&
    _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toBool() != isChecked())
  _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), isChecked());
}

QWidget* getRootWidget(QWidget* child)
{
  while (child->parentWidget()) {
    if (child->parentWidget()->objectName() != "") {
      child = child->parentWidget();
    } else {
      break;
    }
  }

  return child;
}

void XCheckBox::init()
{
  if(_initialized)
    return;

  QString pname;
  if (parent()) {
    pname = getRootWidget(this)->objectName() + "/";
  }
  _settingsName = pname + objectName();

  if(_x_preferences)
  {
    if (!_forgetful)
      setCheckState((Qt::CheckState)(_x_preferences->value(_settingsName + "/checked").toInt()));
  }
  _initialized = true;
}

void XCheckBox::setForgetful(bool p)
{
  if (_x_preferences && !p)
    _forgetful = _x_preferences->value("XCheckBox/forgetful").startsWith("t", Qt::CaseInsensitive);
  else
    _forgetful = p;

  if (! _forgetful)
  {
    Q_INIT_RESOURCE(widgets);
    if (! _checkedIcon)
      _checkedIcon = new QPixmap(":/widgets/images/xcheckbox.png");

    setIcon(*_checkedIcon);
    setIconSize(_checkedIcon->size());
  }
  else
    setIcon(QPixmap());
}

XCheckBox::~XCheckBox()
{
  if (!_settingsName.isEmpty() && _x_preferences)
  {
    if (_forgetful)
      _x_preferences->remove(_settingsName + "/checked");
    else
      _x_preferences->set(_settingsName + "/checked", (int)checkState());
  }
}

void XCheckBox::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "checked", "defaultChecked");
  _mapper=m;
  connect(this, SIGNAL(stateChanged(int)), this, SLOT(setData()));
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue constructXCheckBox(QScriptContext *context,
                                QScriptEngine  *engine)
{
#if QT_VERSION >= 0x050000
  XCheckBox *cbox = 0;

  if (context->argumentCount() == 0)
    cbox = new XCheckBox();

  else if (context->argumentCount() == 1 &&
           context->argument(0).isString())
    cbox = new XCheckBox(context->argument(0).toString());
  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    cbox = new XCheckBox(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() == 2 &&
           context->argument(0).isString() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)))
    cbox = new XCheckBox(context->argument(0).toString(),
                         qscriptvalue_cast<QWidget*>(context->argument(1)));

  else
    context->throwError(QScriptContext::UnknownError,
                        QString("Could not find an appropriate XCheckBox constructor"));

  return engine->toScriptValue(cbox);
#else
  Q_UNUSED(context); Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupXCheckBox(QScriptEngine *engine)
{
  if (! engine->globalObject().property("XCheckBox").isFunction())
  {
    QScriptValue ctor = engine->newFunction(constructXCheckBox);
    QScriptValue meta = engine->newQMetaObject(&XCheckBox::staticMetaObject, ctor);

    engine->globalObject().setProperty("XCheckBox", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
