/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristicAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QRegExpValidator>

#include <metasql.h>
#include "errorReporter.h"
#include "format.h"
#include "widgets.h"
#include "xdoublevalidator.h"

// macros to emulate values from guiclient.h
// we moved this file from guiclient to widgets
#define NoError 0
#define Error_NoSetup 6
// macros to handle characteristic enum values - must match guiclient/characteristic.h
#define CHARTEXT 0
#define CHARLIST 1
#define CHARDATE 2

class CharacteristicAssignmentPrivate
{
  public:
    static QMap<QString, QString> targetTypeMap;
    characteristicAssignment     *parent;
    QString                       targetType;
    bool                          _template;
    int                           idCol;
    int                           nameCol;
    XDoubleValidator             *priceVal;
    int                           typeCol;

    CharacteristicAssignmentPrivate(characteristicAssignment *p)
      : parent(p),
        _template(false),
        idCol(0),
        nameCol(0),
        typeCol(0)
    {
      priceVal = new XDoubleValidator(0, 9999999.0, decimalPlaces("purchprice"), parent);
      if (targetTypeMap.isEmpty())
      {
        XSqlQuery q("SELECT * FROM source WHERE source_charass != '';");
        while (q.next())
        {
          targetTypeMap.insert(q.value("source_charass").toString(),
                               parent->tr(q.value("source_descrip")
                                           .toString().toLatin1()));
        }
      }
    }
    void handleTargetType();
};

QMap<QString, QString> CharacteristicAssignmentPrivate::targetTypeMap;

characteristicAssignment::characteristicAssignment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
  setupUi(this);

  if (name) setObjectName(name);
  setModal(modal);

  _d = new CharacteristicAssignmentPrivate(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_char, SIGNAL(currentIndexChanged(int)), this, SLOT(sHandleChar()));

  _listpriceLit->hide();
  _listprice->hide();
  _listprice->setValidator(_d->priceVal);

  adjustSize();
}

characteristicAssignment::~characteristicAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristicAssignment::languageChange()
{
  retranslateUi(this);
}

int characteristicAssignment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  /* derive the targetType from the source table and pParams,
     skipping params we know don't describe targets.
   */
  QStringList passedIn;
  foreach (Parameter p, pParams)
  {
    if (p.name() != "charass_id" && p.name() != "char_id" &&
        p.name() != "mode"       && p.name() != "showPrices")
    {
      passedIn << p.name();
    }
  }
  if (! passedIn.isEmpty())
  {
    ParameterList srcp;
    srcp.append("paramName", passedIn);

    MetaSQLQuery srcm("SELECT source.* FROM source WHERE source_key_param IN ("
                      "<? foreach('paramName') ?>"
                      "  <? if not isfirst('paramName') ?>, <? endif ?>"
                      "  <? value('paramName') ?>"
                      "<? endforeach ?>);");
    XSqlQuery srcq = srcm.toQuery(srcp);
    if (srcq.first())
    {
      QString paramName = srcq.value("source_key_param").toString();
      param = pParams.value(paramName, &valid);
      if (valid)
      {
        _targetId = param.toInt();
        _d->targetType = srcq.value("source_charass").toString();
        _d->handleTargetType();
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error Finding Characteristic Information"),
                                  srcq, __FILE__, __LINE__))
    {
      return Error_NoSetup;
    }
  }

  param = pParams.value("charass_id", &valid);
  if (valid)
  {
    _charassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(false);
      _value->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("showPrices", &valid);
  if (valid)
  {
    _listpriceLit->show();
    _listprice->show();
  }

  param = pParams.value("char_id", &valid);
  if (valid)
  {
    for (int i = 0; i < _char->model()->rowCount(); i++)
    {
      QModelIndex idx = _char->model()->index(i, _d->idCol);
      if (_char->model()->data(idx) == param)
        _char->setCurrentIndex(i);
    }
  }

  return NoError;
}

