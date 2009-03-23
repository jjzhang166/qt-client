/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QVBoxLayout>
#include <QUrl>
#include <QDesktopServices>

#include <metasql.h>
#include <QMessageBox>

#include "xsqlquery.h"

#include "contactcluster.h"

void ContactCluster::init()
{
    _ignoreSignals = false;

    _titleSingular = tr("Contact");
    _titlePlural = tr("Contacts");
    _query = "SELECT cntct.*, crmacct_name "
	     "FROM cntct LEFT OUTER JOIN crmacct ON (cntct_crmacct_id = crmacct_id) ";

    _layoutDone = false;
    _minimalLayout = false;
    _searchAcctId = -1;

    _grid->removeWidget(_label);	// will be reinserted
    _grid->removeWidget(_description);
    _grid->removeWidget(_list);		// will be reinserted
    _grid->removeWidget(_info);		// will be reinserted
    delete _description;

    _grid->setMargin(0);
    _grid->setSpacing(2);

    _nameBox		= new QHBoxLayout;
    _nameBox->setSpacing(2);
    _initialsBox	= new QHBoxLayout;
    _initialsBox->setSpacing(2);
    _titleBox		= new QHBoxLayout;
    _titleBox->setSpacing(2);

    _change             = new XLineEdit(this);
    _numberLit		= new QLabel(tr("Number:"), this, "_numberLit");
    _number		= new XLineEdit(this, "_number");
    
    _change->hide();
    _number->hide();
    _numberLit->hide();
    
    _nameLit		= new QLabel(tr("Name:"), this, "_nameLit");
    _honorific		= new XComboBox(this, "_honorific");
    _first		= new XLineEdit(this, "_first");
    _middle		= new XLineEdit(this, "_middle");
    _middle->setFixedWidth(20);
    _last		= new XLineEdit(this, "_last");
    _suffix		= new XLineEdit(this, "_suffix");
    _suffix->setFixedWidth(30);
    _initialsLit	= new QLabel(tr("Initials:"), this, "_initialsLit");
    _initials		= new XLineEdit(this, "_initials");
    _initials->setFixedWidth(_initials->size().width() / 3);
    _titleLit		= new QLabel(tr("Job Title:"), this, "_titleLit");
    _title		= new XLineEdit(this, "_title");
    
    _mapper		= new XDataWidgetMapper(this);

    //_nameBox->addWidget(_nameLit,	0);
    _nameBox->addWidget(_honorific,	0);
    _nameBox->addWidget(_first,		1);
    _nameBox->addWidget(_middle,	0);
    _nameBox->addWidget(_last,		2);
    _nameBox->addWidget(_suffix,	0);
    _nameBox->addWidget(_list,		0, Qt::AlignRight);
    _nameBox->addWidget(_info,		0, Qt::AlignRight);
    
    //_initialsBox->addWidget(_initialsLit, 0);
    _initialsBox->addWidget(_initials,	  0);
    _initialsBox->addStretch(0);

    //_titleBox->addWidget(_titleLit,	0);
    _titleBox->addWidget(_title,	2);

    _buttonBox 		= new QGridLayout;
    _crmAcct		= new CRMAcctCluster(this, "_crmAcct");
    _active		= new QCheckBox(tr("Active"), this, "_active");
    _owner              = new UsernameCluster(this, "_owner");

    _buttonBox->addWidget(_active,	0, 0, Qt::AlignTop);
    _buttonBox->addWidget(_crmAcct,	0, 1, Qt::AlignTop);
    _buttonBox->addWidget(_owner, 	0, 2, Qt::AlignTop);


    _phoneLit		= new QLabel(tr("Voice:"), this, "_phoneLit");
    _phone		= new XLineEdit(this, "_phone");
    _phone2Lit		= new QLabel(tr("Alternate:"), this, "_phone2Lit");
    _phone2		= new XLineEdit(this, "_phone2");
    _faxLit		= new QLabel(tr("Fax:"), this, "_faxLit");
    _fax		= new XLineEdit(this, "_fax");
    _emailLit		= new QLabel(tr("E-Mail:"), this, "_emailLit");
    _email		= new XLineEdit(this, "_email");
    _webaddrLit		= new QLabel(tr("Web:"), this, "_webaddrLit");
    _webaddr		= new XLineEdit(this, "_webaddr");
    _address		= new AddressCluster(this, "_address");

#if defined Q_OS_MAC
    _phone->setMinimumWidth(140);
#endif    

    QPalette p = _email->palette();
    p.setColor(QColorGroup::Text, Qt::blue);
    _email->setPalette(p);
    _webaddr->setPalette(p);
    
    QFont newFont = _email->font();
    newFont.setUnderline(TRUE);
    _email->setFont(newFont);
    _webaddr->setFont(newFont);

    _numberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _nameLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _titleLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _phoneLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _phone2Lit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _faxLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _emailLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _initialsLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _webaddrLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _honorific->setEditable(true);
    _honorific->setType(XComboBox::Honorifics);

    _crmAcct->setLabel(tr("CRM Account:"));
    _owner->setLabel(tr("Owner:"));

    layout();

    connect(_list,	SIGNAL(clicked()),	this, SLOT(sEllipses()));
    connect(_info,	SIGNAL(clicked()),	this, SLOT(sInfo()));

    connect(_honorific,	SIGNAL(newID(int)),		     this, SIGNAL(changed()));
    connect(_first,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_middle,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_last,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_suffix,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_initials,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_crmAcct,	SIGNAL(newId(int)),		     this, SIGNAL(changed()));
    connect(_title,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_phone,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_phone2,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_fax,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_email,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_webaddr,	SIGNAL(textChanged(const QString&)), this, SIGNAL(changed()));
    connect(_address,	SIGNAL(changed()),                   this, SIGNAL(changed()));
    connect(this,       SIGNAL(changed()),                   this, SLOT(setChanged()));

    connect(_honorific,	SIGNAL(newID(int)),  this, SLOT(sCheck()));
    connect(_first,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_middle,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_last,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_suffix,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_initials,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_crmAcct,	SIGNAL(newId(int)),  this, SLOT(sCheck()));
    connect(_title,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_phone,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_phone2,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_fax,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_email,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_webaddr,	SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_address,	SIGNAL(changed()),   this, SLOT(sCheck()));
    
    connect(_email,     SIGNAL(doubleClicked()), this, SLOT(sLaunchEmail()));
    connect(_webaddr,   SIGNAL(doubleClicked()), this, SLOT(sLaunchWebaddr()));
    
    connect(_first, SIGNAL(requestSearch()),	this, SLOT(sSearch()));
    connect(_middle, SIGNAL(requestSearch()),	this, SLOT(sSearch()));
    connect(_last, SIGNAL(requestSearch()),	this, SLOT(sSearch()));
    connect(_suffix, SIGNAL(requestSearch()),	this, SLOT(sSearch()));
    connect(_first, SIGNAL(requestList()),	this, SLOT(sList()));
    connect(_middle, SIGNAL(requestList()),	this, SLOT(sList()));
    connect(_last, SIGNAL(requestList()),	this, SLOT(sList()));
    connect(_suffix, SIGNAL(requestList()),	this, SLOT(sList()));
    
    connect(_first, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));
    connect(_last, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));

    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(_honorific);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setLabel("");
    _limits = 0;
    setInfoVisible(false);	// TODO - remove this and implement Info button
    silentSetId(-1);
    setOwnerVisible(false);
}

