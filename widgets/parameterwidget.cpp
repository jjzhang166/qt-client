/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <parameter.h>
#include <xsqlquery.h>

#include <QtScript>
#include <QMessageBox>
#include <QTableWidget>

#ifdef Q_WS_MAC
#include <QFont>
#endif

#include "parameterwidget.h"
#include "widgets.h"
#include "xcombobox.h"
#include "usernamecluster.h"
#include "datecluster.h"
#include "crmacctcluster.h"
#include "filterManager.h"
#include "contactcluster.h"
#include "filtersave.h"

#define DEBUG false

/* TODO: - add Flag to the set of types to handle parameters that are either
           passed or not (this is different from boolean parameters, that can
           be either true or false).
         - add XComboBoxCode to the set of types to handle parameters that
           require the code value from an xcombobox instead of the id.
         - store the text value of the enum in the filter definition, not the
           integer. otherwise the db contents will prevent reorganizing the
           order of the symbolic values of the type enumeration.
         - rename methods to align better with qt naming conventions
         - fix indentation - ts=8 sw=2 tabexpand=true
         - find a solution to the problem that multiselect's use of
           qtablewidget violates the one-unit-high rule
 */

ParameterWidget::ParameterWidget(QWidget *pParent, const char *pName)  :
    QWidget(pParent)
{
  setupUi(this);


  if(pName)
    setObjectName(pName);

  _initialized = false;
  _shared = false;
  setSavedFilters(-1);
  _filterSignalMapper = new QSignalMapper(this);
  _saveButton->setDisabled(true);

	_filterGroup->setVisible(false);
	_filterButton->setChecked(false);

  connect(_addFilterRow, SIGNAL(clicked()), this, SLOT( addParam() ) );
  connect(_filterButton, SIGNAL(clicked()), this, SLOT( setFiltersVisabiltyPreference() ) );
  connect(_filterSignalMapper, SIGNAL(mapped(int)), this, SLOT( removeParam(int) ));
  connect(_saveButton, SIGNAL(clicked()), this, SLOT( save() ) );
  connect(_manageButton, SIGNAL(clicked()), this, SLOT( sManageFilters() ) );
  connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( applySaved(int) ) );
  connect(_filterList, SIGNAL(currentIndexChanged(int)), this, SLOT( setFiltersDefault() ) );
  connect(this, SIGNAL(updated()), this, SLOT( toggleSave() ) );
}

void ParameterWidget::showEvent(QShowEvent * event)
{
	
  if(_initialized)
    return;

  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + objectName();

  if(_x_preferences)
  {
    if (_x_preferences->value(_settingsName + "/checked") == "f")
    {
      _filterGroup->setVisible(false);
      _filterButton->setChecked(false);
    }
    else
    {
      _filterGroup->setVisible(true);
      _filterButton->setChecked(true);
    }
  }
  _initialized = true;
  QWidget::showEvent(event);
	
}

void ParameterWidget::appendValue(ParameterList &pParams)
{
  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();
    if (pParams.inList(tempPair.first))
    {
      pParams.remove(tempPair.first);
    }

    pParams.append(tempPair.first, tempPair.second);
  }
}


void ParameterWidget::applyDefaultFilterSet()
{
  XSqlQuery qry;
  QString filter_name;
  int filter_id;
	QString pname;

 

  //hides parameterwidget when it's embedded within another widget with a parent
  if (this->parent() && this->parent()->parent())
  {
    clearFilters();
    this->hide();

    return;
  }

	if(window())
    pname = window()->objectName() + "/";
	_settingsName2 = pname + this->objectName();

	if(_x_preferences)
  {
		_settingsName2 += "/filter_default";
		filter_id = _x_preferences->value(_settingsName2).toInt();
	}

  QString query = "SELECT filter_name "
                  "FROM filter "
                  "WHERE filter_id=:id ";
                  

  if (this->parent())
  {
    QString classname(parent()->objectName());
    if (classname.isEmpty())
      classname = parent()->metaObject()->className();

    qry.prepare(query);
		qry.bindValue(":id", filter_id);
    qry.exec();

    if (qry.first())
    {
      filter_name = qry.value("filter_name").toString();
      setSavedFiltersIndex(filter_name);
      applySaved(0, filter_id);
    }
    else
    {
      addParam();
			applySaved(0, 0);
    }

  }

}