void characteristicAssignment::sSave()
{
  if(_d->targetType == "I" || _d->targetType == "ITEMGRP")
  {
    if ( ((_stackedWidget->currentIndex() == CHARTEXT) && (_value->text().trimmed() == "")) ||
         ((_stackedWidget->currentIndex() == CHARLIST) && (_listValue->currentText() == "")) ||
         ((_stackedWidget->currentIndex() == CHARDATE) && (_dateValue->date().toString() == "")) )
      {
          QMessageBox::information( this, tr("No Value Entered"),
                                    tr("You must enter a value before saving this Characteristic.") );
          return;
      }
  }

  XSqlQuery characteristicSave;
  if (_char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)) == -1)
  {
    QMessageBox::information( this, tr("No Characteristic Selected"),
                              tr("You must select a Characteristic before saving this Characteristic Assignment.") );
    _char->setFocus();
    return;
  }
  if (_mode == cNew &&
      _d->_template &&
      _stackedWidget->currentIndex() == CHARDATE)
  {
    characteristicSave.prepare("SELECT charass_id "
              "FROM charass "
              "WHERE ((charass_char_id=:charass_char_id) "
              "  AND (charass_target_id=:charass_target_id) "
              "  AND (charass_target_type=:charass_target_type));");
    characteristicSave.bindValue(":charass_target_id", _targetId);
    characteristicSave.bindValue(":charass_target_type", _d->targetType);
    characteristicSave.bindValue(":charass_char_id", _char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)));
    characteristicSave.exec();
    if (characteristicSave.first())
    {
      QMessageBox::critical(this, tr("Error"), tr("You can not use the same characteristic "
                                                  "for date type characteristics more than "
                                                  "once in this context."));
      return;
    }
  }

  if (_mode == cNew)
  {
    characteristicSave.exec("SELECT NEXTVAL('charass_charass_id_seq') AS charass_id;");
    if (characteristicSave.first())
    {
      _charassid = characteristicSave.value("charass_id").toInt();

      characteristicSave.prepare( "INSERT INTO charass "
                 "( charass_id, charass_target_id, charass_target_type, charass_char_id, charass_value, charass_price, charass_default ) "
                 "VALUES "
                 "( :charass_id, :charass_target_id, :charass_target_type, :charass_char_id, :charass_value, :charass_price, :charass_default );" );
    }
  }
  else if (_mode == cEdit)
    characteristicSave.prepare( "UPDATE charass "
               "SET charass_char_id=:charass_char_id, charass_value=:charass_value, "
               "charass_price=:charass_price, charass_default=:charass_default "
               "WHERE (charass_id=:charass_id);" );

  characteristicSave.bindValue(":charass_id", _charassid);
  characteristicSave.bindValue(":charass_target_id", _targetId);
  characteristicSave.bindValue(":charass_target_type", _d->targetType);
  characteristicSave.bindValue(":charass_char_id", _char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)));
  if (_stackedWidget->currentIndex() == CHARTEXT)
    characteristicSave.bindValue(":charass_value", _value->text());
  else if (_stackedWidget->currentIndex() == CHARLIST)
    characteristicSave.bindValue(":charass_value", _listValue->currentText());
  else if (_stackedWidget->currentIndex() == CHARDATE)
    characteristicSave.bindValue(":charass_value", _dateValue->date());
  characteristicSave.bindValue(":charass_price", _listprice->toDouble());
  characteristicSave.bindValue(":charass_default", QVariant(_default->isChecked()));
  characteristicSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Characteristic"),
                           characteristicSave, __FILE__, __LINE__))
    return;

  done(_charassid);
}

