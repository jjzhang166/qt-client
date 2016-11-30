/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/* Important note: currently this window is only called from Setup, which
   wraps everything in a transaction. If this changes we'll have to revisit
   this code to add transaction handling and shift error reporting.
 */

#include "characteristic.h"

#include <QDebug>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QSqlError>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QVariant>

#include <metasql.h>
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

class characteristicPrivate {
  public:
    int idCol;
    int charIdCol;
    int valueCol;
    int orderCol;

    int charid;
    QSqlTableModel *charoptModel;
    QMap<QString, QCheckBox*> checkboxMap;
    int mode;
    characteristic *parent;
    QCheckBox *firstWidget;

    characteristicPrivate(characteristic *p)
      : parent(p)
    {
      charid = -1;
      firstWidget = 0;

      charoptModel = new QSqlTableModel;
      charoptModel->setTable("charopt");
      charoptModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

      idCol     = charoptModel->fieldIndex("charopt_id");
      charIdCol = charoptModel->fieldIndex("charopt_char_id");
      valueCol  = charoptModel->fieldIndex("charopt_value");
      orderCol  = charoptModel->fieldIndex("charopt_order");

      int counter   = 0;
      int colHeight = 7;
      QGroupBox   *useGroup = parent->findChild<QGroupBox*>("_useGroup");
      QLayout     *lyt      = useGroup->layout();
      QGridLayout *grid     = qobject_cast<QGridLayout*>(lyt);

      XSqlQuery q("SELECT source_charass, source_descrip"
                  "  FROM source"
                  "  JOIN pg_class c on source_table = relname AND relkind = 'r'"
                  "  JOIN pg_namespace n on relnamespace = n.oid"
                  "  JOIN regexp_split_to_table(buildSearchPath(), ',\\s*') sp"
                  "       on nspname = sp"
                  " WHERE source_charass not in ('', 'CT')" // bug 25940
                  " ORDER BY source_descrip;");
      // make the window a max of 4 columns of checkboxes wide
      if (q.size() > 4 * colHeight)
      {
        colHeight = q.size() / 4 + 1;
      }

      while (q.next())
      {
        QString    descrip = q.value("source_descrip").toString();
        QCheckBox *cb      = new QCheckBox(parent->tr(descrip.toLatin1()), parent);
        QString    abbr    = q.value("source_charass").toString();
        cb->setObjectName(QString("_") + abbr.toLower());
        if (DEBUG) qDebug() << abbr << cb->objectName();
        checkboxMap.insert(abbr, cb);
        grid->addWidget(cb, counter % colHeight, counter / colHeight);
        counter++;

        if (! firstWidget)
          firstWidget = cb;
      }
      ErrorReporter::error(QtCriticalMsg, parent,
                           parent->tr("Error Finding Characteristic Information"),
                           q, __FILE__, __LINE__);
      setMode(cView); // must follow building the checkboxes
    }

    bool updateCharUse(int charId, QString targetType, bool isUsed)
    {
      QCheckBox *cb = checkboxMap.value(targetType);
      if (cb)
      {
        QString str;
        if (isUsed)
          str = "INSERT INTO charuse (charuse_char_id, charuse_target_type)"
                " SELECT :char_id, :target_type"
                "  WHERE NOT EXISTS (SELECT 1"
                "                      FROM charuse"
                "                     WHERE charuse_char_id = :char_id"
                "                       AND charuse_target_type = :target_type);";
        else
          str = "DELETE FROM charuse"
                " WHERE charuse_char_id = :char_id"
                "   AND charuse_target_type = :target_type;";

        XSqlQuery q;
        q.prepare(str);
        q.bindValue(":char_id",     charId);
        q.bindValue(":target_type", targetType);
        q.exec();
        ErrorReporter::error(QtCriticalMsg, parent,
                             parent->tr("Error Saving Characteristic Usage"),
                             q, __FILE__, __LINE__);
        return (q.lastError().type() == QSqlError::NoError);
      }
      return false;
    }

  public slots:
    void setId(int id)
    {
      charid = id;
    }
    void setMode(int pMode)
    {
      mode = pMode;
      foreach (QCheckBox *cb, checkboxMap.values())
      {
        cb->setEnabled(mode != cView);
      }
    }
};