ContactCluster::ContactCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    init();
}

void ContactCluster::findDuplicates()
{
  if (_first->text().isEmpty() && _last->text().isEmpty())
    return;
    
  QString msg;
  XSqlQuery r;
  
  r.prepare(  "SELECT cntct_id, COALESCE(cntct_crmacct_id,0) AS cntct_crmacct_id "
              "FROM cntct "
              "WHERE ( ( cntct_first_name ~~* :first) "
              " AND (cntct_last_name ~~* :last) );");
  r.bindValue(":first", _first->text());
  r.bindValue(":last", _last->text());
  r.bindValue(":crmacct_id", _searchAcctId);
  r.exec();
  if (r.size() == 1)
  { 
    r.first();
    msg = tr("A contact exists with the same first and last name");
    if (_searchAcctId > 0 && r.value("cntct_crmacct_id").toInt() == 0)
      msg += tr(" not associated with any CRM Account");
    else if (_searchAcctId == r.value("cntct_crmacct_id").toInt())
      msg += tr(" on the current CRM Account");
    else if (_searchAcctId > 0)
      msg += tr(" associated with another CRM Account");
    msg += tr(". Would you like to use the existing contact?");
    
    if (QMessageBox::question(this, tr("Existing Contact"), msg,
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      r.first();
      setId(r.value("cntct_id").toInt());
    }
  }
  else if (r.size() > 1)
  {
    if (_searchAcctId > 0)
    {
      int cnt = 0;
      int cntctid = 0;
      while (r.next())
      {
        if (r.value("cntct_crmacct_id").toInt() == _searchAcctId)
        {
          cnt += 1;
          cntctid = r.value("cntct_id").toInt();
        }
      }
      if (cnt == 1)
      {
         msg = tr("A contact exists with the same first and last name "
                  "on the current CRM Account. "
                  "Would you like to use the existing contact?");
        if (QMessageBox::question(this, tr("Existing Contact"), msg,
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
          setId(cntctid);
        return;
      }
      if (cnt == 0)
        _searchAcctId = -1;
    }

    QString msg = tr("Multple contacts exist with the same first and last name");
    msg += tr(". Would you like to view all the existing contacts?");
    
    if (QMessageBox::question(this, tr("Existing Contacts"), msg,
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      ContactSearch* newdlg = new ContactSearch(this);
      if (newdlg)
      {
        newdlg->_searchFirst->setChecked(true);
        newdlg->_searchLast->setChecked(true);
        newdlg->_search->setText(_first->text() + " " + _last->text());
        newdlg->sFillList();
	      int id = newdlg->exec();
	      setId(id);
      }
      else
	       QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
			      .arg(__FILE__)
				    .arg(__LINE__),
			    tr("Could not instantiate a Search Dialog"));
    }
  }
  else if (r.lastError().type() != QSqlError::None)
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                        .arg(__FILE__)
                                        .arg(__LINE__),
                                r.lastError().databaseText());

}

void ContactCluster::setChanged()
{
  _changed=true;
}

void ContactCluster::setId(const int pId)
{
    if (pId == _id)
	return;
    else
    {
	silentSetId(pId);
	emit newId(pId);
    }
}

void ContactCluster::silentSetId(const int pId)
{
  disconnect(_first, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));
  disconnect(_last, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));
  if (pId == -1 || pId == 0)
  {
    _id = -1;
    _valid = false;
    connect(_first, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));
    connect(_last, SIGNAL(lostFocus()), this, SLOT(findDuplicates()));
    clear();
  }
  else if (pId == _id)
    return;
  else
  {   
      XSqlQuery idQ;
      idQ.prepare(_query + " WHERE cntct_id = :id;");
      idQ.bindValue(":id", pId);
      idQ.exec();
      if (idQ.first())
      {
          _ignoreSignals = true;

          _id = pId;
          _valid = true;
          _number->setText(idQ.value("cntct_number").toString());
          _honorific->setEditText(idQ.value("cntct_honorific").toString());
          _first->setText(idQ.value("cntct_first_name").toString());
          _middle->setText(idQ.value("cntct_middle").toString());
          _last->setText(idQ.value("cntct_last_name").toString());
          _suffix->setText(idQ.value("cntct_suffix").toString());
          _initials->setText(idQ.value("cntct_initials").toString());
          _crmAcct->setId(idQ.value("cntct_crmacct_id").toInt());
          _title->setText(idQ.value("cntct_title").toString());
          _phone->setText(idQ.value("cntct_phone").toString());
          _phone2->setText(idQ.value("cntct_phone2").toString());
          _fax->setText(idQ.value("cntct_fax").toString());
          _email->setText(idQ.value("cntct_email").toString());
          _webaddr->setText(idQ.value("cntct_webaddr").toString());
          _address->setId(idQ.value("cntct_addr_id").toInt());
          _active->setChecked(idQ.value("cntct_active").toBool());
          _notes = idQ.value("cntct_notes").toString();
          _owner->setUsername(idQ.value("cntct_owner_username").toString());

          if (_mapper->model())
          { 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_number)),       _number->text()); 	               _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_number)),       _number->text());
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_honorific)),    _honorific->currentText()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_first)),        _first->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_last)),         _last->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_initials)),     _initials->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)),          _number->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_title)),        _title->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_phone)),        _phone->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_phone2)),       _phone2->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_fax)),          _fax->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_email)),        _email->text()); 	 
            _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_webaddr)),      _webaddr->text()); 	 
           }

          _ignoreSignals = false;
      }
      else if (idQ.lastError().type() != QSqlError::None)
          QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                        .arg(__FILE__)
                                        .arg(__LINE__),
                                idQ.lastError().databaseText());
    _valid = true;
  }

  _changed=false;
  // _parsed = TRUE;
}

