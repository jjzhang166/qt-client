/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "sysLocale.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

sysLocale::sysLocale(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_alternate,      SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_costScale,      SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_country,        SIGNAL(newID(int)),        this, SLOT(sUpdateSamples()));
  connect(_currencyScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_emphasis,       SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_error,          SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_expired,        SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_extPriceScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_future,         SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));
  connect(_language,       SIGNAL(newID(int)),        this, SLOT(sUpdateCountries()));
  connect(_purchPriceScale,SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyScale,       SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyPerScale,    SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_salesPriceScale,SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_save,           SIGNAL(clicked()),         this, SLOT(sSave()));
  connect(_uomRatioScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_warning,        SIGNAL(editingFinished()), this, SLOT(sUpdateColors()));

  _localeid = -1;
}

sysLocale::~sysLocale()
{
  // no need to delete child widgets, Qt does it all for us
}

void sysLocale::languageChange()
{
  retranslateUi(this);
}

enum SetResponse sysLocale::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("locale_id", &valid);
  if (valid)
  {
    _localeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      q.prepare("SELECT copyLocale(:locale_id) AS _locale_id;");
      q.bindValue(":locale_id", _localeid);
      q.exec();
      if (q.first())
      {
        _localeid = q.value("_locale_id").toInt();
	populate();
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }

      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _language->setEnabled(FALSE);
      _country->setEnabled(FALSE);
      _currencyScale->setEnabled(FALSE);
      _salesPriceScale->setEnabled(FALSE);
      _purchPriceScale->setEnabled(FALSE);
      _extPriceScale->setEnabled(FALSE);
      _costScale->setEnabled(FALSE);
      _qtyScale->setEnabled(FALSE);
      _qtyPerScale->setEnabled(FALSE);
      _uomRatioScale->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  return NoError;
}

void sysLocale::sSave()
{
  if (_code->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Locale"),
                           tr("<p>You must enter a Code for this Locale before "
                              "you may save it.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('locale_locale_id_seq') AS _locale_id");
    if (q.first())
      _localeid = q.value("_locale_id").toInt();

    q.prepare( "INSERT INTO locale "
               "( locale_id, locale_code, locale_descrip,"
               "  locale_lang_id, locale_country_id, "
	       "  locale_curr_scale,"
               "  locale_salesprice_scale, locale_purchprice_scale,"
               "  locale_extprice_scale, locale_cost_scale,"
               "  locale_qty_scale, locale_qtyper_scale,"
               "  locale_uomratio_scale,"
               "  locale_comments, "
               "  locale_error_color, locale_warning_color,"
               "  locale_emphasis_color, locale_altemphasis_color,"
               "  locale_expired_color, locale_future_color) "
               "VALUES "
               "( :locale_id, :locale_code, :locale_descrip,"
               "  :locale_lang_id, :locale_country_id,"
	       "  :locale_curr_scale,"
               "  :locale_salesprice_scale, :locale_purchprice_scale,"
               "  :locale_extprice_scale, :locale_cost_scale,"
               "  :locale_qty_scale, :locale_qtyper_scale,"
               "  :locale_uomratio_scale,"
               "  :locale_comments,"
               "  :locale_error_color, :locale_warning_color,"
               "  :locale_emphasis_color, :locale_altemphasis_color,"
               "  :locale_expired_color, :locale_future_color);" );
  }
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE locale "
                "SET locale_code=:locale_code,"
                "    locale_descrip=:locale_descrip,"
                "    locale_lang_id=:locale_lang_id,"
                "    locale_country_id=:locale_country_id,"
		"    locale_curr_scale=:locale_curr_scale,"
                "    locale_salesprice_scale=:locale_salesprice_scale,"
                "    locale_purchprice_scale=:locale_purchprice_scale,"
                "    locale_extprice_scale=:locale_extprice_scale,"
                "    locale_cost_scale=:locale_cost_scale,"
                "    locale_qty_scale=:locale_qty_scale,"
                "    locale_qtyper_scale=:locale_qtyper_scale,"
                "    locale_uomratio_scale=:locale_uomratio_scale,"
                "    locale_comments=:locale_comments,"
                "    locale_error_color=:locale_error_color,"
                "    locale_warning_color=:locale_warning_color,"
                "    locale_emphasis_color=:locale_emphasis_color,"
                "    locale_altemphasis_color=:locale_altemphasis_color,"
                "    locale_expired_color=:locale_expired_color,"
                "    locale_future_color=:locale_future_color "
                "WHERE (locale_id=:locale_id);" );

  q.bindValue(":locale_id",                _localeid);
  q.bindValue(":locale_code",              _code->text());
  q.bindValue(":locale_descrip",           _description->text());
  q.bindValue(":locale_lang_id",           _language->id());
  q.bindValue(":locale_country_id",        _country->id());
  q.bindValue(":locale_curr_scale",        _currencyScale->text());
  q.bindValue(":locale_salesprice_scale",  _salesPriceScale->text());
  q.bindValue(":locale_purchprice_scale",  _purchPriceScale->text());
  q.bindValue(":locale_extprice_scale",    _extPriceScale->text());
  q.bindValue(":locale_cost_scale",        _costScale->text());
  q.bindValue(":locale_qty_scale",         _qtyScale->text());
  q.bindValue(":locale_qtyper_scale",      _qtyPerScale->text());
  q.bindValue(":locale_uomratio_scale",    _uomRatioScale->text());
  q.bindValue(":locale_comments",          _comments->text());
  q.bindValue(":locale_error_color",       _error->text());
  q.bindValue(":locale_warning_color",     _warning->text());
  q.bindValue(":locale_emphasis_color",    _emphasis->text());
  q.bindValue(":locale_altemphasis_color", _alternate->text());
  q.bindValue(":locale_expired_color",     _expired->text());
  q.bindValue(":locale_future_color",      _future->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_localeid);
}

