/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtextedit.h"

XTextEdit::XTextEdit(QWidget *pParent) :
  QTextEdit(pParent)
{
  _mapper = 0;
}

void XTextEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  disconnect(this, SIGNAL(lostFocus()), this, SLOT(updateMapperData()));
  if (acceptRichText())
    m->addMapping(this, _fieldName, "html", "defaultText");
  else
    m->addMapping(this, _fieldName, "plainText", "defaultText");
  _mapper = m;
  connect(this, SIGNAL(lostFocus()), this, SLOT(updateMapperData()));
}

void XTextEdit::updateMapperData()
{
  if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),
                     _mapper->mappedSection(this))).toString() != toPlainText())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),
                                                        _mapper->mappedSection(this)),
                                toPlainText()); 
}