void ContactCluster::setNumber(QString p)
{
  if (p == _number->text())
    return;

  _number->setText(p);
  XSqlQuery q;
  q.prepare("SELECT cntct_id "
              "FROM cntct "
              "WHERE (cntct_number=:number);");
  q.bindValue(":number", p);
  q.exec();
  if (q.first())
  {
    if (q.value("cntct_id").toInt() != _id)
      silentSetId(q.value("cntct_id").toInt());
  }
  else
    silentSetId(-1);  
}

void ContactCluster::clear()
{
  _id = -1;
  _valid = false;

  _honorific->clearEditText();
  _first->clear();
  _middle->clear();
  _last->clear();
  _suffix->clear();
  _initials->clear();
  _crmAcct->setId(-1);
  _title->clear();
  _phone->clear();
  _phone2->clear();
  _fax->clear();
  _email->clear();
  _webaddr->clear();
  _address->clear();
  _active->setChecked(true);
  _notes = "";
}

QString ContactCluster::name() const
{
  QString name;
  
  if (!_first->text().isEmpty())
    name = _first->text();
    
  if (!_middle->text().isEmpty())
  {
    if (!name.isEmpty())
      name += " ";
    name += _middle->text();
  }
  
  if (!_last->text().isEmpty())
  {
    if (!name.isEmpty())
      name += " ";
    name += _last->text();
  }

  return name;
}

void ContactCluster::setDataWidgetMap(XDataWidgetMapper* m)
{  
  m->addMapping(this          ,  _fieldNameNumber,        "number", "defaultText");
  m->addMapping(_change      ,  _fieldNameChange,         "text",   "defaultText");
  m->addMapping(_active      ,  _fieldNameActive);
  m->addMapping(_first       ,  _fieldNameFirst,          "text",   "defaultText");
  m->addMapping(_middle      ,  _fieldNameMiddle,         "text",   "defaultText");
  m->addMapping(_last        ,  _fieldNameLast,           "text",   "defaultText");
  m->addMapping(_suffix      ,  _fieldNameSuffix,         "text",   "defaultText");  
  m->addMapping(_initials    ,  _fieldNameInitials,       "text",   "defaultText");
  m->addMapping(_title       ,  _fieldNameTitle,          "text",   "defaultText");
  m->addMapping(_phone       ,  _fieldNamePhone,          "text",   "defaultText");
  m->addMapping(_phone2      ,  _fieldNamePhone2,         "text",   "defaultText");
  m->addMapping(_fax         ,  _fieldNameFax,            "text",   "defaultText");
  m->addMapping(_email       ,  _fieldNameEmailAddress,   "text",   "defaultText");
  m->addMapping(_webaddr     ,  _fieldNameWebAddress,     "text",   "defaultText");
  _honorific->setFieldName(_fieldNameHonorific);
  _honorific->setDataWidgetMap(m);
  _crmAcct->setFieldName(_fieldNameCrmAccount);
  _crmAcct->setDataWidgetMap(m);
  _address->setFieldNameAddrChange(_fieldNameAddrChange);
  _address->setFieldNameNumber(_fieldNameAddrNumber);
  _address->setFieldNameLine1(_fieldNameLine1);
  _address->setFieldNameLine2(_fieldNameLine2);
  _address->setFieldNameLine3(_fieldNameLine3);
  _address->setFieldNameCity(_fieldNameCity);
  _address->setFieldNameState(_fieldNameState);
  _address->setFieldNamePostalCode(_fieldNamePostalCode);
  _address->setFieldNameCountry(_fieldNameCountry);
  _address->setDataWidgetMap(m);
  _mapper=m;
}