characteristic::characteristic(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _d = new characteristicPrivate(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_charoptView, SIGNAL(clicked(QModelIndex)), this, SLOT(sCharoptClicked(QModelIndex)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _validator->append(0, "[Y|N]");
  _validator->append(1, "\\S+");
  _validator->append(2, "[1-9]\\d{0,3}");
  _validator->append(3, "[A-Z]\\d{5}[1-9]");
  _validator->append(4, "(https?:\\/\\/(?:www\\.|(?!www))[^\\s\\.]+\\.[^\\s]{2,}|www\\.[^\\s]+\\.[^\\s]{2,})");
}

characteristic::~characteristic()
{
  if (_d)
  {
    delete _d;
    _d = 0;
  }
}

void characteristic::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristic::set(const ParameterList &pParams)
{
  XSqlQuery characteristicet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("char_id", &valid);
  if (valid)
  {
    _d->setId(param.toInt());
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _d->setMode(cNew);

      characteristicet.exec("SELECT NEXTVAL('char_char_id_seq') AS char_id;");
      if (characteristicet.first())
        _d->setId(characteristicet.value("char_id").toInt());

      sFillList();
    }
    else if (param.toString() == "edit")
    {
      _d->setMode(cEdit);
    }
    else if (param.toString() == "view")
    {
      _d->setMode(cView);
      _name->setEnabled(false);
      _search->setEnabled(false);
      _unique->setEnabled(false);
      _useGroup->setEnabled(false);
      _order->setEnabled(false);
      _mask->setEnabled(false);
      _validator->setEnabled(false);

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void characteristic::sSave()
{
// TODO: verify that _mask      applies to all existing charass
// TODO: verify that _validator applies to all existing charass
  XSqlQuery characteristicSave;

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                        tr("<p>You must name this Characteristic before saving it."));

  if(GuiErrorCheck::reportErrors(this,tr("Unable To Save Characteristic"),errors))
      return;

   bool allClear = true;
  foreach (QCheckBox *cb, _d->checkboxMap.values())
  {
    if (cb && cb->isChecked())
    {
      allClear = false;
      break;
    }
  }
  
  if (allClear)
  {
    QMessageBox::critical(this, tr("Apply Characteristic"),
			  tr("<p>You must apply this Characteristic to at "
			     "least one type of application object."));
    _d->firstWidget->setFocus();
    return;
  }

  QStringList values;
  for (int i = 0; i < _d->charoptModel->rowCount(); i++)
  {
    QString data = _d->charoptModel->data(_d->charoptModel->index(i,_d->valueCol), Qt::EditRole).toString();
    if (values.contains(data))
    {
      QMessageBox::critical(this, tr("Error"), tr("Option list may not contain duplicates."));
      return;
    }
    values.append(data);
  }

  if (_d->mode == cNew)
  {
    characteristicSave.prepare( "INSERT INTO char "
               "( char_id, char_name, char_options, char_attributes,"
               "  char_notes, char_mask, char_validator, char_type, "
               "  char_order, char_search, char_unique ) "
               "VALUES "
               "( :char_id, :char_name, :char_options, :char_attributes,"
               "  :char_notes, :char_mask, :char_validator, :char_type, "
               "  :char_order, :char_search, :char_unique );" );

    characteristicSave.bindValue(":char_type", _type->currentIndex());
  }
  else if (_d->mode == cEdit)
    characteristicSave.prepare( "UPDATE char "
               "SET char_name=:char_name, "
               "    char_options=:char_options,"
               "    char_attributes=:char_attributes, "
               "    char_notes=:char_notes,"
               "    char_mask=:char_mask,"
               "    char_validator=:char_validator, "
               "    char_order=:char_order, "
               "    char_search=:char_search, "
               "    char_unique=:char_unique "
               "WHERE (char_id=:char_id);" );

  characteristicSave.bindValue(":char_id", _d->charid);
  characteristicSave.bindValue(":char_name", _name->text());

  characteristicSave.bindValue(":char_options",     QVariant(false));
  characteristicSave.bindValue(":char_attributes",  QVariant(false));
  characteristicSave.bindValue(":char_notes",       _description->toPlainText().trimmed());

  if (_mask->currentText().trimmed().size() > 0)
    characteristicSave.bindValue(":char_mask",        _mask->currentText());
  if (_validator->currentText().trimmed().size() > 0)
    characteristicSave.bindValue(":char_validator",   _validator->currentText());
  characteristicSave.bindValue(":char_order", _order->value());
  characteristicSave.bindValue(":char_search", QVariant(_search->isChecked()));
  characteristicSave.bindValue(":char_unique", QVariant(_unique->isChecked()));
  characteristicSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Characteristic"),
                           characteristicSave, __FILE__, __LINE__))
  {
    return;
  }

  foreach (QString targetType, _d->checkboxMap.keys())
  {
    QCheckBox *cb = _d->checkboxMap.value(targetType);
    if (! _d->updateCharUse(_d->charid, targetType, cb->isChecked()))
    {
      cb->setFocus();
      return;
    }
  }

  _d->charoptModel->submitAll();

  done(_d->charid);
}

void characteristic::sCheck()
{
  XSqlQuery characteristicCheck;
  _name->setText(_name->text().trimmed());
  if ((_d->mode == cNew) && (_name->text().trimmed().length()))
  {
    characteristicCheck.prepare( "SELECT char_id "
               "FROM char "
               "WHERE (UPPER(char_name)=UPPER(:char_name));" );
    characteristicCheck.bindValue(":char_name", _name->text());
    characteristicCheck.exec();
    if (characteristicCheck.first())
    {
      _d->setId(characteristicCheck.value("char_id").toInt());
      _d->setMode(cEdit);
      populate();

      _name->setEnabled(false);
    }
  }
}

void characteristic::populate()
{
  XSqlQuery charq;

  charq.prepare("SELECT * FROM char WHERE (char_id=:char_id);");
  charq.bindValue(":char_id", _d->charid);
  charq.exec();
  if (charq.first())
  {
    _name->setText(charq.value("char_name").toString());
    _description->setText(charq.value("char_notes").toString());
    _mask->setText(charq.value("char_mask").toString());
    _validator->setText(charq.value("char_validator").toString());
    _type->setCurrentIndex(charq.value("char_type").toInt());
    _type->setEnabled(false);
    _order->setValue(charq.value("char_order").toInt());
    _search->setChecked(charq.value("char_search").toBool());
    _unique->setChecked(charq.value("char_unique").toBool());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error Getting Characteristic"),
                           charq, __FILE__, __LINE__))
  {
    return;
  }

  charq.prepare("SELECT * FROM charuse WHERE charuse_char_id=:char_id;");
  charq.bindValue(":char_id", _d->charid);
  charq.exec();
  while (charq.next())
  {
    QCheckBox *cb = _d->checkboxMap.value(charq.value("charuse_target_type")
                                               .toString());
    if (DEBUG)
      qDebug() << charq.value("charuse_target_type")
               << (cb ? cb->objectName() : "[no cb]");
    if (cb)
      cb->setChecked(true);
  }
  ErrorReporter::error(QtCriticalMsg, this,
                       tr("Error Getting Characteristic"),
                       charq, __FILE__, __LINE__);

  sFillList();
}