void characteristicAssignment::sCheck()
{
  XSqlQuery characteristicCheck;
  if ((_mode == cNew) || (_char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)) == -1))
  {
    characteristicCheck.prepare( "SELECT charass_id "
               "FROM charass "
               "WHERE ( (charass_target_type=:charass_target_id)"
               " AND (charass_target_id=:charass_target_id)"
               " AND (charass_char_id=:char_id) );" );
    characteristicCheck.bindValue(":charass_target_type", _d->targetType);
    characteristicCheck.bindValue(":charass_target_id", _targetId);
    characteristicCheck.bindValue(":char_id", _char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)));
    characteristicCheck.exec();
    if (characteristicCheck.first())
    {
      _charassid = characteristicCheck.value("charass_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void characteristicAssignment::populate()
{
  XSqlQuery characteristicpopulate;
  characteristicpopulate.prepare("SELECT charass.*, char_type"
             "  FROM charass "
             "  JOIN char ON (charass_char_id=char_id)"
             " WHERE (charass_id=:charass_id);" );
  characteristicpopulate.bindValue(":charass_id", _charassid);
  characteristicpopulate.exec();
  if (characteristicpopulate.first())
  {
    _targetId = characteristicpopulate.value("charass_target_id").toInt();
    _d->targetType = characteristicpopulate.value("charass_target_type").toString();
    _d->handleTargetType();

    for (int i = 0; i < _char->model()->rowCount(); i++)
    {
      QModelIndex idx = _char->model()->index(i, _d->idCol);
      if (_char->model()->data(idx) == characteristicpopulate.value("charass_char_id").toInt())
        _char->setCurrentIndex(i);
    }
    _listprice->setDouble(characteristicpopulate.value("charass_price").toDouble());
    _default->setChecked(characteristicpopulate.value("charass_default").toBool());
    int chartype = _char->model()->data(_char->model()->index(_char->currentIndex(),
                                                              _d->typeCol)).toInt();
    if (chartype == CHARTEXT)
      _value->setText(characteristicpopulate.value("charass_value").toString());
    else if (chartype == CHARLIST)
    {
      int idx = _listValue->findText(characteristicpopulate.value("charass_value").toString());
      _listValue->setCurrentIndex(idx);
    }
    else if (chartype == CHARDATE)
      _dateValue->setDate(characteristicpopulate.value("charass_value").toDate());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Characteristic Assignment"),
                                characteristicpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void characteristicAssignment::sHandleChar()
{
  QModelIndex midx = _char->model()->index(_char->currentIndex(), _d->typeCol); // char_type from model->setQuery
  int sidx = _char->model()->data(midx).toInt();

  _stackedWidget->setCurrentIndex(sidx);

  if (sidx == CHARTEXT)
  {
    XSqlQuery mask;
    mask.prepare( "SELECT COALESCE(char_mask, '') AS char_mask,"
                  "       COALESCE(char_validator, '.*') AS char_validator "
                  "FROM char "
                  "WHERE (char_id=:char_id);" );
    mask.bindValue(":char_id", _char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)).toInt());
    mask.exec();
    if (mask.first())
    {
      _value->setInputMask(mask.value("char_mask").toString());
      QRegExp rx(mask.value("char_validator").toString());
      QValidator *validator = new QRegExpValidator(rx, this);
      _value->setValidator(validator);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Characteristic Information"),
                                  mask, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (sidx == CHARLIST)
  {
    QSqlQuery qry;
    qry.prepare("SELECT charopt_id, charopt_value "
                 "FROM charopt "
                 "WHERE (charopt_char_id=:char_id) "
                 "ORDER BY charopt_order, charopt_value;");
    qry.bindValue(":char_id", _char->model()->data(_char->model()->index(_char->currentIndex(), _d->idCol)).toInt());
    qry.exec();
    _listValue->populate(qry);
  }

  if (sidx != CHARDATE && _d->_template)
    _default->setVisible(true);
  else
  {
    _default->setVisible(false);
    _default->setChecked(false);
  }

}

void CharacteristicAssignmentPrivate::handleTargetType()
{
  QString charuseTargetType = targetType;

  if (targetType == "I" || targetType == "ITEMGRP")
  {
    _template=true;
  }
  else if (targetType == "CT")
  {
    _template=true;
    charuseTargetType = "C"; // bug 25940
  }
  else
    parent->_default->hide();

  if (targetType != "I")
    parent->_listprice->hide();

  parent->setWindowTitle(parent->tr("Characteristic: %1").arg(targetTypeMap.value(targetType)));

  QSqlQueryModel *model = new QSqlQueryModel;
  model->setQuery("SELECT char_id, char_name, char_type"
                  "  FROM char JOIN charuse ON char_id = charuse_char_id"
                  " WHERE charuse_target_type = '" + charuseTargetType + "'"
                  " ORDER BY char_order, char_name");
  parent->_char->setModel(model);
  idCol   = model->query().record().indexOf("char_id");
  nameCol = model->query().record().indexOf("char_name");
  typeCol = model->query().record().indexOf("char_type");
  parent->_char->setModelColumn(nameCol); // char_name
  parent->sHandleChar();
}