void ContactCluster::setName(const QString& p)
{
  int firstSpace = p.indexOf(" ");
  if (firstSpace > 0)
  {
    setFirst(p.left(firstSpace));
    setLast(p.right(p.length() - firstSpace - 1));
  }
  else
    setLast(p);
}

/*
  returns  -1 if error
  	   -2 if address is shared
	  -10 if unclear if intent is to overwrite or create new contact
 */
int ContactCluster::save(AddressCluster::SaveFlags flag)
{
  int addrSave = _address->save(flag);
  
  if (addrSave < 0)
    return addrSave;

  XSqlQuery datamodQ;
  datamodQ.prepare("SELECT COALESCE(saveCntct(:cntct_id,:cntct_number,:crmacct_id,:addr_id,"
		   ":honorific,:first,:middle,:last,:suffix,:initials,"
		   ":active,:phone,:phone2,:fax,:email,:webaddr,"
		   ":notes,:title,:flag,:owner),0) AS result;");
  datamodQ.bindValue(":cntct_number", _number->text());
  datamodQ.bindValue(":cntct_id",  id());
  datamodQ.bindValue(":honorific", _honorific->currentText());
  if (_address->id() > 0)
    datamodQ.bindValue(":addr_id", _address->id());
  datamodQ.bindValue(":first",	   _first->text());
  datamodQ.bindValue(":middle",	   _middle->text());
  datamodQ.bindValue(":last",	   _last->text());
  datamodQ.bindValue(":suffix",	   _suffix->text());
  datamodQ.bindValue(":initials",  _initials->text());
  datamodQ.bindValue(":title",	   _title->text());
  datamodQ.bindValue(":phone",	   _phone->text());
  datamodQ.bindValue(":phone2",	   _phone2->text());
  datamodQ.bindValue(":fax",	   _fax->text());
  datamodQ.bindValue(":email",	   _email->text());
  datamodQ.bindValue(":webaddr",   _webaddr->text());
  datamodQ.bindValue(":notes",	   _notes);
  datamodQ.bindValue(":owner",     _owner->username());
  if (flag == AddressCluster::CHECK)
    datamodQ.bindValue(":flag", QString("CHECK"));
  else if (flag == AddressCluster::CHANGEALL)
    datamodQ.bindValue(":flag", QString("CHANGEALL"));
  else if (flag == AddressCluster::CHANGEONE)
    datamodQ.bindValue(":flag", QString("CHANGEONE"));
  else
    return -1;
  if (_crmAcct->id() > 0)
    datamodQ.bindValue(":crmacct_id",_crmAcct->id());	// else NULL
  datamodQ.bindValue(":active",    QVariant(_active->isChecked()));
  datamodQ.exec();
  if (datamodQ.first())
  {
    if (datamodQ.value("result").toInt() == 0)
      return 0;
    else if (datamodQ.value("result").toInt() > 0)
      if (_mapper->model())
      {
        _id=datamodQ.value("result").toInt();
        _changed=false;
      }
      else
        silentSetId(datamodQ.value("result").toInt());
    else if (datamodQ.value("result").toInt() == -10)
      return -10;
  }
  else
    return -1;
  return id();
}

void ContactCluster::setAccount(const int p)
{
  if (_crmAcct->id() != p)
  {
    _crmAcct->setId(p);
    _changed = true;
  }
}

void ContactCluster::setOwnerVisible(const bool vis)
{
  _owner->setVisible(vis);
  layout();
}

void ContactCluster::setOwnerEnabled(const bool vis)
{
  _owner->setEnabled(vis);
}

void ContactCluster::setNumberVisible(const bool vis)
{
  _number->setVisible(vis);
  _numberLit->setVisible(vis);
  layout();
}

void ContactCluster::setAddressVisible(const bool vis)
{
  _address->setVisible(vis);
  layout();
}

void ContactCluster::setAccountVisible(const bool p)
{
  _crmAcct->setVisible(p);
  layout();
}

void ContactCluster::setActiveVisible(const bool p)
{
  _active->setVisible(p);
  layout();
}

void ContactCluster::setEmailVisible(const bool p)
{
  _emailLit->setVisible(p);
  _email->setVisible(p);
  // layout();
}

void ContactCluster::setInitialsVisible(const bool p)
{
  _initialsLit->setVisible(p);
  _initials->setVisible(p);
  layout();
}

void ContactCluster::setPhonesVisible(const bool p)
{
  _phoneLit->setVisible(p);
  _phone->setVisible(p);
  _phone2Lit->setVisible(p);
  _phone2->setVisible(p);
  _faxLit->setVisible(p);
  _fax->setVisible(p);
  // layout();
}

void ContactCluster::setWebaddrVisible(const bool p)
{
  _webaddrLit->setVisible(p);
  _webaddr->setVisible(p);
  // layout();
}

