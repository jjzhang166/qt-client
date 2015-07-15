/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristicswidget.h"

#include <QVariant>
#include <QtScript>

#include "errorReporter.h"
#include "characteristicAssignment.h"

class CharacteristicsWidgetPrivate
{
  public:
    int                    id;
    QString                type;
    QString                paramName;
    CharacteristicsWidget *parent;
    bool                   readOnly;

    CharacteristicsWidgetPrivate(CharacteristicsWidget *p)
      : id(-1),
        parent(p),
        readOnly(false)
    {
    }
};

CharacteristicsWidget::CharacteristicsWidget(QWidget* parent, const char* name, QString type, int id)
    : QWidget(parent),
      _d(0)
{
  setupUi(this);
  if (name) setObjectName(name);
  _d = new CharacteristicsWidgetPrivate(this);

  connect(_newCharacteristic,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_editCharacteristic,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDelete()));

  connect(_charass, SIGNAL(valid(bool)), _editCharacteristic,   SLOT(setEnabled(bool)));
  connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));

  _charass->addColumn(tr("Characteristic"), _itemColumn,   Qt::AlignLeft,   true, "char_name" );
  _charass->addColumn(tr("Value"),          -1,            Qt::AlignLeft,   true, "charass_value" );
  _charass->addColumn(tr("Default"),        _ynColumn*2,   Qt::AlignCenter, true, "charass_default" );

  if (! type.isEmpty()) setType(type);
  if (id >= 0)          setId(id);
}

CharacteristicsWidget::~CharacteristicsWidget()
{
  if (_d)
  {
    delete _d;
    _d = 0;
  }
}

void CharacteristicsWidget::languageChange()
{
  retranslateUi(this);
}

void CharacteristicsWidget::setReadOnly(bool newState)
{
  bool prevState = _d->readOnly;

  _d->readOnly = newState;
  _newCharacteristic->setEnabled(! newState);
  _editCharacteristic->setEnabled(! newState && _charass->id() >= 0);
  _deleteCharacteristic->setEnabled(! newState && _charass->id() >= 0);
  if (prevState != newState)
  {
    if (newState)
    {
      disconnect(_charass, SIGNAL(valid(bool)), _editCharacteristic,   SLOT(setEnabled(bool)));
      disconnect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      disconnect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
    }
    else
    {
      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic,   SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
    }
  }
}

void CharacteristicsWidget::setType(QString doctype)
{
  QString prevtype = _d->type;
  XSqlQuery q;
  q.prepare("select * from source where source_charass = :charass;");
  q.bindValue(":charass", doctype);
  q.exec();
  if (q.first())
  {
    _d->paramName = q.value("source_key_param").toString();
    _d->type = doctype;
  }
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Document Type"),
                           q, __FILE__, __LINE__))
  {
    _d->type = "";
  }
  emit valid(isValid());
  if (_d->type != prevtype) emit newType(_d->type);
}

void CharacteristicsWidget::setId(int id)
{
  int previd = _d->id;
  _d->id = (id < -1) ? -1 : id;
  sFillList();
  emit valid(isValid());
  if (_d->id != previd) emit newId(_d->id);
}

bool CharacteristicsWidget::isValid() const
{
  return (_d->id >= 0 && ! _d->type.isEmpty());
}

void CharacteristicsWidget::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append(_d->paramName, _d->id);

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void CharacteristicsWidget::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void CharacteristicsWidget::sDelete()
{
  XSqlQuery q;
  q.prepare("DELETE FROM charass WHERE charass_id = :charass_id;");
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Characteristic"),
                       q, __FILE__, __LINE__);

  sFillList();
}

void CharacteristicsWidget::sFillList()
{
  XSqlQuery q;
  q.prepare( "SELECT charass_id, char_name,"
             "       CASE WHEN char_type < 2 THEN charass_value"
             "            ELSE formatDate(charass_value::date)"
             "       END AS charass_value,"
             "       charass_default"
             "  FROM charass JOIN char ON charass_char_id = char_id"
             " WHERE charass_target_type = :type"
             "   AND charass_target_id   = :id"
             " ORDER BY char_order, char_name;" );
  q.bindValue(":type", _d->type);
  q.bindValue(":id",   _d->id);
  q.exec();
  _charass->populate(q);
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Characteristics"),
                       q, __FILE__, __LINE__);
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue CharacteristicsWidgettoScriptValue(QScriptEngine *engine, CharacteristicsWidget* const &item)
{
  return engine->newQObject(item);
}

void CharacteristicsWidgetfromScriptValue(const QScriptValue &obj, CharacteristicsWidget* &item)
{
  item = qobject_cast<CharacteristicsWidget*>(obj.toQObject());
}

QScriptValue constructCharacteristicsWidget(QScriptContext *context,
                                            QScriptEngine  *engine)
{
  QWidget *parent = (qscriptvalue_cast<QWidget*>(context->argument(0)));
  if (context->argumentCount() == 0)
    return engine->toScriptValue(new CharacteristicsWidget());
  if (context->argumentCount() == 1)
    return engine->toScriptValue(new CharacteristicsWidget(parent, "_characteristicsWidget"));
  if (context->argumentCount() == 2)
    return engine->toScriptValue(new CharacteristicsWidget(parent,
                            context->argument(1).toString().toLatin1().data()));
  if (context->argumentCount() == 3)
    return engine->toScriptValue(new CharacteristicsWidget(parent,
                            context->argument(1).toString().toLatin1().data(),
                            context->argument(2).toString()));
  return engine->toScriptValue(new CharacteristicsWidget(parent,
                          context->argument(1).toString().toLatin1().data(),
                          context->argument(2).toString(),
                          context->argument(2).toInteger()));
}

void setupCharacteristicsWidget(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags stdflags = QScriptValue::ReadOnly |
                                         QScriptValue::Undeletable;

  qScriptRegisterMetaType(engine, CharacteristicsWidgettoScriptValue,
                          CharacteristicsWidgetfromScriptValue);

  QScriptValue constructor = engine->newFunction(constructCharacteristicsWidget);
  engine->globalObject().setProperty("CharacteristicsWidget", constructor, stdflags);
}
