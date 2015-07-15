/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "projectCopy.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVariant>
#include <QSqlError>
#include <QMessageBox>

#include <parameter.h>
#include <xsqlquery.h>
#include "datecluster.h"

projectCopy::projectCopy(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl) :
  QDialog(parent, fl)
{
  setObjectName(name ? name : "projectCopy");
  setModal(modal);

  setWindowTitle(tr( "Copy Project"));

  _newProjectId = -1;

  if ( !name )
   setObjectName( "projectCopy" );

  QHBoxLayout *_mainLayout = new QHBoxLayout(this);
  QGridLayout *_grid = new QGridLayout();
  QHBoxLayout *_numberLayout = new QHBoxLayout(0);
  QHBoxLayout *_newnumberLayout = new QHBoxLayout(0);
  QVBoxLayout *_buttonsLayout= new QVBoxLayout(0);

  _number    	= new XLineEdit(this);
  _newnumber    	= new XLineEdit(this);
  _name    	= new XLineEdit(this);
  _newname    	= new XLineEdit(this);
  _due    	= new DLineEdit(this);

  QLabel *_numberLit = new QLabel(tr("Number:"), this);
  QLabel *_nameLit = new QLabel(tr("Name:"), this);
  QLabel *_newnumberLit = new QLabel(tr("New Number:"), this);
  QLabel *_newnameLit = new QLabel(tr("New Name:"), this);
  QLabel *_dueLit = new QLabel(tr("Due Date:"), this);

  _number->setEnabled(false);
  _name->setEnabled(false);

  _numberLayout->addWidget(_number);
  QSpacerItem* spacer = new QSpacerItem( 200, 20, QSizePolicy::Minimum );
  _numberLayout->addItem( spacer );

  _newnumberLayout->addWidget(_newnumber);
  QSpacerItem* spacer1 = new QSpacerItem( 200, 20, QSizePolicy::Minimum );
  _newnumberLayout->addItem( spacer1 );

  _grid->setMargin(0);
  _grid->setSpacing(2);
  _grid->addWidget(_numberLit,		0, 0);
  _grid->addLayout(_numberLayout,		0, 1, 1, -1, Qt::AlignRight);
  _grid->addWidget(_nameLit,		1, 0);
  _grid->addWidget(_name,		1, 1);
  _grid->addWidget(_newnumberLit,		2, 0);
  _grid->addLayout(_newnumberLayout,	2, 1, 1, -1, Qt::AlignRight);
  _grid->addWidget(_newnameLit,	3, 0);
  _grid->addWidget(_newname,		3, 1);
  _grid->addWidget(_dueLit,		4, 0);
  _grid->addWidget(_due,		4, 1);

  _close = new QPushButton(tr("&Cancel"), this);
  _buttonsLayout->addWidget(_close);

  _copy = new QPushButton(tr("&Copy"), this);
  _copy->setEnabled( false );
  _copy->setAutoDefault( true );
  _copy->setDefault( true );
  _buttonsLayout->addWidget(_copy);
  QSpacerItem* spacer2 = new QSpacerItem( 20, 100, QSizePolicy::Minimum );
  _buttonsLayout->addItem( spacer2 );

  _mainLayout->addLayout(_grid);
  _mainLayout->addLayout(_buttonsLayout);

  connect( _copy, SIGNAL( clicked() ), this, SLOT( sCopy() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );
  connect(_newnumber, 	SIGNAL(textChanged(const QString&)), this, SLOT( sHandleButtons() ) );
  connect(_newname, 	SIGNAL(textChanged(const QString&)), this, SLOT( sHandleButtons() ) );
  connect(_due, 	SIGNAL(newDate(const QDate &) ), this, SLOT( sHandleButtons() ) );

}

void projectCopy::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
    _projectId = param.toInt();
  else
    reject();

  XSqlQuery prj;
  prj.prepare("SELECT * FROM prj WHERE prj_id=:prj_id;");
  prj.bindValue(":prj_id", _projectId);
  prj.exec();
  if (prj.first())
  {
    _number->setText(prj.value("prj_number"));
    _name->setText(prj.value("prj_name"));
  }
}

void projectCopy::sCopy()
{
  XSqlQuery copyproject;
  copyproject.prepare("SELECT copyproject(:prj_id, :new_number, :new_desc, :due); ");
  copyproject.bindValue(":prj_id", _projectId);
  copyproject.bindValue(":new_number", _newnumber->text());
  copyproject.bindValue(":new_desc", _newname->text());
  copyproject.bindValue(":due", _due->date());
  copyproject.exec();
  if (copyproject.first())
  {
    _newProjectId = copyproject.value("copyproject").toInt();
    if (_newProjectId == -2)
    {
      QMessageBox::critical(this, "System Error", tr("The Project Number already exists.  Please choose another number."));
      reject();
    }
  }
  else if (copyproject.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, "System Error", tr("A system error occurred when copying this Project"));
    reject();
  }

  done(_newProjectId);
}

void projectCopy::sHandleButtons()
{
  _copy->setEnabled(!_newnumber->text().isEmpty() && !_newname->text().isEmpty() && _due->isValid());
}

