/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reverseGLSeries.h"

#include <QVariant>
#include <QMessageBox>

reverseGLSeries::reverseGLSeries(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _glseries = -1;
}

reverseGLSeries::~reverseGLSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void reverseGLSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reverseGLSeries::set( const ParameterList & pParams )
{
  XSqlQuery reverseet;
  XDialog::set(pParams);
  QVariant param;
  bool valid = false;
  
  param = pParams.value("glseries", &valid);
  if(valid)
  {
    _glseries = param.toInt();
    
    reverseet.prepare("SELECT gltrans_journalnumber AS journalnumber, gltrans_date AS transdate "
              "FROM gltrans "
              "WHERE (gltrans_sequence=:glseries) "
              "UNION "
              "SELECT sltrans_journalnumber AS journalnumber, sltrans_date AS transdate "
              "FROM sltrans "
              "WHERE (sltrans_sequence=:glseries); " );
    reverseet.bindValue(":glseries", _glseries);
    reverseet.exec();
    if(reverseet.first())
    {
      _notes->setText(tr("Reversal for Journal #") +
                      reverseet.value("journalnumber").toString());
      _journalNum->setText(reverseet.value("journalnumber").toString());
      _distDate->setDate(reverseet.value("transdate").toDate());
    }
    else
    {
      systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
      return UndefinedError;
    }
  }
  
  return NoError;
}

void reverseGLSeries::sPost()
{
  XSqlQuery reversePost;
  if(!_distDate->isValid())
  {
    QMessageBox::warning(this, tr("Cannot Reverse Series"),
                         tr("A valid distribution date must be entered before the G/L Series can be reversed.") );
    _distDate->setFocus();
    return;
  }

  if(_metrics->boolean("MandatoryGLEntryNotes") && _notes->toPlainText().trimmed().isEmpty())
  {
    QMessageBox::information( this, tr("Cannot Post G/L Series"),
                                    tr("You must enter some Notes to describe this transaction.") );
    _notes->setFocus();
    return;
  }

  
  reversePost.prepare("SELECT reverseGLSeries(:glseries, :distdate, :notes) AS result;");
  reversePost.bindValue(":glseries", _glseries);
  reversePost.bindValue(":distdate", _distDate->date());
  reversePost.bindValue(":notes", _notes->toPlainText());
  reversePost.exec();
  if(reversePost.first())
  {
    int result = reversePost.value("result").toInt();
    if(result < 0)
    {
      switch(result)
      {
      case -1:
      default:
        QMessageBox::warning( this, tr("Error Reversing G/L Series"),
                              tr("An Unknown Error was encountered while reversing the G/L Series.") );
      }
      return;
    }
    else
      QMessageBox::information( this, tr("Reversed G/L Series"),
                                tr("Reversing Journal #%1 was sucessfully created.").arg(reversePost.value("result").toString()) );
  }
  else
    systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
  
  accept();
}