void ContactCluster::setMinimalLayout(const bool p)
{

  // always visible
  _label->setVisible(true);
  _nameLit->setVisible(true);
  _honorific->setVisible(true);
  _first->setVisible(true);
  _middle->setVisible(true);
  _last->setVisible(true);
  _title->setVisible(true);
  _phoneLit->setVisible(true);
  _phone->setVisible(true);
  _faxLit->setVisible(true);
  _fax->setVisible(true);
  _emailLit->setVisible(true);
  _email->setVisible(true);

  // at the mercy of minimal layout
  _initialsLit->setVisible(! p);
  _initials->setVisible(! p);
  _crmAcct->setVisible(! p);
  _active->setVisible(! p);
  _phone2Lit->setVisible(! p);
  _phone2->setVisible(! p);
  _webaddrLit->setVisible(! p);
  _webaddr->setVisible(! p);
  _address->setVisible(! p);

  _minimalLayout = p;
  layout();
}

void ContactCluster::layout()
{
  QLayout* pwl = 0;
  if(parentWidget())
    pwl = parentWidget()->layout();
  QLayout* currLayout = (_layoutDone) ? _grid : pwl;

  if (_layoutDone)
  {
    currLayout->removeItem(_nameBox);
    currLayout->removeItem(_initialsBox);
    currLayout->removeItem(_buttonBox);
    currLayout->removeItem(_titleBox);
  }

  if (currLayout)
  {
    currLayout->removeWidget(_label);
    currLayout->removeItem(_nameBox);
    currLayout->removeItem(_buttonBox);
    currLayout->removeItem(_titleBox);
    currLayout->removeWidget(_phoneLit);
    currLayout->removeWidget(_phone);
    currLayout->removeWidget(_phone2Lit);
    currLayout->removeWidget(_phone2);
    currLayout->removeWidget(_faxLit);
    currLayout->removeWidget(_fax);
    currLayout->removeWidget(_emailLit);
    currLayout->removeWidget(_email);
    currLayout->removeWidget(_webaddrLit);
    currLayout->removeWidget(_webaddr);
    currLayout->removeWidget(_address);
  }

  _grid->addWidget(_label,	0, 0, 1, -1);
  //_grid->addItem(_numberBox,    1, 0, 1, -1);  Possible implement at a later time
  _grid->addWidget(_nameLit,    1, 0, 1, 1);
  _grid->addItem(_nameBox,	1, 1, 1, -1);
  _grid->addWidget(_initialsLit,2, 0, 1, 1);
  _grid->addItem(_initialsBox,  2, 1, 1, -1);
  _grid->addWidget(_active,       2, 2, 1, -1);
  _grid->addItem(_buttonBox,	3, 0, 1, -1);
  _grid->addWidget(_titleLit,   4, 0, 1, 1);
  _grid->addItem(_titleBox,	4, 1, 1, -1);

  if (_minimalLayout)
  {
    _grid->setColumnStretch(0, 0);
    _grid->setColumnStretch(1, 1);
    _grid->setColumnStretch(2, 0);
    _grid->setColumnStretch(3, 1);
    _grid->setColumnStretch(4, 0);
    _grid->setColumnStretch(5, 2);

    _grid->addWidget(_phoneLit,	5, 0);
    _grid->addWidget(_phone,	5, 1);
    _grid->addWidget(_faxLit,	5, 2);
    _grid->addWidget(_fax,	5, 3);
    _grid->addWidget(_emailLit,	5, 4);
    _grid->addWidget(_email,	5, 5);

    _grid->addWidget(_phone2Lit,	5, 0);
    _grid->addWidget(_phone2,		5, 1);
    _grid->addWidget(_webaddrLit,	5, 4);
    _grid->addWidget(_webaddr,		6, 0);
    _grid->addWidget(_address,		6, 1); //, -1, -1);
  }
  else if (_address->isVisibleTo(this))
  {
    _grid->setColumnStretch(0, 0);
    _grid->setColumnStretch(1, 1);
    _grid->setColumnStretch(2, 3);

    _grid->addWidget(_phoneLit,		5, 0);
    _grid->addWidget(_phone,		5, 1);
    _grid->addWidget(_address,		5, 2, 5, -1);
    _grid->addWidget(_phone2Lit,	6, 0);
    _grid->addWidget(_phone2,		6, 1);
    _grid->addWidget(_faxLit,		7, 0);
    _grid->addWidget(_fax,		7, 1);
    _grid->addWidget(_emailLit,		8, 0);
    _grid->addWidget(_email,		8, 1);
    _grid->addWidget(_webaddrLit,	9, 0);
    _grid->addWidget(_webaddr,		9, 1);
  }
  else
  {
    _grid->setColumnStretch(0, 0);
    _grid->setColumnStretch(1, 1);
    _grid->setColumnStretch(2, 0);
    _grid->setColumnStretch(3, 2);
    _grid->setColumnStretch(4, 0);
    _grid->setColumnStretch(5, 2);

    _grid->addWidget(_phoneLit,		6, 0);
    _grid->addWidget(_phone,		6, 1);
    _grid->addWidget(_faxLit,		6, 2);
    _grid->addWidget(_fax,		6, 3);
    _grid->addWidget(_webaddrLit,	6, 4);
    _grid->addWidget(_webaddr,		5, 5);
    _grid->addWidget(_phone2Lit,	7, 0);
    _grid->addWidget(_phone2,		7, 1);
    _grid->addWidget(_emailLit,		7, 2);
    _grid->addWidget(_email,		7, 3);
    _grid->addWidget(_address,		8, 0, 1, -1);
  }

  _layoutDone = true;
}

