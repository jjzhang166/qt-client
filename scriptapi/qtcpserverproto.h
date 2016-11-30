#ifndef __QTCPSERVERPROTO_H__
#define __QTCPSERVERPROTO_H__

#include <QObject>
#include <QtScript>

void setupQTcpServerProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QHostAddress>
#include <QAbstractSocket>

QScriptValue constructQTcpServer(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(QTcpServer*)

class QTcpServerProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTcpServerProto(QObject *parent);

    Q_INVOKABLE void                         close();
    Q_INVOKABLE QString                      errorString() const;
    Q_INVOKABLE bool                         hasPendingConnections() const;
    Q_INVOKABLE bool                         isListening() const;
    Q_INVOKABLE bool                         listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    Q_INVOKABLE virtual QTcpSocket*          nextPendingConnection();
    Q_INVOKABLE void                         pauseAccepting();
    Q_INVOKABLE QNetworkProxy                proxy() const;
    Q_INVOKABLE void                         resumeAccepting();
    Q_INVOKABLE QHostAddress                 serverAddress() const;
    Q_INVOKABLE QAbstractSocket::SocketError serverError() const;
    Q_INVOKABLE quint16                      serverPort() const;
    Q_INVOKABLE void                         setMaxPendingConnections(int numConnections);
    Q_INVOKABLE void                         setProxy(const QNetworkProxy & networkProxy);
    Q_INVOKABLE bool                         setSocketDescriptor(qintptr socketDescriptor);
    Q_INVOKABLE qintptr                      socketDescriptor() const;
    Q_INVOKABLE bool                         waitForNewConnection(int msec = 0, bool * timedOut = 0);

  signals:
    void acceptError(QAbstractSocket::SocketError socketError);
    void newConnection();
};
#endif

#endif