void ParameterWidget::addParam()
{
  XComboBox *xcomboBox = new XComboBox(_filterGroup);
  QToolButton *toolButton = new QToolButton(_filterGroup);
  QLineEdit *lineEdit = new QLineEdit(_filterGroup);
  QGridLayout *gridLayout = new QGridLayout();
  QVBoxLayout *xcomboLayout = new QVBoxLayout();
  QHBoxLayout *widgetLayout1 = new QHBoxLayout();
  QVBoxLayout *widgetLayout2 = new QVBoxLayout();
  QVBoxLayout *buttonLayout = new QVBoxLayout();

  int nextRow = _filtersLayout->rowCount();
  QString currRow = QString().setNum(nextRow);

  // Set up objects
  gridLayout->setObjectName("topLayout" + currRow);

  xcomboLayout->setObjectName("xcomboLayout" + currRow);
  xcomboLayout->setContentsMargins(0, 0, 0, 0);

  xcomboBox->setObjectName("xcomboBox" + currRow);
  xcomboBox->addItem("", currRow + ":" + "2");

  widgetLayout1->setObjectName("widgetLayout1" + currRow);

  widgetLayout2->setObjectName("widgetLayout2" + currRow);
  widgetLayout2->setContentsMargins(0, 0, 0, 0);

  lineEdit->setObjectName("widget" + currRow);
  lineEdit->setDisabled(true);

  buttonLayout->setObjectName("buttonLayout" + currRow);
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  toolButton->setObjectName("button" + currRow);
  toolButton->setText(tr("-"));
 #ifdef Q_WS_MAC
  toolButton->setMinimumWidth(25);
 #else
  toolButton->setMinimumWidth(21);
 #endif

  //grab the items provided by other widgets to populate xcombobox with
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > i(_types);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, ParameterWidgetTypes> tempPair = i.value();
    QString value = QString().setNum(nextRow) + ":" + QString().setNum(tempPair.second);
    if ( _usedTypes.isEmpty() || !containsUsedType(i.key()) )
      xcomboBox->addItem(i.key(), value );
  }

  xcomboLayout->addWidget(xcomboBox);
  xcomboLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  // Place the default line edit/button combo
  widgetLayout1->addWidget(lineEdit);
  widgetLayout1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));

  widgetLayout2->addLayout(widgetLayout1);
  widgetLayout2->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(widgetLayout2, 0, 0, 1, 1);

  // Place Button
  buttonLayout->addWidget(toolButton);
  buttonLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));

  gridLayout->addLayout(buttonLayout, 0, 1, 1, 1);

  _filtersLayout->addLayout(gridLayout, nextRow, 1, 1, 1);
  _filtersLayout->addLayout(xcomboLayout, nextRow, 0, 1, 1);


  // Hook up connections
  connect(toolButton, SIGNAL(clicked()), _filterSignalMapper, SLOT(map()));
  connect(toolButton, SIGNAL(clicked()), gridLayout, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), xcomboBox, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
  connect(toolButton, SIGNAL(clicked()), toolButton, SLOT( deleteLater() ) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( addUsedType() ) );
  connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
	connect(xcomboBox, SIGNAL(currentIndexChanged(int)), this, SLOT( repopulateComboboxes() ) );
  connect(lineEdit, SIGNAL(editingFinished()), this, SLOT( storeFilterValue() ) );

  _filterSignalMapper->setMapping(toolButton, nextRow);

  _addFilterRow->setDisabled(true);

}