void sysLocale::close()
{
  qDebug("sysLocale::close()");
  if (_mode == cCopy && _localeid > 0)
  {
    q.prepare( "DELETE FROM locale "
               "WHERE (locale_id=:locale_id);" );
    q.bindValue(":locale_id", _localeid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}
 
void sysLocale::sUpdateCountries()
{
  QLocale::Language localeLang = QLocale::C;

  if (_language->id() > 0)
  {
    q.prepare("SELECT lang_qt_number FROM lang WHERE (lang_id=:langid);");
    q.bindValue(":langid", _language->id());
    q.exec();
    if (q.first())
    {
      localeLang = QLocale::Language(q.value("lang_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  int currentCountry = _country->id();

  QList<QLocale::Country> clist = QLocale::countriesForLanguage(localeLang);

  _country->clear();
  if(!clist.isEmpty())
  {
    QString sql = "SELECT country_id, country_name, country_name"
                  "  FROM country"
                  " WHERE((country_qt_number IS NOT NULL)"
                  "   AND (country_qt_number IN (-1";
    for (int i = 0; i < clist.size(); ++i)
    {
       sql.append(",");
       sql.append(QString::number((int)clist.at(i)));
    }
    sql += ")))"
           " ORDER BY country_name;";
    _country->populate(sql, currentCountry);
  }

  sUpdateSamples();
}

void sysLocale::sUpdateSamples()
{
  QLocale::Language localeLang = QLocale::C;
  QLocale::Country  localeCountry = QLocale::AnyCountry;

  if (_language->id() > 0)
  {
    q.prepare("SELECT lang_qt_number FROM lang WHERE (lang_id=:langid);");
    q.bindValue(":langid", _language->id());
    q.exec();
    if (q.first())
    {
      localeLang = QLocale::Language(q.value("lang_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_country->id() > 0)
  {
    q.prepare("SELECT country_qt_number "
              "FROM country "
              "WHERE (country_id=:countryid);");
    q.bindValue(":countryid", _country->id());
    q.exec();
    if (q.first())
    {
      localeCountry = QLocale::Country(q.value("country_qt_number").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  q.prepare("SELECT CURRENT_DATE AS dateSample, CURRENT_TIME AS timeSample,"
            "       CURRENT_TIMESTAMP AS timestampSample,"
            "       CURRENT_TIMESTAMP - CURRENT_DATE AS intervalSample,"
            "       987654321.987654321 AS doubleSample;");
  q.exec();
  if (q.first())
  {
    QLocale sampleLocale = QLocale(localeLang, localeCountry);

    _dateSample->setText(sampleLocale.toString(q.value("dateSample").toDate(), QLocale::ShortFormat));
    _timeSample->setText(sampleLocale.toString(q.value("timeSample").toTime(), QLocale::ShortFormat));
    _timestampSample->setText(sampleLocale.toString(q.value("timestampSample").toDate(), QLocale::ShortFormat) +
                              " " +
                              sampleLocale.toString(q.value("timestampSample").toTime(), QLocale::ShortFormat));
    _intervalSample->setText(sampleLocale.toString(q.value("intervalSample").toTime(), QLocale::ShortFormat));

    _currencySample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _currencyScale->value()));
    _salesPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _salesPriceScale->value()));
    _purchPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _purchPriceScale->value()));
    _extPriceSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _extPriceScale->value()));
    _costSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _costScale->value()));
    _qtySample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _qtyScale->value()));
    _qtyPerSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _qtyPerScale->value()));
    _uomRatioSample->setText(sampleLocale.toString(q.value("doubleSample").toDouble(), 'f', _uomRatioScale->value()));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void sysLocale::sUpdateColors()
{
  QList<QLineEdit*> colorwidgets;
  colorwidgets << _error        << _warning << _emphasis
               << _alternate    << _expired << _future;
  for (int i = 0; i < colorwidgets.size(); i++)
  {
    QLineEdit *widget = colorwidgets.at(i);
    QPalette colors = widget->palette();
    colors.setBrush(QPalette::Text, QBrush(QColor(widget->text())));
    widget->setPalette(colors);
  }
}

void sysLocale::populate()
{
  XSqlQuery popq;
  popq.prepare( "SELECT * "
             "FROM locale "
             "WHERE (locale_id=:locale_id);" );
  popq.bindValue(":locale_id", _localeid);
  popq.exec();
  if (popq.first())
  {
    _code->setText(popq.value("locale_code").toString());
    _description->setText(popq.value("locale_descrip").toString());
    _language->setId(popq.value("locale_lang_id").toInt());
    _country->setId(popq.value("locale_country_id").toInt());
    _currencyScale->setValue(popq.value("locale_curr_scale").toInt());
    _salesPriceScale->setValue(popq.value("locale_salesprice_scale").toInt());
    _purchPriceScale->setValue(popq.value("locale_purchprice_scale").toInt());
    _extPriceScale->setValue(popq.value("locale_extprice_scale").toInt());
    _costScale->setValue(popq.value("locale_cost_scale").toInt());
    _qtyScale->setValue(popq.value("locale_qty_scale").toInt());
    _qtyPerScale->setValue(popq.value("locale_qtyper_scale").toInt());
    _uomRatioScale->setValue(popq.value("locale_uomratio_scale").toInt());
    _comments->setText(popq.value("locale_comments").toString());
    _error->setText(popq.value("locale_error_color").toString());
    _warning->setText(popq.value("locale_warning_color").toString());
    _emphasis->setText(popq.value("locale_emphasis_color").toString());
    _alternate->setText(popq.value("locale_altemphasis_color").toString());
    _expired->setText(popq.value("locale_expired_color").toString());
    _future->setText(popq.value("locale_future_color").toString());

    sUpdateSamples();
    sUpdateColors();
  }
  if (popq.lastError().type() != QSqlError::None)
  {
    systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
