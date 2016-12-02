/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCheck.h"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrintDialog>
#include <QSqlError>

#include <metasql.h>
#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>

#include "xtsettings.h"
#include "confirmAchOK.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

QString printCheck::eftFileDir = QString();

printCheck::printCheck(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_check,     SIGNAL(newID(int)), this, SLOT(sEnableCreateEFT()));
  connect(_createEFT, SIGNAL(clicked()),  this, SLOT(sCreateEFT()));
  connect(_print,     SIGNAL(clicked()),  this, SLOT(sPrint()));
  connect(_printed,   SIGNAL(clicked()),  this, SLOT(sPrintedAlready()));

  _nextCheckNum->setValidator(new QIntValidator(1,999999999));

  _captive = false;
  _setCheckNumber = -1;

  _check->setAllowNull(true);

  _bankaccnt->setType(XComboBox::APBankAccounts);
  sHandleBankAccount(_bankaccnt->id());

  _createEFT->setVisible(_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"));
}

printCheck::~printCheck()
{
  // no need to delete child widgets, Qt does it all for us
}

void printCheck::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printCheck::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("check_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(false);
    _check->setEnabled(false);
  }

  return NoError;
}

void printCheck::sPrint()
{
  sPrintImpl(false);
}

void printCheck::sPrintedAlready()
{
  sPrintImpl(true);
}

void printCheck::sPrintImpl(bool printedAlready)
{
  XSqlQuery printPrint;
  if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt())
  {
    printPrint.prepare("SELECT checkhead_id "
              "FROM checkhead "
              "WHERE ( (checkhead_bankaccnt_id=:bankaccnt_id) "
              "  AND   (checkhead_id <> :checkhead_id) "
              "  AND   (checkhead_number=:nextCheckNumber));");
    printPrint.bindValue(":bankaccnt_id", _bankaccnt->id());
    printPrint.bindValue(":checkhead_id", _check->id());
    printPrint.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
    printPrint.exec();
    if (printPrint.first())
    {
      QMessageBox::information( this, tr("Check Number Already Used"),
                    tr("<p>The selected Check Number has already been used.") );
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                  printPrint, __FILE__, __LINE__))
    {
      return;
    }
  }

  if (_createEFT->isEnabled() &&
      !printedAlready &&
      QMessageBox::question(this, tr("Print Anyway?"),
                            tr("<p>The recipient of this check has been "
                               "configured for EFT transactions. Do you want "
                               "to print this check for them anyway?<p>If you "
                               "answer 'Yes' then a check will be printed. "
                               "If you say 'No' then you should click %1. ")
                            .arg(_createEFT->text()),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::No)
    return;
  printPrint.prepare( "SELECT checkhead_printed, form_report_name AS report_name, bankaccnt_id "
             "FROM checkhead, bankaccnt, form "
             "WHERE ((checkhead_bankaccnt_id=bankaccnt_id)"
             "  AND  (bankaccnt_check_form_id=form_id)"
             "  AND  (checkhead_id=:checkhead_id) );" );
  printPrint.bindValue(":checkhead_id", _check->id());
  printPrint.exec();
  if (printPrint.first())
  {
    if(printPrint.value("checkhead_printed").toBool())
    {
      QMessageBox::information( this, tr("Check Already Printed"),
		    tr("<p>The selected Check has already been printed.") );
      return;
    }
    QString reportname = printPrint.value("report_name").toString();

// get the report definition out of the database
// this should somehow be condensed into a common code call or something
// in the near future to facilitate easier conversion in other places
// of the application to use the new rendering engine directly
    XSqlQuery report;
    report.prepare( "SELECT report_source "
                    "  FROM report "
                    " WHERE (report_name=:report_name) "
                    "ORDER BY report_grade DESC LIMIT 1;" );
    report.bindValue(":report_name", reportname);
    report.exec();
    QDomDocument docReport;
    if (report.first())
    {
      QString errorMessage;
      int     errorLine;
  
      if (!docReport.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                             report, __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                           report, __FILE__, __LINE__);
      return;
    }
// end getting the report definition out the database

    if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt())
    {
      printPrint.prepare("SELECT setNextCheckNumber(:bankaccnt_id, :nextCheckNumber) AS result;");
      printPrint.bindValue(":bankaccnt_id", _bankaccnt->id());
      printPrint.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
      printPrint.exec();
      if (printPrint.first())
      {
        int result = printPrint.value("result").toInt();
        if (result < 0)
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                 storedProcErrorLookup("setNextCheckNumber", result),
                                 __FILE__, __LINE__);
          return;
        }
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                    printPrint, __FILE__, __LINE__))
      {
        return;
      }
    }

    printPrint.prepare("UPDATE checkhead SET checkhead_number=fetchNextCheckNumber(checkhead_bankaccnt_id)"
              " WHERE(checkhead_id=:checkhead_id);");
    printPrint.bindValue(":checkhead_id", _check->id());
    printPrint.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Check Information"),
                                  printPrint, __FILE__, __LINE__))
    {
      return;
    }

    if(printedAlready)
    {
      markCheckAsPrinted(_check->id());
      return;
    }

    ParameterList params;

    params.append("checkhead_id", _check->id());