void ParameterWidget::applySaved(int pId, int filter_id)
{
  QWidget *found = 0;
  QDate tempdate;
  XSqlQuery qry;
  QString query;
  QString filterValue;
	QDate today = QDate::currentDate();
  int xid, init_filter_id;

	init_filter_id = filter_id;

  QMapIterator<int, QPair<QString, QVariant> > j(_filterValues);
	QPair<QString, ParameterWidgetTypes> tempPair;

  clearFilters();

  if (!parent())
    return;

  if (_filterList->id() == -1)
  {
    emit updated();
    return;
  }

  if (filter_id == 0 && _filterList->id() != -1)
    filter_id = _filterList->id(_filterList->currentIndex());

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  query = " SELECT filter_value, "
          "  CASE WHEN (filter_username IS NULL) THEN true "
          "  ELSE false END AS shared "
          " FROM filter "
          " WHERE filter_id=:id ";

  qry.prepare(query);
  qry.bindValue(":id", filter_id );

  qry.exec();

  if (qry.first())
  {
    filterValue = qry.value("filter_value").toString();
    _shared = qry.value("shared").toBool();
  }

	
  QStringList filterRows = filterValue.split("|");
  QString tempFilter = QString();

  int windowIdx = _filtersLayout->rowCount();

	if (filterRows.size() == 1  && pId == 0 && filter_id != 0)
	{
		emit updated();
		return;
	}

  for (int i = 0; i < filterRows.size(); ++i)
  {
    tempFilter = filterRows[i];
    if ( !(tempFilter.isEmpty()) )
    {
      //0 is filterType, 1 is filterValue, 2 is parameterwidgettype
      QStringList tempFilterList = tempFilter.split(":");
			QString key = this->getParameterTypeKey(tempFilterList[0]);
			if (key.isEmpty())
			{
				//parametertype is no longer found, prompt user to delete filter
				if (QMessageBox::question(this, tr("Invalid Filter Set"), tr("This filter set contains an obsolete filter and will be deleted. Do you want to do this?"),
					  QMessageBox::No | QMessageBox::Default,
            QMessageBox::Yes) == QMessageBox::No)
				return;
				else
				{
					QString query = "delete from filter where filter_id=:filter_id";
					XSqlQuery qry;

					qry.prepare(query);
					qry.bindValue(":filter_id", filter_id);
					qry.exec();

					setSavedFilters();
					return;
				}
			}
			else
			{
				this->addParam();

				QLayoutItem *test = _filtersLayout->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
				XComboBox *mybox = (XComboBox*)test->widget();

      
				int idx = mybox->findText(key);

				mybox->setCurrentIndex(idx);
	
		    found = getFilterWidget(windowIdx);

			  int widgetType = tempFilterList[2].toInt();

			  //grab pointer to newly created filter object
				switch (widgetType)
				{
					case Date:
						DLineEdit *dLineEdit;
						dLineEdit = qobject_cast<DLineEdit*>(found);
						if (dLineEdit != 0)
							dLineEdit->setDate(today.addDays(tempFilterList[1].toInt()), true);
						break;
					case User:
						UsernameCluster *usernameCluster;
							usernameCluster = qobject_cast<UsernameCluster*>(found);
						if (usernameCluster != 0)
							usernameCluster->setUsername(tempFilterList[1]);
						break;
					case Crmacct:
						CRMAcctCluster *crmacctCluster;
						crmacctCluster = qobject_cast<CRMAcctCluster*>(found);
						if (crmacctCluster != 0)
							crmacctCluster->setId(tempFilterList[1].toInt());
							break;
					case Contact:
						ContactCluster *contactCluster;
						contactCluster = qobject_cast<ContactCluster*>(found);
						if (contactCluster != 0)
							contactCluster->setId(tempFilterList[1].toInt());
						break;
					case XComBox:
						XComboBox *xBox;
						xBox = qobject_cast<XComboBox*>(found);
						if (xBox != 0)
						{
							//fix for setid not emitting id signal if id found for filter is first in list
							//set to any other valid id first to fix it
							xBox->setId(2);

							xid = tempFilterList[1].toInt();
							xBox->setId(xid);
						}
						break;
					case Multiselect:
					{
						QTableWidget *tab;
						tab = qobject_cast<QTableWidget*>(found);
						if (tab != 0)
						{
							QStringList   savedval = tempFilterList[1].split(",");
							bool oldblk = tab->blockSignals(true);
							/* the obvious, loop calling tab->selectRow(), gives one selected row,
							 so try this to get multiple selections:
							   make only the desired values selectable,
							   select everything, and
							   connect to a slot that can clean up after us.
							 yuck.
						*/
							for (int j = 0; j < tab->rowCount(); j++)
							{
								if (! savedval.contains(tab->item(j, 0)->data(Qt::UserRole).toString()))
									tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() & (~ Qt::ItemIsSelectable));
							}
							QTableWidgetSelectionRange range(0, 0, tab->rowCount() - 1,
							                               tab->columnCount() - 1);
							tab->setRangeSelected(range, true);
							connect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

							tab->blockSignals(oldblk);
							storeFilterValue(-1, tab);
						}
					}
					break;
					default:
					{
						QLineEdit *lineEdit;
						lineEdit = qobject_cast<QLineEdit*>(found);
						if (lineEdit != 0)
						{
							lineEdit->setText(tempFilterList[1]);
							storeFilterValue(-1, lineEdit);
						}
						}
								break;
				}//end of switch
				
			}//end of not empty key else
			windowIdx++;
		}//end of if tempfilter not empty
  }//end of for

  // TODO: the switch below is like the switch above. can we remove duplication?
	//This code is similar in concept, but the implementation is different. 
	if (filterRows.size() == 1  && pId == 0)
	{
			//apply filter defaults					
			QMapIterator<QString, QVariant> k(_defaultTypes);
			
			while(k.hasNext())
			{
				k.next();
				int windowIdx = _filtersLayout->rowCount();
				this->addParam();

				QLayoutItem *test = _filtersLayout->itemAtPosition(windowIdx, 0)->layout()->itemAt(0);
				XComboBox *mybox = (XComboBox*)test->widget();

				int idx = mybox->findText(k.key());

				mybox->setCurrentIndex(idx);
				
				found = getFilterWidget(windowIdx);

				tempPair = _types[k.key()];
				int widgetType = tempPair.second;
				
				switch (widgetType)
				{
					case Date:
						DLineEdit *dLineEdit;
						dLineEdit = qobject_cast<DLineEdit*>(found);
						if (dLineEdit != 0)
							dLineEdit->setDate(QDate::fromString(k.value().toString(), "yyyy-MM-dd"), true);
						break;
					case User:
						UsernameCluster *usernameCluster;
						usernameCluster = qobject_cast<UsernameCluster*>(found);
						if (usernameCluster != 0)
							usernameCluster->setUsername(k.value().toString());
						break;
					case Crmacct:
						CRMAcctCluster *crmacctCluster;
						crmacctCluster = qobject_cast<CRMAcctCluster*>(found);
						if (crmacctCluster != 0)
							crmacctCluster->setId(k.value().toInt());
						break;
					case Contact:
						ContactCluster *contactCluster;
						contactCluster = qobject_cast<ContactCluster*>(found);
						if (contactCluster != 0)
							contactCluster->setId(k.value().toInt());
						break;
					case XComBox:
						XComboBox *xBox;
						xBox = qobject_cast<XComboBox*>(found);
						if (xBox != 0)
						{
							//fix for setid not emitting id signal if id found for filter is first in list
							//set to any other valid id first to fix it
							xBox->setId(2);

							xid = k.value().toInt();
							xBox->setId(xid);
						}
						break;
          case Multiselect:
            {
              QTableWidget *tab;
							tab = qobject_cast<QTableWidget *>(found);
							if (tab != 0)
							{
								QVariantList vlist = k.value().toList();
								bool oldblk = tab->blockSignals(true);
								/* see the comment in the 'case Multiselect:' above */
								for (int j = 0; j < tab->rowCount(); j++)
								{
								  if (! vlist.contains(tab->item(j, 0)->data(Qt::UserRole).toString()))
								    tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() & (~ Qt::ItemIsSelectable));
								}
								QTableWidgetSelectionRange range(0, 0, tab->rowCount() - 1,
                                               tab->columnCount() - 1);
								tab->setRangeSelected(range, true);
								connect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

								tab->blockSignals(oldblk);
							}
						}
            break;
					default:
						QLineEdit *lineEdit;
						lineEdit = qobject_cast<QLineEdit*>(found);
						if (lineEdit != 0)
						{
							lineEdit->setText(k.value().toString());
							storeFilterValue(-1, lineEdit);
						}
						break;
				}			
				windowIdx++;
			}//end of while _defaultTypes
	}

  emit updated();
}

