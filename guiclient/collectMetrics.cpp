#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QApplication>

#include "guiclient.h"
#include "version.h"

void collectMetrics()
{
  QSqlQuery qry;
  QString result;
  QStringList list;
  QMap<QString,QString> values;
  // Version client
  values.insert("client_version", _Version);

  // Edition
  values.insert("edition", _metrics->value("Application"));

  // custom?
  // TODO: I don't know how we can tell this

  // Users
  result = "";
  qry.exec("SELECT count(*) FROM pg_stat_activity WHERE datname=current_database();");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("users", result);

  // clients
  // TODO: Don't know how to get this

  // packages loaded
  qry.exec("SELECT pkghead_name FROM pkghead;");
  list.clear();
  while(qry.next())
    list.append(qry.value(0).toString());
  values.insert("packages", list.join(","));

  // patches
  values.insert("patches", _metrics->value("ServerPatchVersion"));

  // shared memory
  result = "";
  qry.exec("SHOW shared_buffers");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("shared_buffers", result);

  // db version
  result = "";
  qry.exec("SHOW server_version");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_version", result);

  // db name
  result = "";
  qry.exec("SELECT current_database();");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_name", result);

  // db port
  result = "";
  qry.exec("SHOW port");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("database_port", result);

  // ssl
  result = "";
  qry.exec("SHOW ssl");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("ssl", result);

  // enhanced auth
  result = "";
  qry.exec("SELECT usrpref_value "
           "  FROM usrpref "
           " WHERE ( (usrpref_name = 'UseEnhancedAuthentication') "
           "   AND (usrpref_username=CURRENT_USER) ); ");
  if(qry.first())
    result = qry.value(0).toString();
  values.insert("enhanced_auth", result);

  // Create XML document
  QDomDocument doc;
  QDomElement root = doc.createElement("properties");
  doc.appendChild(root);

  QMapIterator<QString, QString> i(values);
  while (i.hasNext())
  {
    i.next();

    QDomElement elem = doc.createElement("property");
    root.appendChild(elem);

    QDomElement name = doc.createElement("name");
    elem.appendChild(name);
  
    QDomText t1 = doc.createTextNode(i.key());
    name.appendChild(t1);

    QDomElement value = doc.createElement("value");
    elem.appendChild(value);
  
    QDomText t2 = doc.createTextNode(i.value());
    value.appendChild(t2);
  }

  // write the xml to a file
  QFile file("xtuple-ns-metrics.xml");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qDebug("whoops");
    return;
  }

  QTextStream out(&file);
  out << doc.toString();

  file.close();

  QString path = QApplication::applicationDirPath();
  QFile f(path+"/network.cfg");
  if(f.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QTextStream in(&f);
    in >> path;
    f.close();
  }
  QString exe = path + "/agent";
#if defined Q_WS_X11
  exe.append(".bin");
#endif
  QProcess * proc = new QProcess();
#if defined Q_WS_MACX
  QStringList arguments;
  arguments << exe;
  proc->start("open", arguments);
#else
  proc->start(exe);
#endif
  
  
}