void ContactCluster::setSearchAcct(const int p)
{
  _searchAcctId = p;
}

void ContactCluster::check()
{
  QString oldNumber = _number->text();
  XSqlQuery tx;
  tx.exec("BEGIN;");
  int result=save();
  tx.exec("ROLLBACK;");
  _number->setText(oldNumber);
  if (result == -2)
  {
    int answer = 2;	// Cancel
    answer = QMessageBox::question(this, tr("Question Saving Address"),
                tr("There are multiple Contacts sharing this Address.\n"
                   "What would you like to do?"),
                tr("Change This One"),
                tr("Change Address for All"),
                0, 0);
     if (answer==0)
     {
       _address->setAddrChange(QString("CHANGEONE"));
       _change->setText("CHANGEONE");
     }
     else if (answer==1)
     {
       _address->setAddrChange(QString("CHANGEALL"));  
       _change->setText("CHANGEALL");
     }
  }
  else if (result == -10)
  {
    int answer;
    answer = QMessageBox::question(this,
                tr("Question Saving %1").arg(label()),
                tr("<p>Would you like to update the existing Contact or "
                   "create a new one?"),
                tr("Create New"),
                tr("Change Existing"),
                0, 0);
    if (0 == answer)
      _change->setText("CHANGEONE");
    else if (1 == answer)
      _change->setText("CHANGEALL");

    // Make sure the mapper is aware of this change
    if (_mapper && _mapper->model() &&
        (_mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),
                                                        _mapper->mappedSection(_change))).toString() != _change->text()))
        _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),
                                                          _mapper->mappedSection(_change)),
                                  _change->text());
  }
  else if (result < 0)
  {
    QMessageBox::critical(this, tr("Error"),tr("There was an error checking this Contact (%1).").arg(result));
    return;
  }
}

void ContactCluster::sCheck()
{
  if(_ignoreSignals)
    return;

  if ((! _honorific->isVisibleTo(this) || _honorific->currentText().simplified().isEmpty()) &&
      (! _first->isVisibleTo(this)   || _first->text().simplified().isEmpty()) &&
      (! _middle->isVisibleTo(this)   || _middle->text().simplified().isEmpty()) &&
      (! _last->isVisibleTo(this)    || _last->text().simplified().isEmpty()) &&
      (! _suffix->isVisibleTo(this)   || _suffix->text().simplified().isEmpty()) &&
      (! _initials->isVisibleTo(this) || _initials->text().simplified().isEmpty()) &&
      (! _crmAcct->isVisibleTo(this) || _crmAcct->id() <= 0) &&
      (! _title->isVisibleTo(this)   || _title->text().simplified().isEmpty()) &&
      (! _phone->isVisibleTo(this)   || _phone->text().simplified().isEmpty()) &&
      (! _phone2->isVisibleTo(this)  || _phone2->text().simplified().isEmpty()) &&
      (! _fax->isVisibleTo(this)     || _fax->text().simplified().isEmpty()) &&
      (! _email->isVisibleTo(this)   || _email->text().simplified().isEmpty()) &&
      (! _webaddr->isVisibleTo(this) || _webaddr->text().simplified().isEmpty()) &&
      (! _address->isVisibleTo(this) || _address->id() <= 0))
    setId(-1);
}

void ContactCluster::sEllipses()
{
    if (_x_preferences && _x_preferences->value("DefaultEllipsesAction") == "search")
	sSearch();
    else
	sList();
}

void ContactCluster::sInfo()
{
    /*
    ContactInfo* newdlg = new ContactInfo(this, "ContactInfo", true);
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
    */
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("%1::sInfo() not yet defined").arg(className()));
}

void ContactCluster::sList()
{
    ContactList* newdlg = new ContactList(this);
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("Could not instantiate a List Dialog"));
}

void ContactCluster::sSearch()
{
    ContactSearch* newdlg = new ContactSearch(this);
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("Could not instantiate a Search Dialog"));
}

void ContactCluster::setChange(QString p)
{
  _change->setText(p);

  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_change))).toString() != _change->text())
    _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(_change)), _change->text());
}

void ContactCluster::sLaunchEmail()
{
  QDesktopServices::openUrl(QUrl("mailto:" + _email->text()));
}

void ContactCluster::sLaunchWebaddr()
{
  QDesktopServices::openUrl(QUrl("http://" + _webaddr->text()));
}

///////////////////////////////////////////////////////////////////////////////

ContactList::ContactList(QWidget* pParent, const char* pName, bool, Qt::WFlags) 
  : VirtualList(pParent, 0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    _parent = (ContactCluster*)(pParent);
    _id = _parent->_id;
    if (!pName)
	setObjectName("ContactList");
    else
        setObjectName(pName);
    setWindowTitle(_parent->_titlePlural);
    
    _searchAcct = new QCheckBox();
    _searchAcct->setMaximumWidth(480);

    _listTab->setColumnCount(0);

    _listTab->addColumn(tr("First Name"),   80, Qt::AlignLeft, true, "cntct_first_name");
    _listTab->addColumn(tr("Last Name"),    -1, Qt::AlignLeft, true, "cntct_last_name");
    _listTab->addColumn(tr("CRM Account"), 100, Qt::AlignLeft, true, "crmacct_name");
    
    if (_parent->_searchAcctId > 0)
    {
      XSqlQuery crmacct;
      crmacct.prepare("SELECT crmacct_number || ' - ' || crmacct_name AS account "
                      "FROM crmacct "
                      "WHERE (crmacct_id = :crmacct_id);");
      crmacct.bindValue(":crmacct_id", _parent->_searchAcctId);
      crmacct.exec();
      if (crmacct.first())
      {
        _searchAcct->setChecked(true);
        _dialogLyt->addWidget(_searchAcct);
        _searchAcct->setText(tr("Only contacts for %1").arg(crmacct.value("account").toString()));
        connect(_searchAcct, SIGNAL(toggled(bool)), this, SLOT(sFillList(bool)));
      }
    }
    else
      _searchAcct->hide();

    resize(500, size().height());
    
    sFillList(_searchAcct->isChecked());
}

