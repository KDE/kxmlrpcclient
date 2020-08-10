/*
    SPDX-FileCopyrightText: 2003-2004 Frerich Raabe <raabe@kde.org>
    SPDX-FileCopyrightText: 2003-2004 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/
/**
  @file

  This file is part of KXmlRpc and defines our internal classes.

  @author Frerich Raabe <raabe@kde.org>
  @author Tobias Koenig <tokoe@kde.org>
  @author Narayan Newton <narayannewton@gmail.com>
*/

#ifndef KXML_RPC_QUERY_H
#define KXML_RPC_QUERY_H

#include "kxmlrpcclient_export.h"

#include <QList>
#include <QObject>
#include <QVariant>
#include <QMap>
#include <QUrl>

namespace KIO
{
class Job;
}
class KJob;
class QString;

/** Namespace for XmlRpc related classes */
namespace KXmlRpc
{
class QueryPrivate;
/**
  @brief
  Query is a class that represents an individual XML-RPC call.

  This is an internal class and is only used by the KXmlRpc::Client class.
  @internal
  @since 5.8
 */
class KXMLRPCCLIENT_EXPORT Query : public QObject
{
    friend class Result;
    Q_OBJECT

public:

    /**
      Constructs a query.

      @param id an optional id for the query.
      @param parent an optional parent for the query.
     */
    static Query *create(const QVariant &id = QVariant(), QObject *parent = nullptr);

public Q_SLOTS:
    /**
      Calls the specified method on the specified server with
      the given argument list.

      @param server the server to contact.
      @param method the method to call.
      @param args an argument list to pass to said method.
      @param jobMetaData additional arguments to pass to the KIO::Job.
     */
    void call(const QUrl &server, const QString &method,
              const QList<QVariant> &args,
              const QMap<QString, QString> &jobMetaData);

Q_SIGNALS:
    /**
      A signal sent when we receive a result from the server.
     */
    void message(const QList<QVariant> &result, const QVariant &id);

    /**
      A signal sent when we receive an error from the server.
     */
    void fault(int errCode, const QString &errString, const QVariant &id);

    /**
      A signal sent when a query finishes.
     */
    void finished(Query *query);

private:
    explicit Query(const QVariant &id, QObject *parent = nullptr);
    ~Query();

    QueryPrivate *const d;

    Q_PRIVATE_SLOT(d, void slotData(KIO::Job *, const QByteArray &))
    Q_PRIVATE_SLOT(d, void slotResult(KJob *))
};

} // namespace XmlRpc

#endif

