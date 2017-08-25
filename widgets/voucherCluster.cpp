/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherCluster.h"

VoucherCluster::VoucherCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new VoucherClusterLineEdit(this, pName));
}

VoucherClusterLineEdit::VoucherClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "vohead", "vohead_id", "vohead_number", "(SELECT vend_number FROM vendinfo WHERE vend_id=vohead_vend_id)", 0, 0, pName)
{
  setTitles(tr("Voucher"), tr("Vouchers"));
  setUiName("voucher");
  setEditPriv("EditAPOpenItems");
  setViewPriv("ViewAPOpenItems");
  setExtraClause("(checkVoucherSitePrivs(vohead_id))");
}
