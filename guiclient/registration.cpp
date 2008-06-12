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

#include "registration.h"

#include <QMessageBox>
#include <QUrl>

#include "guiclient.h"

registration::registration(QWidget* parent, Qt::WindowFlags fl)
    : XDialog(parent, fl)
{
  setupUi(this);

  connect(_back,     SIGNAL(clicked()), this, SLOT(sBack()));
  connect(_cancel,   SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_next,     SIGNAL(clicked()), this, SLOT(sNext()));
  connect(_register, SIGNAL(clicked()), this, SLOT(sRegister()));

  _postreq = 0;
  _contact->setAccountVisible(false);
  _contact->setActiveVisible(false);
  _contact->setInitialsVisible(false);
  _contact->setMinimalLayout(true);

  if (_metrics->value("Registered") == "Yes")
      _weaskLit->setText(tr("<p><i>It seems you or someone else using this "
                            "same database have already registered with "
                            "xTuple.</i> You can fill out this window and send "
                            "us registration information again, perhaps to "
                            "register another user, or click the %1 button.")
                         .arg(_later->text()));
}

registration::~registration()
{
  if (_postreq)
  {
    while (_postreq->hasPendingRequests())
      qDebug("~registration() waiting for _postreq's last request");
    delete _postreq;
    _postreq = 0;
  }
}

void registration::languageChange()
{
  retranslateUi(this);
}

void registration::sBack()
{
  int prevIndex = _wizardStack->currentIndex() - 1;
  int validcount = _wizardStack->count() - 1;

  if (! _enduser->isChecked())
  {
    validcount--;
    if (prevIndex == 2)
      prevIndex--;
  }

  _wizardStack->setCurrentIndex(prevIndex);

  _back->setEnabled(_wizardStack->currentIndex() > 0);
  _next->setEnabled(_wizardStack->currentIndex() < validcount - 1);
  _register->setEnabled(! _next->isEnabled());
}

void registration::reject()
{
  if (_metrics->value("Registered") != "Yes")
    _metrics->set("Registered", QString("No"));
  XDialog::reject();
}

void registration::sNext()
{
  int nextIndex = _wizardStack->currentIndex() + 1;
  int validcount = _wizardStack->count() - 1;

  if (! _enduser->isChecked() && ! _var->isChecked())
  {
    validcount = validcount - 2;
    if (nextIndex > 1)
      nextIndex--;
  }
  else if (! _var->isChecked())
  {
    validcount--;
    if (nextIndex > 2)
      nextIndex--;
  }
  else if (! _enduser->isChecked())
  {
    validcount--;
    if (nextIndex == 2)
      nextIndex++;
  }

  _wizardStack->setCurrentIndex(nextIndex);

  _back->setEnabled(_wizardStack->currentIndex() > 0);
  _next->setEnabled(_wizardStack->currentIndex() < validcount - 1);
  _register->setEnabled(! _next->isEnabled());
}

QString registration::encodedPair(const QString pfield, const QString pvalue)
{
  QString encodedValue = pvalue;
  QUrl::encode(encodedValue);
  return pfield + "=" + encodedValue;
}