void ParameterWidget::resetMultiselect(QTableWidgetItem *item)
{
  QTableWidget *tab = qobject_cast<QTableWidget*>(sender());
  if (! tab)
    return;

  if (DEBUG)
    qDebug("%s::resetMultiselect(%p) entered for %p",
           qPrintable(objectName()), item, tab);

  disconnect(tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(resetMultiselect(QTableWidgetItem*)));

  for (int j = 0; j < tab->rowCount(); j++)
    tab->item(j, 0)->setFlags(tab->item(j, 0)->flags() | Qt::ItemIsSelectable);

  tab->selectRow(tab->row(item));
  storeFilterValue(-1, tab); // why is this necessary?
}

void ParameterWidget::changeFilterObject(int index)
{
  XComboBox *mybox = (XComboBox *)sender();
  QStringList split = mybox->itemData(index).toString().split(":");
  QString row = split.at(0);
  int type = split.at(1).toInt();
  XSqlQuery qry;

  QWidget *widget = _filterGroup->findChild<QWidget *>("widget" + row);
  QWidget *button = _filterGroup->findChild<QToolButton *>("button" + row);
  QHBoxLayout *layout = _filterGroup->findChild<QHBoxLayout *>("widgetLayout1" + row);;

  QPair<QString, ParameterWidgetTypes> tempPair;


  if (widget && layout && button)
    delete widget;
  else
    return;

  QWidget *newWidget = 0;
  switch (type)
  {
  case Date:
    {
      DLineEdit *dLineEdit= new DLineEdit(_filterGroup);
      newWidget = dLineEdit;
      connect(dLineEdit, SIGNAL(newDate(QDate)), this, SLOT( storeFilterValue() ) );
    }
    break;
  case User:
    {
      UsernameCluster *usernameCluster = new UsernameCluster(_filterGroup);
      newWidget = usernameCluster;
      usernameCluster->setNameVisible(false);
      usernameCluster->setDescriptionVisible(false);
      usernameCluster->setLabel("");

      connect(button, SIGNAL(clicked()), usernameCluster, SLOT( deleteLater() ) );
      connect(usernameCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Crmacct:
    {
      CRMAcctCluster *crmacctCluster = new CRMAcctCluster(_filterGroup);
      newWidget = crmacctCluster;
      crmacctCluster->setNameVisible(false);
      crmacctCluster->setDescriptionVisible(false);
      crmacctCluster->setLabel("");

      connect(button, SIGNAL(clicked()), crmacctCluster, SLOT( deleteLater() ) );
      connect(crmacctCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case Contact:
    {
      ContactCluster *contactCluster = new ContactCluster(_filterGroup);
      newWidget = contactCluster;
      contactCluster->setDescriptionVisible(false);
      contactCluster->setLabel("");

      connect(button, SIGNAL(clicked()), contactCluster, SLOT( deleteLater() ) );
      connect(contactCluster, SIGNAL(newId(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;
  case XComBox:
    {
      XComboBox *xBox = new XComboBox(_filterGroup);
      newWidget = xBox;
			xBox->setNullStr("--- Please Select ---");
			xBox->setAllowNull(true);

      xBox->setType(_comboTypes[mybox->currentText()]);
      if (_comboTypes[mybox->currentText()] == XComboBox::Adhoc)
      {
        qry.prepare( _query[mybox->currentText()] );

        qry.exec();
        xBox->populate(qry);
      }
			//xBox->setAllowNull(true);
      connect(button, SIGNAL(clicked()), xBox, SLOT( deleteLater() ) );
      connect(xBox, SIGNAL(newID(int)), this, SLOT( storeFilterValue(int) ) );
    }
    break;

  case Multiselect:
    {
      QTableWidget *tab = new QTableWidget(_filterGroup);
#ifdef Q_WS_MAC
      QFont f = tab->font();
      f.setPointSize(f.pointSize() - 2);
      tab->setFont(f);
#endif
      newWidget = tab;
      tab->setShowGrid(false);
      tab->setSelectionBehavior(QAbstractItemView::SelectRows);
      tab->setSelectionMode(QAbstractItemView::ExtendedSelection);
      tab->horizontalHeader()->setVisible(false);
      tab->horizontalHeader()->setStretchLastSection(true);
      tab->verticalHeader()->setVisible(false);
      tab->setColumnCount(1);

      XSqlQuery msq;
      msq.prepare(_query[mybox->currentText()]);
      msq.exec();
      for (int i = 0; msq.next(); i++)
      {
        if (i >= tab->rowCount())
          tab->insertRow(tab->rowCount());
        QTableWidgetItem *item = new QTableWidgetItem(msq.value(1).toString());
        item->setData(Qt::UserRole, msq.value(0));
        item->setFlags(item->flags() & (~ Qt::ItemIsEditable));
        if (DEBUG)
          qDebug("%s::changeFilterObject() Multiselect added %s (%s) at %d",
                 qPrintable(objectName()), qPrintable(item->text()),
                 qPrintable(item->data(Qt::UserRole).toString()), i);
        tab->setItem(i, 0, item);
        tab->setRowHeight(i, (tab->fontMetrics().height() * 110) / 100); // TODO: a better way?
      }
      // keep the filter vertically small
      tab->setMaximumHeight(qMin(6, tab->rowCount()) * tab->rowHeight(0));
      tab->show();

      connect(tab, SIGNAL(itemSelectionChanged()), this, SLOT(storeFilterValue()));
    }
    break;

  default:
    {
      QLineEdit *lineEdit = new QLineEdit(_filterGroup);
      newWidget = lineEdit;

      connect(button, SIGNAL(clicked()), lineEdit, SLOT( deleteLater() ) );
      connect(lineEdit, SIGNAL(editingFinished()), this, SLOT( storeFilterValue() ) );
    }
    break;
  }

  if (newWidget)
  {
    connect(button, SIGNAL(clicked()), newWidget, SLOT( deleteLater() ) );
    newWidget->setObjectName("widget" + row);
    layout->insertWidget(0, newWidget);
  }
	_saveButton->setDisabled(true);
}

void ParameterWidget::clearFilters()
{
  QStringList del;
  QList<QLayout *> llist = _filterGroup->findChildren<QLayout*>();
  QList<QWidget *> wlist = _filterGroup->findChildren<QWidget*>();

  // Find layout deletion candidates first
  for (int i = 0; i < llist.count(); i++)
  {
    if (llist.at(i)->objectName().startsWith("topLayout") ||
        llist.at(i)->objectName().startsWith("widgetLayout2") ||
        llist.at(i)->objectName().startsWith("buttonLayout"))
      del.append(llist.at(i)->objectName());
  }

  // Then find widget deletion candidates
  for (int i = 0; i < wlist.count(); i++)
  {
    if (wlist.at(i)->objectName().startsWith("xcombo") ||
        wlist.at(i)->objectName().startsWith("widget") ||
        wlist.at(i)->objectName().startsWith("button"))
      del.append(wlist.at(i)->objectName());
  }

  // Perform deletions
  while (del.count())
  {
    QObject *obj = _filterGroup->findChild<QObject*>(del.at(0));
    delete obj;
    del.takeFirst();
  }

  _filterValues.clear();
  _usedTypes.clear();
  _addFilterRow->setDisabled(false);
}


ParameterList ParameterWidget::parameters()
{
  ParameterList params;

  appendValue(params);

  return params;
}

void ParameterWidget::repopulateComboboxes()
{
  QRegExp rx("^xcomboBox");

  QList<XComboBox *> xlist = _filterGroup->findChildren<XComboBox*>(rx);
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > j(_types);
  QString current;
  QString value;
  int idx;
  for (int i = 0; i < xlist.count(); i++)
  {
    current = xlist.at(i)->currentText();
    QStringList split = xlist.at(i)->itemData(xlist.at(i)->currentIndex()).toString().split(":");
    disconnect(xlist.at(i), 0, this, 0);
    xlist.at(i)->clear();
    while (j.hasNext())
    {
      j.next();
      QPair<QString, ParameterWidgetTypes> tempPair = j.value();
      value = split[0] + ":" + QString().setNum(tempPair.second);
      if ( !containsUsedType(j.key()) || current == j.key() )
      {
        xlist.at(i)->addItem(j.key(), value );
      }
    }
    j.toFront();

    idx = xlist.at(i)->findText(current);
    xlist.at(i)->setCurrentIndex(idx);
		connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( addUsedType() ) );
    connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( changeFilterObject(int)) );
		connect(xlist.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT( repopulateComboboxes() ) );
  }
}

void ParameterWidget::addUsedType()
{
	XComboBox *mybox = (XComboBox *)sender();
	QVariant filterVar(mybox->itemData(mybox->currentIndex()));
  QString filterType = filterVar.toString();
  QStringList split = filterType.split(":");
	int row = split[0].toInt();

	if (!_usedTypes.isEmpty())
    _usedTypes.remove(row);

  _usedTypes[row] = mybox->currentText();
}

void ParameterWidget::removeParam(int pRow)
{
  QLayoutItem *test;
  QLayoutItem *test2;

  test = _filtersLayout->itemAtPosition(pRow, 0)->layout()->itemAt(0);
  XComboBox *mybox = (XComboBox*)test->widget();

  QVariant filterVar(mybox->itemData(mybox->currentIndex()));
  QString filterType = filterVar.toString();
  QStringList split = filterType.split(":");

  QPair<QString, QVariant> tempPair = _filterValues.value(split[0].toInt());

  _filterValues.remove(split[0].toInt());


  test2 = _filtersLayout->itemAtPosition(pRow, 0)->layout()->takeAt(1);
  delete test2;
  test2 = 0;
  _filtersLayout->update();
  _addFilterRow->setDisabled(false);
  repopulateComboboxes();
  if (!mybox->currentText().isEmpty())
    _usedTypes.remove(pRow);
  emit updated();
}

void ParameterWidget::save()
{
  QString filter = "";
  QString variantString;
  QString username;
  QString query;
  QVariant tempVar;
	QDate variantDate;
	QDate today = QDate::currentDate();
  int filter_id;

  QMapIterator<int, QPair<QString, QVariant> > i(_filterValues);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, QVariant> tempPair = i.value();

    tempVar = tempPair.second;

    QLayoutItem *test = _filtersLayout->itemAtPosition(i.key(), 0)->layout()->itemAt(0);
    XComboBox* mybox = (XComboBox*)test->widget();
    QStringList split = mybox->itemData(mybox->currentIndex()).toString().split(":");

    if ( tempVar.canConvert(QVariant::String) )
    {
			if ( tempVar.canConvert<QDate>() )
				variantDate = tempVar.toDate();

			if (variantDate.isValid())
			{
				variantString = QString().setNum(today.daysTo(variantDate));
			}
			else
			{
				variantString = tempVar.toString();
			}
      filter = filter + tempPair.first + ":" + variantString + ":" + split[1] + "|";
    }
    else if (tempVar.canConvert(QVariant::StringList))
      filter += tempPair.first + ":" + tempVar.toStringList().join(",")
                               + ":" + split[1] + "|";
  }

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  ParameterList params;
  params.append("filter", filter);
  if (_filterList->id())
    params.append("filtersetname", _filterList->currentText());
  params.append("classname", classname);
  if (_shared)
    params.append("shared", true);

  filterSave newdlg(this);
  newdlg.set(params);
  filter_id = newdlg.exec();
  if (!filter_id)
    return;
  emit filterSetSaved();

  setSavedFilters();
  _filterList->setId(filter_id);
}

void ParameterWidget::setFiltersVisabiltyPreference()
{
  QString pname;
  if(window())
    pname = window()->objectName() + "/";
  _settingsName = pname + this->objectName();

  if (!_settingsName.isEmpty() && _x_preferences)
  {
    _x_preferences->set(_settingsName + "/checked", _filterGroup->isVisible());
  }
}
void ParameterWidget::setFiltersDefault()
{
	QString pname;
	int filter_id;

  if(parent())
    pname = parent()->metaObject()->className();
		pname += "/";
  _settingsName2 = pname + this->objectName();

	filter_id = _filterList->id(_filterList->currentIndex());

	if (!_settingsName2.isEmpty() && _x_preferences)
  {
    _x_preferences->set(_settingsName2 + "/filter_default", filter_id);
  }

}
void ParameterWidget::setSavedFilters(int defaultId)
{
  QString query;
  XSqlQuery qry;

  if(this->parent())
  {
    QString classname(parent()->objectName());
    if (classname.isEmpty())
      classname = parent()->metaObject()->className();

    query = " SELECT 0 AS filter_id, :none AS filter_name, 1 AS seq "
            " UNION "
            " SELECT filter_id, filter_name, 2 AS seq "
            " FROM filter "
            " WHERE COALESCE(filter_username,current_user)=current_user "
            " AND filter_screen=:screen "
            " ORDER BY seq, filter_name ";

    qry.prepare(query);

    qry.bindValue(":screen", classname);
    qry.bindValue(":none", tr("Default"));
    qry.exec();
    if (defaultId)
      _filterList->populate(qry, defaultId);
    else
      _filterList->populate(qry, 0);
  }
}

void ParameterWidget::setSavedFiltersIndex(QString filterSetName)
{
  _filterList->findText(filterSetName);
  _filterList->setText(filterSetName);
}

void ParameterWidget::setType(QString pName, QString pParam, ParameterWidgetTypes type, QVariant pDefault, QVariant extraInfo)
{
  _types[pName] = qMakePair(pParam, type);
  if (pDefault != 0)
    _defaultTypes[pName] = pDefault;

  if (type == Multiselect)
  {
    if (extraInfo.type() == QVariant::String)
      _query[pName] = extraInfo.toString();
    else
      qWarning("%s::setType(%s, %s, %d, %s, %s) called for Multiselect but "
               "was not given a query to use",
               qPrintable(objectName()), qPrintable(pName), qPrintable(pParam),
               type, qPrintable(pDefault.toString()),
               qPrintable(extraInfo.toString()));
  }
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, XComboBox::XComboBoxTypes xType, QVariant pDefault)
{
  _comboTypes[pName] = xType;
  _types[pName] = qMakePair(pParam, XComBox);
	if (pDefault != 0)
	{
		_defaultTypes[pName] = pDefault;
	}
}

void ParameterWidget::setXComboBoxType(QString pName, QString pParam, QString pQry, QVariant pDefault)
{
  _comboTypes[pName] = XComboBox::Adhoc;
  _query[pName] = pQry;
  _types[pName] = qMakePair(pParam, XComBox);
	if (pDefault != 0)
	{
		_defaultTypes[pName] = pDefault;
	}
}

void ParameterWidget::sManageFilters()
{
  ParameterList params;
  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();
  params.append("screen", classname);

  filterManager *newdlg = new filterManager(this, "");
  newdlg->set(params);
  newdlg->exec();
}

void ParameterWidget::toggleSave()
{
  if (!_filterValues.isEmpty())
  {
    _saveButton->setDisabled(false);
  }
}

// get the row number of a particular filter widget
// this has to change if we change the way things get laid out
int ParameterWidget::getFilterIndex(const QWidget *filterwidget)
{
  int index = -1;

  for (int i = 1; i < _filtersLayout->rowCount(); i++)
  {
    QLayoutItem *rowLI   = _filtersLayout->itemAtPosition(i, 1);
    if (rowLI)
    {
      QGridLayout *rowgrid = qobject_cast<QGridLayout *>(rowLI->layout());
      if (rowgrid)
      {
        QLayoutItem *childLI = rowgrid->itemAtPosition(0, 0);
        QHBoxLayout *filterlyt = qobject_cast<QHBoxLayout *>(childLI->layout()->itemAt(0)->layout());
        if (filterlyt && filterlyt->itemAt(0)->widget() == filterwidget)
        {
          index = i;
          break;
        }
      }
    }
  }

  return index;
}

// get the filter widget at a particular row
// this has to change if we change the way things get laid out
QWidget *ParameterWidget::getFilterWidget(const int index)
{
  QWidget *filterwidget = 0;

  QGridLayout *rowgrid = _filtersLayout->findChild<QGridLayout *>("topLayout"
                                                      + QString::number(index));
  QLayoutItem *childLI = rowgrid->itemAtPosition(0, 0)->layout()->itemAt(0);
  QHBoxLayout *datalyt = (QHBoxLayout *)childLI->layout();
  QLayoutItem *filterLI = datalyt->itemAt(0);
  if (filterLI)
    filterwidget = filterLI->widget();

  return filterwidget;
}

// stores the value of a filter object into the filtervalues map
void ParameterWidget::storeFilterValue(int pId, QObject* filter)
{
  if (!filter)
    filter = (QObject *)sender();

  if (DEBUG)
    qDebug("%s::storeFilterValue(%d, %p)",
           qPrintable(objectName()), pId, filter);

  if (! filter)
  {
    qWarning("%s::storeFilterValue(%d, %p) could not find filter",
           qPrintable(objectName()), pId, filter);
    return;
  }

  int foundRow = getFilterIndex(qobject_cast<QWidget*>(filter));
  if (foundRow < 0)
  {
    qWarning("%s::storeFilterValue(%d, %p) could not find filter index",
           qPrintable(objectName()), pId, filter);
    return;
  }

  QLayoutItem *selectorLI = _filtersLayout->itemAtPosition(foundRow, 0)->layout()->itemAt(0);
  XComboBox *selector = qobject_cast<XComboBox*>(selectorLI->widget());
  QString currText = selector->currentText();
  QPair<QString, ParameterWidgetTypes> tempPair = _types[currText];

  QString classname(filter->metaObject()->className());

  // check usernamecluster before pid because we key off text value for users
  if (classname == "UsernameCluster")
  {
    UsernameCluster *usernameCluster = (UsernameCluster *)filter;
    QString username = usernameCluster->username();
    _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(username));
    emit updated();
		if (classname == "XComboBox")
		{
			_saveButton->setDisabled(true);
		}
  }
  else if (pId != -1)
  {
    _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(pId));
    emit updated();
  }
  else if (classname == "QLineEdit")
  {
    QLineEdit *lineEdit = (QLineEdit *)filter;
    _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(lineEdit->text()));
    emit updated();
  }
  else if (classname == "DLineEdit")
  {
    DLineEdit *dateedit = qobject_cast<DLineEdit *>(filter);
    _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(dateedit->date()));
    emit updated();
  }
  else if (classname == "QTableWidget")
  {
    QList<QTableWidgetItem*> selected = (qobject_cast<QTableWidget*>(filter))->selectedItems();
    QVariantList value;
    for (int i = 0; i < selected.size(); i++)
      value.append(selected.at(i)->data(Qt::UserRole));
    _filterValues[foundRow] = qMakePair(tempPair.first, QVariant(value));
    emit updated();
  }

  if (!_usedTypes.isEmpty())
    _usedTypes.remove(foundRow);
  _usedTypes[foundRow] = selector->currentText();

  _addFilterRow->setDisabled(false);
  repopulateComboboxes();
}

QString ParameterWidget::getParameterTypeKey(QString pValue)
{
  QMapIterator<QString, QPair<QString, ParameterWidgetTypes> > i(_types);
  while (i.hasNext())
  {
    i.next();
    QPair<QString, ParameterWidgetTypes> tempPair = i.value();

    if (pValue == tempPair.first)
      return i.key();
  }

  return QString();
}


//updates selected filter set to be the default upon loading of the current screen
//for the current user
void ParameterWidget::setSelectedFilter(int filter_id)
{
  XSqlQuery qry;

  QString classname(parent()->objectName());
  if (classname.isEmpty())
    classname = parent()->metaObject()->className();

  QString query = "UPDATE filter SET filter_selected=false "
                  "WHERE filter_screen=:screen AND filter_username=current_user";
  QString query2 = "UPDATE filter SET filter_selected=true "
                   "WHERE filter_screen=:screen AND filter_id=:id AND filter_username=current_user";
  qry.prepare(query);
  qry.bindValue(":screen", classname);

  qry.exec();

  qry.prepare(query2);
  qry.bindValue(":screen", classname);
  qry.bindValue(":id", filter_id);

  qry.exec();
}

bool ParameterWidget::containsUsedType(QString value)
{
  QMapIterator<int, QString > i(_usedTypes);
  while (i.hasNext()) {
    i.next();
    if (i.value() == value)
      return true;
  }

  return false;
}

// script exposure ///////////////////////////////////////////////////////////

void ParameterWidgetfromScriptValue(const QScriptValue &obj, ParameterWidget* &item)
{
  item = qobject_cast<ParameterWidget*>(obj.toQObject());
}

QScriptValue ParameterWidgettoScriptValue(QScriptEngine *engine, ParameterWidget* const &item)
{
  return engine->newQObject(item);
}

QScriptValue constructParameterWidget(QScriptContext *context,
                                      QScriptEngine  *engine)
{
  ParameterWidget *obj = 0;
  if (context->argumentCount() == 1 &&
      qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)));
  else if (context->argumentCount() >= 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    obj = new ParameterWidget(qscriptvalue_cast<QWidget*>(context->argument(0)),
                              context->argument(1).toString());

  return engine->toScriptValue(obj);
}

void setupParameterWidget(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, ParameterWidgettoScriptValue, ParameterWidgetfromScriptValue);

  QScriptValue widget = engine->newFunction(constructParameterWidget);

  widget.setProperty("Crmacct", QScriptValue(engine, ParameterWidget::Crmacct), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("User", QScriptValue(engine, ParameterWidget::User), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Text", QScriptValue(engine, ParameterWidget::Text), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Date", QScriptValue(engine, ParameterWidget::Date), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("XComBox", QScriptValue(engine, ParameterWidget::XComBox), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact", QScriptValue(engine, ParameterWidget::Contact), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Multiselect", QScriptValue(engine, ParameterWidget::Multiselect), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ParameterWidget", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}