void characteristic::sFillList()
{
  QString filter = QString("charopt_char_id=%1").arg(_d->charid);
  _d->charoptModel->setFilter(filter);
  _d->charoptModel->setSort(_d->orderCol, Qt::AscendingOrder);
  _d->charoptModel->select();
  _d->charoptModel->setHeaderData(_d->valueCol, Qt::Horizontal, QVariant(tr("Value")));
  _d->charoptModel->setHeaderData(_d->orderCol, Qt::Horizontal, QVariant(tr("Order")));

  _charoptView->setModel(_d->charoptModel);
  for (int i = 0; i < _d->charoptModel->columnCount(); i++) {
    if (DEBUG)
    {
      qDebug() << i << _d->valueCol << _d->orderCol
               << (i != _d->valueCol && i !=_d->orderCol);
    }
    _charoptView->setColumnHidden(i, i != _d->valueCol && i !=_d->orderCol);
  }
}

void characteristic::sNew()
{
  int row = _d->charoptModel->rowCount();
  _d->charoptModel->insertRows(row,1);
  _d->charoptModel->setData(_d->charoptModel->index(row, _d->charIdCol), QVariant(_d->charid));
  _d->charoptModel->setData(_d->charoptModel->index(row, _d->orderCol), 0);
  QModelIndex idx = _d->charoptModel->index(row, _d->idCol);
  _charoptView->selectionModel()->select(QItemSelection(idx, idx),
                                         QItemSelectionModel::ClearAndSelect |
                                         QItemSelectionModel::Rows);
}

void characteristic::sDelete()
{
  int row = _charoptView->selectionModel()->currentIndex().row();
  QVariant value = _d->charoptModel->data(_d->charoptModel->index(row, _d->valueCol));

  // Validate
  XSqlQuery qry;
  qry.prepare("SELECT charass_id "
              "FROM charass "
              "WHERE ((charass_char_id=:char_id) "
              " AND (charass_value=:value));");
  qry.bindValue(":char_id", _d->charid);
  qry.bindValue(":value", value);
  qry.exec();
  if (qry.first())
  {
    QMessageBox::critical(this, tr("Error"), tr("This value is in use and can not be deleted."));
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Characteristic Option"),
                                qry, __FILE__, __LINE__))
  {
    return;
  }

  _d->charoptModel->removeRows(row,  _d->charIdCol);
  _charoptView->setRowHidden(row, QModelIndex(), true);
}

void characteristic::sCharoptClicked(QModelIndex idx)
{
  _delete->setEnabled(idx.isValid());
}

