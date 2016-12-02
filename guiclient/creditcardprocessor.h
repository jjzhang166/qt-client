/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CREDITCARDPROCESSOR_H
#define CREDITCARDPROCESSOR_H

#include <QHash>
#include <QObject>
#include <QString>
#if QT_VERSION < 0x050000
#include <QHttp>
#else
#include <QNetworkAccessManager>
#endif
#include <parameter.h>

class QSslCertificate;

class CreditCardProcessor : public QObject
{
  Q_OBJECT

  Q_ENUMS(CCTransaction)
  Q_ENUMS(CreditCardProcessor::FraudCheck)
  Q_FLAGS(CreditCardProcessor::FraudChecks)

  public:
    enum CCTransaction { Authorize, Reverse, Capture, Charge, Credit, Void };
    enum FraudCheck    { Match              = 0x0000, NoMatch            = 0x0001,
                         NotAvail           = 0x0002, Address            = 0x0004,
                         PostalCode         = 0x0008, Name               = 0x0010,
                         NotProcessed       = 0x0020, Invalid            = 0x0040,
                         ServiceUnavailable = 0x0080, IssuerNotCertified = 0x0100,
                         Suspicious         = 0x0200, Unsupported        = 0x0400
    };
    Q_DECLARE_FLAGS(FraudChecks, FraudCheck)

    class FraudCheckResult
    {
      public:
        QChar       code;
        int         sev;  /*TODO: FraudChecks sev; */
        QString     text;

        FraudCheckResult(QChar pcode, int /*TODO: FraudChecks*/ psev, QString ptext);
    };

    virtual ~CreditCardProcessor();

    // no public constructor for abstract class, just a factory
    Q_INVOKABLE static CreditCardProcessor *getProcessor(const QString = QString());
    Q_INVOKABLE static bool                 certificateIsValid(const QSslCertificate *cert);

    // these are the primary transaction handlers and should not be overridden:
    virtual int authorize(const int pccardid, const QString &pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid);
    virtual int charge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid);
    virtual int chargePreauthorized(const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid);
    virtual int credit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid);
    virtual int reversePreauthorized(const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int prefid);
    virtual int voidPrevious(int &);
    
    // methods for script access
    Q_INVOKABLE static ParameterList authorize(const ParameterList &);
    Q_INVOKABLE static ParameterList charge(const ParameterList &);
    Q_INVOKABLE static ParameterList chargePreauthorized(const ParameterList &);
    Q_INVOKABLE static ParameterList credit(const ParameterList &);
    Q_INVOKABLE static ParameterList voidPrevious(const ParameterList &);

    // these are support methods that typically won't be overridden
    Q_INVOKABLE virtual int	testConfiguration();
    Q_INVOKABLE virtual int     defaultPort(bool = false);
    Q_INVOKABLE virtual QString defaultServer();
    Q_INVOKABLE virtual bool    handlesChecks();
    Q_INVOKABLE virtual bool    handlesCreditCards();
    Q_INVOKABLE virtual bool    isLive();
    Q_INVOKABLE virtual bool    isTest();
    Q_INVOKABLE virtual void    reset();

    Q_INVOKABLE static  QString errorMsg();		// most recent error
    Q_INVOKABLE static  QString errorMsg(const int);
    Q_INVOKABLE static  int     printReceipt(const int);
    Q_INVOKABLE static  QString typeToCode(CCTransaction ptranstype);

  protected:
    CreditCardProcessor();

    // do* handle the processor-specific processing and are expected to be overridden
    virtual int doAuthorize(const int pccardid, const QString &pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams);
    virtual int doCharge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int doChargePreauthorized(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int doCredit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams);
    virtual int doReversePreauthorized(const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int pccpayid, ParameterList &pparams);
    virtual int doVoidPrevious(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams);
    virtual int doTestConfiguration();

    virtual FraudCheckResult *avsCodeLookup(QChar pcode);
    virtual QString buildURL(const QString, const QString, const bool);
    virtual int     checkCreditCard(const int pccid, const QString &pcvv, QString &pccard_x);
    virtual int     checkCreditCardProcessor()	{ return false; };
    virtual FraudCheckResult *cvvCodeLookup(QChar pcode);
    static  double  currToCurr(const int, const int, const double, int * = 0);
    virtual int     fraudChecks();
    virtual int     sendViaHTTP(const QString&, QString&);
    virtual int     updateCCPay(int &, ParameterList &);
    virtual bool    waitForHTTP();

    QList<FraudCheckResult*> _avsCodes;
    QList<FraudCheckResult*> _cvvCodes;
    QString             _company;
    QString		_defaultLiveServer;
    QString		_defaultTestServer;
    int			_defaultLivePort;
    int			_defaultTestPort;
    static QString	_errorMsg;
    bool                _ignoreSslErrors;
    static QHash<int, QString>	_msgHash;
    bool		_passedAvs;
    bool		_passedCvv;
    QString		_plogin;
    QString		_ppassword;
    QString		_pport;
    QString		_pserver;
    QString             _pemfile;
    #if QT_VERSION < 0x050000
    QHttp             * _http;
    #else
    QNetworkAccessManager *_manager;
    #endif
    QList<QPair<QString, QString> > _extraHeaders;

    protected slots:
    #if QT_VERSION < 0x050000
      void sslErrors(const QList<QSslError> &errors);
    #else
      void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    #endif

};

#endif // CREDITCARDPROCESSOR_H