void ContactList::sFillList(const bool searchAcct)
{
  if (searchAcct)
    _parent->_extraClause = " WHERE (cntct_crmacct_id=:searchAcctId) ";
  else
    _parent->_extraClause = "";

  _listTab->clear();
  XSqlQuery query;

  query.prepare(_parent->_query +
		_parent->_extraClause +
		" ORDER BY cntct_last_name, cntct_first_name;");
  query.bindValue(":crmacct_id", _parent->_crmAcct->id());
  query.bindValue(":searchAcctId", _parent->_searchAcctId);
  query.exec();
  query.first();
  if (query.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				  .arg(__FILE__)
				  .arg(__LINE__),
			  query.lastError().databaseText());
    return;
  }
  else if (query.size() < 1)	// no rows found with limit so try without
  {
    query.prepare(_parent->_query +
		  " ORDER BY cntct_last_name, cntct_first_name;");
    query.exec();
    query.first();
    if (query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
			    query.lastError().databaseText());
      return;
    }
  }

  _listTab->populate(query);
}

/* do this differently than VirtualList::sSearch(QString&):
   look for characters that match the beginning of first or last name,
   not just the first field
*/
void ContactList::sSearch(const QString& pTarget)
{

  QTreeWidgetItem *target = 0;
  for (int i = 0; i < _listTab->topLevelItemCount(); i++)
  {
    target = _listTab->topLevelItem(i);
    if (target == NULL ||
        target->text(0).startsWith(pTarget.toUpper(), Qt::CaseInsensitive) ||
        target->text(1).startsWith(pTarget.toUpper(), Qt::CaseInsensitive) ||
        target->text(2).startsWith(pTarget.toUpper(), Qt::CaseInsensitive) ||
        target->text(3).startsWith(pTarget.toUpper(), Qt::CaseInsensitive))
      break;
  }

  if (target)
  {
    _listTab->setCurrentItem(target);
    _listTab->scrollToItem(target);
  }
}

///////////////////////////////////////////////////////////////////////////