void registration::sRegister(bool /*pretry*/)
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { _company->text().isEmpty(),
        tr("Your Company name is required."),                     _company},
    { _contact->first().isEmpty() && _contact->last().isEmpty(),
        tr("Your Name is required."),                             _contact},
    { _contact->emailAddress().isEmpty(),
        tr("Your E-Mail address is required."),                   _contact},
    { _username->text().isEmpty(),
        tr("Your desired Username is required."),                 _username},
    { _password->text().isEmpty(),
        tr("Your desired Password is required."),                 _password},
    { _password->text() != _password2->text(),
        tr("The two versions of your Password do not match."),    _password},
    { true, QString(), 0 }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  error[errIndex].msg);
    _wizardStack->setCurrentIndex(0);
    error[errIndex].widget->setFocus();
    return;
  }

  int companygroupcnt = (_companyUrl->text().isEmpty()        ? 0 : 1) + 
                        (_companyAddr->line1().isEmpty()      ? 0 : 1) +
                        (_companyAddr->city().isEmpty()       ? 0 : 1) +
                        (_companyAddr->state().isEmpty()      ? 0 : 1) +
                        (_companyAddr->postalCode().isEmpty() ? 0 : 1) +
                        (_companyAddr->country().isEmpty()    ? 0 : 1) +
                        (_revenues->currentIndex() == 0       ? 0 : 1) +
                        (_noemployees->currentIndex() == 0    ? 0 : 1) +
                        (_sic->currentIndex() == 0            ? 0 : 1);

  if (companygroupcnt > 0 && companygroupcnt < 9 && // almost done!
      QMessageBox::question(this, tr("Do you want to tell us more?"),
                            tr("You have submitted some of the data required "
                               "for access to extended documentation and "
                               "videos. Would you like to tell us a little "
                               "more about your company to get this access?"),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::Yes)
  {
    if (_companyUrl->text().isEmpty())              _companyUrl->setFocus();
    else if (_companyAddr->line1().isEmpty())       _companyAddr->setFocus();
    else if (_companyAddr->city().isEmpty())        _companyAddr->setFocus();
    else if (_companyAddr->state().isEmpty())       _companyAddr->setFocus();
    else if (_companyAddr->postalCode().isEmpty())  _companyAddr->setFocus();
    else if (_companyAddr->country().isEmpty())     _companyAddr->setFocus();
    else if (_revenues->currentIndex()  == 0)       _revenues->setFocus();
    else if (_noemployees->currentIndex()  == 0)    _noemployees->setFocus();
    else if (_sic->currentIndex()  == 0)            _sic->setFocus();

    _wizardStack->setCurrentIndex(1);

    return;
  }

  if (_enduser->isChecked() && _gotvaryes->isChecked() &&
      _varName->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Need Integrator/Consultant Name"),
                         tr("You've told us that you have a systems "
                            "integrator or consultant but haven't given us "
                            "his/her name. Please do so."));
    _wizardStack->setCurrentIndex(2);
    _varName->setFocus();
    return;
  }

  if (_var->isChecked())
  {
    if (_partnerlevel->currentIndex() == 0)
    {
      QMessageBox::warning(this, tr("Partner Level?"),
                           tr("Please tell us what level of partnership you "
                              "would like to pursue with xTuple."));
      _wizardStack->setCurrentIndex(3);
      _partnerlevel->setFocus();
      return;
    }

    if (! _whaterpquick->isChecked() &&
        ! _whaterpms->isChecked() &&
        ! _whaterpsage->isChecked() &&
        ! _whaterpinfor->isChecked() &&
        ! _whaterporacle->isChecked() &&
        ! _whaterpsap->isChecked() &&
        (! _whaterpotherpropLit->isChecked() ||
         (_whaterpotherpropLit->isChecked() && _whaterpotherprop->text().isEmpty())) &&
        (! _whaterpotherossLit->isChecked() ||
         (_whaterpotherossLit->isChecked() && _whaterpotheross->text().isEmpty())) &&
        ! _whatotherGroup->isChecked() &&
        ! _whatotherErp->isChecked() &&
        ! _whatotherConsulting->isChecked() &&
        ! _whatotherSoftware->isChecked() &&
        ! _whatotherAccounting->isChecked() &&
        ! _whatotherSupport->isChecked() &&
        ! _whatotherHardware->isChecked() &&
        ! _whatotherNetwork->isChecked() &&
        (! _whatotherOtherLit->isChecked() ||
          (_whatotherOtherLit->isChecked() && _whatotherOther->text().isEmpty())) )
    {
      QMessageBox::warning(this, tr("What do you sell or implement?"),
                           tr("Please tell us what products and services you "
                              "sell and what ERP or accounting software you "
                              "currently implement."));
      _wizardStack->setCurrentIndex(3);
      _whaterpquick->setFocus();
      return;
    }
  }

  _wizardStack->setCurrentIndex(4);

  _progressLit->setText(tr("Building data to send"));
  QString request;
  request += "&" + encodedPair("company",    _company->text());
  request += "&" + encodedPair("honorific",  _contact->honorific());
  request += "&" + encodedPair("first_name", _contact->first());
  request += "&" + encodedPair("last_name",  _contact->last());
  request += "&" + encodedPair("email",      _contact->emailAddress());
  request += "&" + encodedPair("member_name",_username->text());
  request += "&" + encodedPair("jobtitle",   _contact->title());
  request += "&" + encodedPair("telephone",  _contact->phone());
  request += "&" + encodedPair("fax",        _contact->fax());
  request += "&" + encodedPair("password",   _password->text());
  request += "&" + encodedPair("addr_1",     _companyAddr->line1());
  request += "&" + encodedPair("addr_2",     _companyAddr->line2());
  request += "&" + encodedPair("addr_3",     _companyAddr->line3());
  request += "&" + encodedPair("city",       _companyAddr->city());
  request += "&" + encodedPair("state",      _companyAddr->state());
  request += "&" + encodedPair("country",    _companyAddr->country());
  request += "&" + encodedPair("zip",        _companyAddr->postalCode());

  _progress->setValue(1);

  request += "&" + encodedPair("revenues",     _revenues->currentText());
  request += "&" + encodedPair("num_employees",_noemployees->currentText());
  request += "&" + encodedPair("sic_code",     _sic->currentText());
  request += "&" + encodedPair("cur_mansoft",  _erp->text());
  request += "&" + encodedPair("cur_accsoft",  _accounting->text());
  request += "&" + encodedPair("bigchall",     _bigchallenge->currentText());
  request += "&" + encodedPair("pref_client",  _client->currentText());
  request += "&" + encodedPair("pref_server",  _server->currentText());
  request += "&" + encodedPair("pref_db",      _db->currentText());

  request += "&" + encodedPair("have_integrator",      
                      (_gotvaryes->isChecked() ? "Yes" : "No"));

  request += "&" + encodedPair("var_name",      _varName->text());
  request += "&" + encodedPair("erp_timeframe", _timeframe->currentText());
  request += "&" + encodedPair("erp_budget",    _budget->currentText());
  request += "&" + encodedPair("provider_type", _partnerlevel->currentText());

  _progress->setValue(2);

  QStringList whaterp;
  if (_whaterpquick->isChecked())	whaterp.append("Quickbooks");
  if (_whaterpms->isChecked())	        whaterp.append("Microsoft");
  if (_whaterpsage->isChecked())	whaterp.append("Sage");
  if (_whaterpinfor->isChecked())	whaterp.append("Infor");
  if (_whaterporacle->isChecked())	whaterp.append("Oracle");
  if (_whaterpsap->isChecked())	        whaterp.append("SAP");
  if (! _whaterpotherprop->text().isEmpty())
    whaterp.append(_whaterpotherprop->text());
  if (! _whaterpotheross->text().isEmpty())
    whaterp.append(_whaterpotheross->text());
  request += "&" + encodedPair("cur_erp_imp", whaterp.join(", "));

  QStringList whatother;
  if (_whatotherErp->isChecked())        whatother.append("ERP");
  if (_whatotherConsulting->isChecked()) whatother.append("Consulting");
  if (_whatotherSoftware->isChecked())   whatother.append("Software");
  if (_whatotherAccounting->isChecked()) whatother.append("Accounting");
  if (_whatotherSupport->isChecked())    whatother.append("Support");
  if (_whatotherHardware->isChecked())   whatother.append("Hardware");
  if (_whatotherNetwork->isChecked())    whatother.append("Network");
  if (! _whatotherOther->text().isEmpty())
    whatother.append(_whatotherOther->text());
  request += "&" + encodedPair("other_products", whatother.join(", "));

  request += "&" + encodedPair("other_info",    _otherInfo->text());
  request += "&" + encodedPair("website",       _companyUrl->text());
  request += "&" + encodedPair("manpro",(_enduser->isChecked() ? "Yes" : "No"));
  request += "&" + encodedPair("solpro",(_var->isChecked() ? "Yes" : "No"));

  _progress->setValue(3);

  QHttpRequestHeader header("POST", "/pbregister.php");
  header.setValue("Host", "www.xtuple.com");
  header.setContentType("application/x-www-form-urlencoded");

  _postreq = new QHttp("www.xtuple.com", QHttp::ConnectionModeHttp);
  _postreq->setHost("www.xtuple.com");
  _postreq->request(header, request.utf8()); 

  _xml->setText(request);
  sState(QHttp::Unconnected);

  _register->setEnabled(false);
  _next->setEnabled(false);
  _back->setEnabled(false);
  _later->setEnabled(false);
  _cancel->setEnabled(true);

  connect(_postreq,SIGNAL(done(bool)),               this,SLOT(sDone(bool)));
  connect(_postreq,SIGNAL(dataReadProgress(int,int)),this,SLOT(sRead(int,int)));
  connect(_postreq,SIGNAL(dataSendProgress(int,int)),this,SLOT(sSent(int,int)));
  connect(_postreq,SIGNAL(stateChanged(int)),        this,SLOT(sState(int)));

  _progress->setValue(4);
}

