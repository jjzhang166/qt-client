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

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "format.h"
#include "xsqlquery.h"

#define MONEYSCALE      2
#define COSTEXTRASCALE  2
#define WEIGHTSCALE     2
#define QTYSCALE        2
#define QTYPERSCALE     6
#define SALESPRICEEXTRASCALE    2
#define PURCHPRICEEXTRASCALE    2
#define UOMRATIOSCALE   6
#define PERCENTSCALE    2

#define DEBUG false

static QColor error("red");
static QColor warning("orange");
static QColor emphasis("blue");
static QColor altemphasis("green");
static QColor expired("red");
static QColor future("blue");

static int costscale       = MONEYSCALE + COSTEXTRASCALE;
static int currvalscale    = MONEYSCALE;
static int extpricescale   = MONEYSCALE;
static int percentscale    = PERCENTSCALE;
static int purchpricescale = MONEYSCALE + PURCHPRICEEXTRASCALE;
static int qtyscale        = QTYSCALE;
static int qtyperscale     = QTYPERSCALE;
static int salespricescale = MONEYSCALE + SALESPRICEEXTRASCALE;
static int uomratioscale   = UOMRATIOSCALE;
static int weightscale     = WEIGHTSCALE;

static bool loadLocale()
{
  static bool firstTime = true;

  if (firstTime)
  {
    XSqlQuery localeq("SELECT * "
		     "FROM usr, locale LEFT OUTER JOIN"
		     "     lang ON (locale_lang_id=lang_id) LEFT OUTER JOIN"
		     "     country ON (locale_country_id=country_id) "
		     "WHERE ( (usr_username=CURRENT_USER)"
		     " AND (usr_locale_id=locale_id) );" );
    localeq.exec();
    if (localeq.first())
    {
      if (! localeq.value("locale_error_color").toString().isEmpty())
	error = QColor(localeq.value("locale_error_color").toString());
      if (! localeq.value("locale_warning_color").toString().isEmpty())
	warning = QColor(localeq.value("locale_warning_color").toString());
      if (! localeq.value("locale_emphasis_color").toString().isEmpty())
	emphasis = QColor(localeq.value("locale_emphasis_color").toString());
      if (! localeq.value("locale_altemphasis_color").toString().isEmpty())
	altemphasis = QColor(localeq.value("locale_altemphasis_color").toString());
      if (! localeq.value("locale_expired_color").toString().isEmpty())
	expired = QColor(localeq.value("locale_expired_color").toString());
      if (! localeq.value("locale_future_color").toString().isEmpty())
	future = QColor(localeq.value("locale_future_color").toString());

      if (! localeq.value("locale_cost_scale").toString().isEmpty())
        costscale       = localeq.value("locale_cost_scale").toInt();
      if (! localeq.value("locale_curr_scale").toString().isEmpty())
        currvalscale    = localeq.value("locale_curr_scale").toInt();
      if (! localeq.value("locale_extprice_scale").toString().isEmpty())
        extpricescale   = localeq.value("locale_extprice_scale").toInt();
      if (! localeq.value("locale_purchprice_scale").toString().isEmpty())
        purchpricescale = localeq.value("locale_purchprice_scale").toInt();
      if (! localeq.value("locale_qty_scale").toString().isEmpty())
        qtyscale        = localeq.value("locale_qty_scale").toInt();
      if (! localeq.value("locale_qtyper_scale").toString().isEmpty())
        qtyperscale     = localeq.value("locale_qtyper_scale").toInt();
      if (! localeq.value("locale_salesprice_scale").toString().isEmpty())
        salespricescale = localeq.value("locale_salesprice_scale").toInt();
      if (! localeq.value("locale_uomratio_scale").toString().isEmpty())
        uomratioscale   = localeq.value("locale_uomratio_scale").toInt();

      // TODO: add locale_percent_scale
      // TODO: add locale_weight_scale
    }
    else if (localeq.lastError().type() != QSqlError::None)
    {
      QMessageBox::critical(0,
                            QObject::tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__, __LINE__),
                            localeq.lastError().databaseText());
      return false;
    }
    firstTime = false;
  }
  return true;
}

int decimalPlaces(QString pName)
{
  int returnVal = MONEYSCALE;
  loadLocale();

  if (pName.contains(QRegExp("^[0-9]+$"))) returnVal = pName.toInt();
  else if (pName == "cost")       returnVal = costscale;
  else if (pName == "extprice")   returnVal = extpricescale;
  else if (pName == "percent")    returnVal = percentscale;
  else if (pName == "purchprice") returnVal = purchpricescale;
  else if (pName == "qty")        returnVal = qtyscale;
  else if (pName == "qtyper")     returnVal = qtyperscale;
  else if (pName == "salesprice") returnVal = salespricescale;
  else if (pName == "uomratio")   returnVal = uomratioscale;
  else if (pName == "weight")     returnVal = weightscale;
  else if (pName.startsWith("curr"))
    returnVal = currvalscale;  // TODO: change this to currency-specific value?

  return returnVal;
}

QColor namedColor(QString pName)
{
  loadLocale();

  if (pName == "error")
    return error;
  else if (pName == "warning")
    return warning;
  else if (pName == "emphasis")
    return emphasis;
  else if (pName == "altemphasis")
    return altemphasis;
  else if (pName == "expired")
    return expired;
  else if (pName == "future")
    return future;

  return QColor(pName);
}

QString formatNumber(double value, int decimals)
{
   if (DEBUG) qDebug("formatNumber(%f, %d)", value, decimals);
   return QLocale().toString(value, 'f', decimals);
}

/*
  different currencies have different rounding conventions, so we need
  the currency id to find the right rounding rules.
  we need extra decimal places for some data because some monetary values,
  like unit costs, are stored with extra precision.
*/
QString formatMoney(double value, int /* curr_id */, int extraDecimals)
{
  return QLocale().toString(value, 'f', currvalscale + extraDecimals);
}

QString formatCost(double value, int /* curr_id */)
{
  return formatNumber(value, costscale);
}

QString formatExtPrice(double value, int /* curr_id */)
{
  return formatNumber(value, extpricescale);
}

QString formatWeight(double value)
{
  return formatNumber(value, weightscale);
}

QString formatQty(double value)
{
  return formatNumber(value, qtyscale);
}

QString formatQtyPer(double value)
{
  return formatNumber(value, qtyperscale);
}

QString formatSalesPrice(double value, int /* curr_id */)
{
  return formatNumber(value, salespricescale);
}

QString formatPurchPrice(double value, int /* curr_id */)
{
  return formatNumber(value, purchpricescale);
}

QString formatUOMRatio(double value)
{
  return formatNumber(value, uomratioscale);
}

QString formatPercent(double value)
{
  return formatNumber(value * 100.0, percentscale);
}
