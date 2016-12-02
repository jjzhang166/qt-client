#ifndef CREATEFISCALYEAR_H
#define CREATEFISCALYEAR_H

#include "ui_createfiscalyear.h"

#include "guiclient.h"
#include "xdialog.h"

class createFiscalYear : public XDialog, private Ui::createFiscalYear
{
    Q_OBJECT

  public:
    explicit createFiscalYear(QWidget *parent = 0);
    static bool hasPriv(const int mode = cView, const int pId = -1);

  public slots:
    void sCancel();
    void sHandleButtons();
    void sHandleChoice(int);
    void sSave();

};

#endif // CREATEFISCALYEAR_H