void registration::sRead(int pread, int ptotal)
{
  _progress->setValue(50 + 40 * pread / ptotal);
}

void registration::sSent(int psent, int ptotal)
{
  _progress->setValue(10 + 40 * psent / ptotal);
}

void registration::sState(int pstate)
{
  //qDebug("registration::sState(%d)", pstate);

  QString msg;
  int     progress = 0;

  switch (pstate)
  {
    case QHttp::Unconnected : msg = tr("Unconnected");               break;
    case QHttp::Connecting  : msg = tr("Connecting"); progress =  5; break;
    case QHttp::Sending     : msg = tr("Sending");    progress = 10; break;
    case QHttp::Reading     : msg = tr("Reading");    progress = 50; break;
    case QHttp::Connected   : msg = tr("Connected");                 break;
    case QHttp::Closing     : msg = tr("Closing");    progress = 90; break;
  }

  //qDebug("msg: %s\tprogress: %d", msg.toAscii().data(), progress);

  if (! msg.isEmpty())
    _progressLit->setText(msg);

  if (progress > 0)
    _progress->setValue(progress);
}

void registration::sDone(bool perror)
{
  disconnect(_postreq,      SIGNAL(done(bool)),     this, SLOT(sDone(bool)));

  QString result = QString(_postreq->readAll());

  if (perror)
  {
    if (QMessageBox::question(this, tr("Problem Registering"),
                              tr("<p>There was a problem sending your "
                                 "registration information to xTuple. Would "
                                 "you like to try again?<p>If you answer No "
                                 "and want to register later, you will have to"
                                 " re-enter all of the data again."),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes)
    {
      sRegister(true);
      return;
    }
    else
    {
      reject();
      return;
    }
  }

  _progress->setValue(100);
  _progressLit->setText(tr("Done!"));
  // TODO: check if the registration failed and handle it here
  _metrics->set("Registered", QString("Yes"));
  QMessageBox::information(this, tr("Thanks for registering!"),
                           tr("<p>The registration server responded with:<p>%1")
                           .arg(result));
  accept();
}

void registration::sCancel()
{
  if (QMessageBox::question(this, tr("Cancel Registration?"),
                            tr("<p>Are you sure you want to cancel this "
                               "registration attempt?<p>If you answer Yes "
                               "then you will have to "
                               "re-enter all of the data again."),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    if (_postreq)
    {
      disconnect(_postreq, SIGNAL(done(bool)), this, SLOT(sDone(bool)));
      connect(_postreq, SIGNAL(done(bool)), this, SLOT(reject()));
      _postreq->abort();
      XDialog::reject();
    }
  }
}