ContactSearch::ContactSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    
    // remove the stuff we won't use
    disconnect(_searchNumber,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    disconnect(_searchName,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    disconnect(_searchDescrip,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));

    selectorsLyt->removeWidget(_searchNumber);
    selectorsLyt->removeWidget(_searchName);
    selectorsLyt->removeWidget(_searchNumber);

    delete _searchNumber;
    delete _searchName;
    delete _searchDescrip;

    _listTab->setColumnCount(0);
    
    _parent = (ContactCluster*)(pParent);

    _searchFirst	= new XCheckBox(tr("Search First Name"));
    _searchLast		= new XCheckBox(tr("Search Last Name"));
    _searchCRMAcct	= new XCheckBox(tr("Search CRM Account"));
    _searchTitle	= new XCheckBox(tr("Search Title"));
    _searchPhones	= new XCheckBox(tr("Search Phone Numbers"));
    _searchEmail	= new XCheckBox(tr("Search Email Address"));
    _searchWebAddr	= new XCheckBox(tr("Search Web Address"));
    _searchInactive	= new XCheckBox(tr("Show Inactive Contacts"));
    
    _searchFirst->setObjectName("_searchName");
    _searchLast->setObjectName("_searchLast");
    _searchCRMAcct->setObjectName("_searchCRMAcct");
    _searchTitle->setObjectName("_searchTitle");
    _searchPhones->setObjectName("_searchPhones");
    _searchEmail->setObjectName("_searchEmail");
    _searchWebAddr->setObjectName("_searchWebAddr");
    _searchInactive->setObjectName("_searchInactive");

    selectorsLyt->addWidget(_searchFirst,    0, 0);
    selectorsLyt->addWidget(_searchLast,     1, 0);
    selectorsLyt->addWidget(_searchCRMAcct,  2, 0);
    selectorsLyt->addWidget(_searchTitle,    3, 0);
    selectorsLyt->addWidget(_searchPhones,   0, 1);
    selectorsLyt->addWidget(_searchEmail,    1, 1);
    selectorsLyt->addWidget(_searchWebAddr,  2, 1);
    selectorsLyt->addWidget(_searchInactive, 3, 1);
    
    _searchAcct = new QCheckBox();
    _searchAcct->setMaximumWidth(480);
  
    if (_parent->_searchAcctId > 0)
    {
      XSqlQuery crmacct;
      crmacct.prepare("SELECT crmacct_number || ' - ' || crmacct_name AS account "
                      "FROM crmacct "
                      "WHERE (crmacct_id = :crmacct_id);");
      crmacct.bindValue(":crmacct_id", _parent->_searchAcctId);
      crmacct.exec();
      if (crmacct.first())
      {
        _searchAcct->setChecked(true);
        _dialogLyt->addWidget(_searchAcct);
        _searchAcct->setText(tr("Only contacts for %1").arg(crmacct.value("account").toString()));
        connect(_searchAcct, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
      }
    }
    else
      _searchAcct->hide();

    connect(_searchFirst,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchLast,     SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchCRMAcct,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchTitle,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchPhones,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchEmail,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchWebAddr,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

    _listTab->addColumn(tr("First Name"),     80, Qt::AlignLeft, true, "cntct_first_name");
    _listTab->addColumn(tr("Last Name"),      -1, Qt::AlignLeft, true, "cntct_last_name");
    _listTab->addColumn(tr("CRM Account"),   100, Qt::AlignLeft, true, "crmacct_name");
    _listTab->addColumn(tr("Title"),         100, Qt::AlignLeft, true, "cntct_honorific");
    _listTab->addColumn(tr("Phone"),	      75, Qt::AlignLeft, true, "cntct_phone");
    _listTab->addColumn(tr("Alt. Phone"),     75, Qt::AlignLeft, true, "cntct_phone2");
    _listTab->addColumn(tr("Fax"),	      75, Qt::AlignLeft, true, "cntct_fax");
    _listTab->addColumn(tr("Email Address"), 100, Qt::AlignLeft, true, "cntct_email");
    _listTab->addColumn(tr("Web Address"),   100, Qt::AlignLeft, true, "cntct_webaddr");

    resize(800, size().height());

    _parent = (ContactCluster*)(pParent);
    setObjectName("contactSearch");
    setWindowTitle(_parent->_titlePlural);
    _titleLit->setText(_parent->_titlePlural);
    _id = _parent->_id;
}

void ContactSearch::sFillList()
{
    if (_searchAcct->isChecked())
      _parent->_extraClause = " WHERE (cntct_crmacct_id=:searchAcctId) ";
    else
      _parent->_extraClause = "";
    
    _listTab->clear();
    if (_search->text().isEmpty() ||
	(!_searchFirst->isChecked()    && !_searchLast->isChecked() &&
	 !_searchCRMAcct->isChecked()  && !_searchTitle->isChecked() &&
	 !_searchPhones->isChecked()   &&
	 !_searchEmail->isChecked()    && !_searchWebAddr->isChecked() ))
      return;

    QString limits = 
      "<? if exists(\"extraClause\") ?> "
      "  AND "
      "<? else ?>"
      "  WHERE "
      "<? endif ?> "
      "<? if exists(\"searchInactive\") ?> "
      "   true "
      "<? else ?>"
      "   cntct_active "
      "<? endif ?>"
      "<? if reExists(\"search[FLCTPEW]\") ?> "
      "  AND ("
      "  <? if exists(\"searchFirst\") ?> "
      "     COALESCE(cntct_first_name,'') || ' ' "
      "  <? else ?>"
      "    '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchLast\") ?> "
      "     || COALESCE(cntct_last_name,'') || '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchCRMAcct\") ?> "
      "     || COALESCE(crmacct_name,'') || '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchTitle\") ?> "
      "     || COALESCE(cntct_title,'') || '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchPhones\") ?> "
      "    || COALESCE(cntct_phone,'') || '\n' "
      "    || COALESCE(cntct_phone2,'') || '\n' "
      "    || COALESCE(cntct_fax,'') || '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchEmail\") ?> "
      "     || COALESCE(cntct_email,'') || '\n' "
      "  <? endif ?>"
      "  <? if exists(\"searchWebAddr\") ?> "
      "     || COALESCE(cntct_webaddr,'') || '\n' "
      "  <? endif ?>"
      "  ~* <? value(\"searchText\") ?> )"
      "<? endif ?>"
      "ORDER BY cntct_last_name, cntct_first_name, crmacct_number;";
    QString sql = _parent->_query +
		  _parent->_extraClause.replace(":searchAcctId",
					QString::number(_parent->_searchAcctId)) +
		  limits;

    ParameterList params;
    if (_searchFirst->isChecked())
      params.append("searchFirst");
    if (_searchLast->isChecked())
      params.append("searchLast");
    if (_searchCRMAcct->isChecked())
      params.append("searchCRMAcct");
    if (_searchTitle->isChecked())
      params.append("searchTitle");
    if (_searchPhones->isChecked())
      params.append("searchPhones");
    if (_searchEmail->isChecked())
      params.append("searchEmail");
    if (_searchWebAddr->isChecked())
      params.append("searchWebAddr");
    if (_searchInactive->isChecked())
      params.append("searchInactive");
    if (! _parent->_extraClause.isEmpty())
      params.append("extraClause", _parent->_extraClause);

    params.append("searchText", _search->text());

    MetaSQLQuery mql(sql);
    XSqlQuery query = mql.toQuery(params);
    if (query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
			    query.lastError().databaseText());
      return;
    }
//    else if (query.size() < 1)	// no rows found with limit so try without
//    {
//      sql = _parent->_query + limits;
//      MetaSQLQuery mqlAllAccnts(sql);
//      query = mqlAllAccnts.toQuery(params);
//      if (query.lastError().type() != QSqlError::NoError)
//      {
//        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
//				      .arg(__FILE__)
//				      .arg(__LINE__),
//			      query.lastError().databaseText());
//        return;
//      }
//    }

    _listTab->populate(query);
}
