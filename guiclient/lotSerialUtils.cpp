/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2013 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 *
 * Originally contributed by Specter Business Solutions - specter.ca/business
 */

#include "lotSerialUtils.h"
#include <metasql.h>
#include <xsqlquery.h>
#include <QtSql>
#include <dlineedit.h>
#include <xcombobox.h>

LotSerialUtils::LotSerialUtils()
{

    XSqlQuery q;
    bool success = q.exec("SELECT char_name, char_type, char_id "
                "FROM char "
                "JOIN charuse ON (char_id=charuse_char_id AND charuse_target_type = 'LS') "
                "ORDER BY char_name ASC");
    while(q.next())
    {
        _charNames.append(q.value("char_name").toString());
        _charIds.append(q.value("char_id").toInt());
        _charTypes.append(q.value("char_type").toInt());
    }
    if (!success)
    {
        qDebug() << "LotSerialChars: Error" << q.lastError().text();
    }

}

LotSerialUtils::~LotSerialUtils()
{
    // Destructor provided for potential future use.
}

QList<int> LotSerialUtils::getLotCharIds() const
{
    return _charIds;
}

QList<int> LotSerialUtils::getLotCharTypes() const
{
    return _charTypes;
}

QStringList LotSerialUtils::getLotCharNames() const
{
    return _charNames;
}

int LotSerialUtils::numLotChars() const
{
    return _charNames.size();
}


void LotSerialUtils::updateLotCharacteristics(int ls_id, const QList<QWidget *> &widgets) const
{
    int i;
    for (i = 0; i < numLotChars(); i++)
    {
        QString char_text;

        if (_charTypes.at(i) == 0)
        {
            char_text = qobject_cast<QLineEdit *>(widgets.at(i))->text();
        }
        else if (_charTypes.at(i) == 1)
        {
            char_text = qobject_cast<XComboBox *>(widgets.at(i))->currentText();
        }
        else if (_charTypes.at(i) == 2)
        {
            QDate d = qobject_cast<DLineEdit *>(widgets.at(i))->date();
            char_text = d.toString("yyyy-MM-dd");
        }

        XSqlQuery q;
        bool success = q.exec(QString("INSERT INTO charass "
                              "(charass_value, charass_target_type, charass_target_id, charass_char_id) "
                              "VALUES ('%1', 'LS', %2, %3)").arg(char_text).arg(ls_id).arg(_charIds.at(i)));
        if (!success)
        {
            qDebug() << "UpdateLotCharacteristics Error:" << q.lastError().text();
            continue;
        }
    }
}

void LotSerialUtils::setParams(ParameterList &params)
{
    int i;
    int ls_id = params.value("ls_id").toInt();
    QVariantList char_id_index_list;
    QStringList charIdIndexClause;
    for (i = 0; i < _charIds.count(); i++)
    {
        char_id_index_list.append(i+1);
        QString charass_value;
        XSqlQuery q;
        q.exec(QString("SELECT charass_value FROM charass WHERE "
                                      "charass_target_type='LS' AND charass_target_id=%1 "
                                      "AND charass_char_id=%2;").arg(ls_id).arg(_charIds.at(i)));
        if (q.first()) {
            charass_value = q.value("charass_value").toString();
        }
        charIdIndexClause.append(QString("charass_alias%1.charass_value='%2'").arg(i+1).arg(charass_value));
    }
    params.append("char_id_index_list", char_id_index_list);
    params.append("charIdIndexClause", charIdIndexClause.join(" AND ").prepend(" AND "));
}

int LotSerialUtils::getNextLotId()
{
    XSqlQuery q;
    q.exec("SELECT last_value from ls_ls_id_seq");
    if (q.first())
    {
        return q.value("last_value").toInt();
    }
    return -1;
}


QList<int> LotSerialUtils::getLotSerialIds()
{
    QList<int> ls_ids;
    XSqlQuery q;
    bool success = q.exec("SELECT ls_id from ls");
    if (!success)
    {
        qDebug() << "getLotSerialIds error:" << q.lastError().text();
        return ls_ids;
    }
    while(q.next())
    {
        ls_ids.append(q.value("ls_id").toInt());
    }
    return ls_ids;

}

QList<QWidget *> LotSerialUtils::addLotCharsToGridLayout(QWidget *parent, QGridLayout *layout, const LotSerialUtils &ls_chars)
{
    int numChars = ls_chars.numLotChars();
    int rowCount = layout->rowCount();
    QStringList charNames = ls_chars.getLotCharNames();
    QList<int> charTypes = ls_chars.getLotCharTypes();
    QList<int> charIds = ls_chars.getLotCharIds();

    QList<QWidget *> widgets;
    for (int i=0; i < numChars; i++)
    {
        QLabel *label = new QLabel(charNames.at(i), parent);
        QWidget * edit = NULL;
        if (charTypes.at(i)== 2)
        {
            edit = new DLineEdit(parent);
        }
        else if (charTypes.at(i) == 1)
        {
            XComboBox *x = new XComboBox(parent);
            XSqlQuery q;
            q.prepare(QString("SELECT charopt_value FROM charopt WHERE charopt_char_id=%1 ORDER BY charopt_id ASC;").arg(charIds.at(i)));
            q.exec();
            while(q.next()) {
                x->addItem(q.value("charopt_value").toString());
            }
            edit = x;

        }
        else
        {
            edit = new QLineEdit(parent);
        }
        layout->addWidget(label, rowCount + i, 0, 0);
        layout->addWidget(edit, rowCount + i, 1, 0);
        widgets.append(edit);
    }
    return widgets;
}


void _addCharsToItem(QTreeWidgetItem *item, int column, int numInitialColumns)
{
    if (item == NULL)
    {
        return;
    }
    XSqlQuery q;
    bool success = q.exec(QString("SELECT charass.charass_value FROM charass LEFT JOIN ls on "
                " charass.charass_target_id=ls.ls_id WHERE charass.charass_target_type = 'LS' AND ls.ls_number='%1'"
                " ORDER BY charass.charass_id ASC").arg(item->text(column)));
    if (!success)
    {
        qDebug() << __FUNCTION__ << q.lastError().text();
    }
    int j = numInitialColumns;
    while (q.next())
    {
        item->setText(j, q.value("charass_value").toString());
        j++;
    }

}


void LotSerialUtils::addCharsToTreeWidget(QTreeWidget *tree, int ls_numberColumn,
                                          int numInitialColums, bool repopulate)
{
    int numRows = tree->topLevelItemCount();
    if (repopulate)
    {
        for (int i=0; i < numRows; i++)
        {
            _addCharsToItem(tree->topLevelItem(i), ls_numberColumn, numInitialColums);
        }
    }
    else
    {
        _addCharsToItem(tree->topLevelItem(numRows - 1), ls_numberColumn, numInitialColums);
    }

}