// replace with new renderer code so we can get a page count
    ORPreRender pre;
    pre.setDom(docReport);
    pre.setParamList(params);
    ORODocument * doc = pre.generate();

    ReportPrinter printer(QPrinter::HighResolution);
    ORPrintRender render;
    render.setupPrinter(doc, &printer);

    QPrintDialog pd(&printer);
    pd.setMinMax(1, doc->pages());
    if(pd.exec() == XDialog::Accepted)
    {
      render.render(doc, &printer);
    }
    else
      return;

    int page_num = 1;
    while(page_num < doc->pages())
    {
      page_num++;

      XSqlQuery qq;
      qq.prepare("INSERT INTO checkhead"
                 "      (checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, checkhead_printed,"
                 "       checkhead_checkdate, checkhead_number,"
                 "       checkhead_amount, checkhead_void,"
                 "       checkhead_misc,"
                 "       checkhead_for, checkhead_notes,"
                 "       checkhead_curr_id, checkhead_deleted) "
                 "SELECT checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, true,"
                 "       checkhead_checkdate, fetchNextCheckNumber(checkhead_bankaccnt_id),"
                 "       checkhead_amount, true, true,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       checkhead_curr_id, true"
                 "  FROM checkhead"
                 " WHERE(checkhead_id=:checkhead_id);");
      qq.bindValue(":checkhead_id", _check->id());
      if(!qq.exec())
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Check Information but will continue anyway\n %1")
                             .arg(qq.lastError().databaseText()),
                             qq, __FILE__, __LINE__);
      }
    }

    omfgThis->sChecksUpdated(_bankaccnt->id(), _check->id(), true);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Check Information"),
                                printPrint, __FILE__, __LINE__))
  {
    return;
  }
  else // join failed
  {
    QMessageBox::critical(this, tr("Cannot Print Check"),
                          tr("<p>The selected Check cannot be printed as the "
			     "Bank Account that it is to draw upon does not "
			     "have a valid Check Format assigned to it. "
			     "Please assign a valid Check Format to this Bank "
			     "Account before attempting to print this Check."));
    return;
  }

  if ( QMessageBox::question( this, tr("Check Printed"),
                             tr("Was the selected Check printed successfully?"),
			     QMessageBox::Yes,
			     QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    markCheckAsPrinted(_check->id());
  else if ( QMessageBox::question(this, tr("Mark Check as Voided"),
                                  tr("<p>Would you like to mark the selected "
				     "Check as Void and create a replacement "
				     "check?"),
				   QMessageBox::Yes,
				   QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    printPrint.prepare("SELECT voidCheck(:checkhead_id) AS result;");
    printPrint.bindValue(":checkhead_id", _check->id());
    printPrint.exec();
    if (printPrint.first())
    {
      int result = printPrint.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Voiding Check"),
                               storedProcErrorLookup("voidCheck", result),
                               __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Voiding Check"),
                                  printPrint, __FILE__, __LINE__))
    {
      return;
    }

    printPrint.prepare( "SELECT checkhead_bankaccnt_id,"
	       "       replaceVoidedCheck(checkhead_id) AS result "
               "FROM checkhead "
               "WHERE (checkhead_id=:checkhead_id);" );
    printPrint.bindValue(":checkhead_id", _check->id());
    printPrint.exec();
    if (printPrint.first())
    {
      int result = printPrint.value("result").toInt();
      if (result < 0)
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                               storedProcErrorLookup("replaceVoidedCheck", result),
                               __FILE__, __LINE__);
        return;
      }
      omfgThis->sChecksUpdated(printPrint.value("checkhead_bankaccnt_id").toInt(),
			       _check->id(), true);

      sHandleBankAccount(_bankaccnt->id());
      _print->setFocus();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                  printPrint, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void printCheck::sHandleBankAccount(int pBankaccntid)
{
  XSqlQuery printHandleBankAccount;
  if (_bankaccnt->id() != -1)
  {
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT bankaccnt_nextchknum, bankaccnt_ach_enabled "
                         "FROM bankaccnt "
                         "WHERE (bankaccnt_id=:bankaccnt_id);" );
    checkNumber.bindValue(":bankaccnt_id", _bankaccnt->id());
    checkNumber.exec();
    if (checkNumber.first())
    {
      _setCheckNumber = checkNumber.value("bankaccnt_nextchknum").toInt();
      _createEFT->setEnabled(checkNumber.value("bankaccnt_ach_enabled").toBool());
      _nextCheckNum->setText(checkNumber.value("bankaccnt_nextchknum").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Bank Information"),
                                  printHandleBankAccount, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    _setCheckNumber = -1;
    _nextCheckNum->clear();
  }

  printHandleBankAccount.prepare( "SELECT checkhead_id,"
	     "       (CASE WHEN(checkhead_number=-1) THEN TEXT('Unspecified')"
             "             ELSE TEXT(checkhead_number)"
             "        END || '-' || checkrecip_name) "
             "FROM checkhead LEFT OUTER JOIN "
	     "      checkrecip ON ((checkhead_recip_id=checkrecip_id)"
	     "                AND  (checkhead_recip_type=checkrecip_type))"
             "WHERE ((NOT checkhead_void)"
             "  AND  (NOT checkhead_printed)"
             "  AND  (NOT checkhead_posted)"
             "  AND  (checkhead_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY checkhead_number;" );
  printHandleBankAccount.bindValue(":bankaccnt_id", pBankaccntid);
  printHandleBankAccount.exec();
  _check->populate(printHandleBankAccount);
  _check->setNull();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Bank Information"),
                                printHandleBankAccount, __FILE__, __LINE__))
  {
    return;
  }
}

void printCheck::populate(int pcheckid)
{
  XSqlQuery printpopulate;
  printpopulate.prepare( "SELECT checkhead_bankaccnt_id "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  printpopulate.bindValue(":checkhead_id", pcheckid);
  printpopulate.exec();
  if (printpopulate.first())
  {
    _bankaccnt->setId(printpopulate.value("checkhead_bankaccnt_id").toInt());
    _check->setId(pcheckid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Check Information"),
                                printpopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void printCheck::sCreateEFT()
{
  XSqlQuery printCreateEFT;
  XSqlQuery releasenum;
  releasenum.prepare("SELECT releaseNumber('ACHBatch', :batch);");

  QString batch;

  MetaSQLQuery mql("SELECT *"
                   "  FROM <? literal(\"func\") ?>(<? value(\"bank\") ?>,"
                   "           <? value(\"check\") ?>, <? value(\"key\") ?>);");
  ParameterList params;
  params.append("func", _metrics->value("EFTFunction"));
  params.append("bank", _bankaccnt->id());
  params.append("check",_check->id());
  params.append("key",  omfgThis->_key);
  printCreateEFT = mql.toQuery(params);
  if (printCreateEFT.first())
  {
    batch = printCreateEFT.value("achline_batch").toString();
    releasenum.bindValue(":batch", batch);
    if (eftFileDir.isEmpty())
    {
      eftFileDir = xtsettingsValue("ACHOutputDirectory").toString();
    }
    QString suffixes = "*.ach *.aba *.dat *.txt";
    if (! suffixes.contains(_metrics->value("ACHDefaultSuffix")))
      suffixes = "*" + _metrics->value("ACHDefaultSuffix") + " " + suffixes;
    QString filename = QFileDialog::getSaveFileName(this, tr("EFT Output File"),
                            printCheck::eftFileDir + QDir::separator() +
                            "eft" + batch + _metrics->value("ACHDefaultSuffix"),
                            "(" + suffixes + ")");
    if (filename.isEmpty())
    {
      releasenum.exec();
      return;
    }
    QFileInfo fileinfo(filename);
    eftFileDir = fileinfo.absolutePath();
    QFile eftfile(filename);
    if (! eftfile.open(QIODevice::WriteOnly))
    {
      releasenum.exec();
      QMessageBox::critical(this, tr("Could Not Open File"),
                            tr("Could not open %1 for writing EFT data.")
                            .arg(filename));
      return;
    }
    do
    {
      eftfile.write(printCreateEFT.value("achline_value").toString().toLatin1());
      eftfile.write("\n");
    } while (printCreateEFT.next());
    eftfile.close();
    if (printCreateEFT.lastError().type() != QSqlError::NoError)
    {
      releasenum.exec();
      eftfile.remove();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating EFT File"),
                           printCreateEFT, __FILE__, __LINE__);
      return;
    }

    if (confirmAchOK::askOK(this, eftfile))
      markCheckAsPrinted(_check->id());
    else
    {
      releasenum.exec();
      XSqlQuery clearq;
      clearq.prepare("UPDATE checkhead "
                     "SET checkhead_printed=false,"
                     "    checkhead_ach_batch=NULL "
                     "WHERE (checkhead_id=:checkhead_id);");
      clearq.bindValue(":checkhead_id", _check->id());
      clearq.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Receiving ACH Confirmation"),
                                    clearq, __FILE__, __LINE__))
      {
        return;
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retreiving Check Information for EFT Creation"),
                                printCreateEFT, __FILE__, __LINE__))
  {
    return;
  }

}

void printCheck::sEnableCreateEFT()
{
  XSqlQuery printEnableCreateEFT;
  if (_bankaccnt->isValid() && _check->isValid() &&
      _metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled"))
  {
    printEnableCreateEFT.prepare("SELECT bankaccnt_ach_enabled AND vend_ach_enabled AS eftenabled "
              "FROM bankaccnt"
              "      JOIN checkhead ON (checkhead_bankaccnt_id=bankaccnt_id)"
              "      JOIN vendinfo ON (checkhead_recip_id=vend_id"
              "                    AND checkhead_recip_type='V') "
              "WHERE ((bankaccnt_id=:bankaccnt_id)"
              "  AND  (checkhead_id=:checkhead_id));");
    printEnableCreateEFT.bindValue(":bankaccnt_id", _bankaccnt->id());
    printEnableCreateEFT.bindValue(":checkhead_id", _check->id());
    printEnableCreateEFT.exec();
    if (printEnableCreateEFT.first())
      _createEFT->setEnabled(printEnableCreateEFT.value("eftenabled").toBool());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Information for EFT Creation"),
                                  printEnableCreateEFT, __FILE__, __LINE__))
    {
      return;
    }
    else // not found
      _createEFT->setEnabled(false);
  }
  else
    _createEFT->setEnabled(false);
}


void printCheck::storeEftFileDir()
{
  if (_metrics->boolean("ACHSupported") && _metrics->boolean("ACHEnabled") && !eftFileDir.isEmpty())
  {
    xtsettingsSetValue("ACHOutputDirectory", eftFileDir);
  }
}

void printCheck::done(int /*p*/)
{
  storeEftFileDir();
  close();
}

void printCheck::markCheckAsPrinted(const int pcheckid)
{
  XSqlQuery markq;
  markq.prepare("SELECT checkhead_bankaccnt_id,"
                "       markCheckAsPrinted(checkhead_id) AS result "
                "FROM checkhead "
                "WHERE (checkhead_id=:checkhead_id);" );
  markq.bindValue(":checkhead_id", pcheckid);
  markq.exec();
  if (markq.first())
  {
    int result = markq.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Marking Check as Printed"),
                             storedProcErrorLookup("markCheckAsPrinted", markq.value("result").toInt()),
                             __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(markq.value("checkhead_bankaccnt_id").toInt(),
                             pcheckid, true);

    if (_captive)
      close();
    else
    {
      sHandleBankAccount(_bankaccnt->id());
      _close->setText(tr("&Close"));
    }
  }
}
