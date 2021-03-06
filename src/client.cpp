/*
    SPDX-FileCopyrightText: 2003-2004 Frerich Raabe <raabe@kde.org>
    SPDX-FileCopyrightText: 2003-2004 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/
/**
  @file
  This file is part of the API for accessing XML-RPC Servers
  and defines the Client class.

  @brief
  Defines the Client class.

  @author Frerich Raabe <raabe@kde.org>
  @author Tobias Koenig <tokoe@kde.org>
  @author Narayan Newton <narayannewton@gmail.com>
*/

#include "client.h"
#include "kxmlrpcclient_debug.h"
#include "query.h"
#include <kio/job.h>

#include <QVariant>

using namespace KXmlRpc;

class ClientPrivate
{
public:
    ClientPrivate()
        : mUserAgent(QStringLiteral("KDE XMLRPC resources"))
        , mDigestAuth(false)
    {
    }

    void queryFinished(Query *);

    QUrl mUrl;
    QString mUserAgent;
    bool mDigestAuth;
    QList<Query *> mPendingQueries;
};

void ClientPrivate::queryFinished(Query *query)
{
    mPendingQueries.removeAll(query);
    query->deleteLater();
}

Client::Client(QObject *parent)
    : QObject(parent)
    , d(new ClientPrivate)
{
}

Client::Client(const QUrl &url, QObject *parent)
    : QObject(parent)
    , d(new ClientPrivate)
{
    d->mUrl = url;
}

Client::~Client()
{
    for (auto it = d->mPendingQueries.begin(), end = d->mPendingQueries.end(); it != end; ++it) {
        (*it)->deleteLater();
    }

    d->mPendingQueries.clear();

    delete d;
}

void Client::setUrl(const QUrl &url)
{
    d->mUrl = url.isValid() ? url : QUrl();
}

QUrl Client::url() const
{
    return d->mUrl;
}

QString Client::userAgent() const
{
    return d->mUserAgent;
}

void Client::setUserAgent(const QString &userAgent)
{
    d->mUserAgent = userAgent;
}

bool Client::isDigestAuthEnabled() const
{
    return d->mDigestAuth;
}

void Client::setDigestAuthEnabled(bool enabled)
{
    d->mDigestAuth = enabled;
}

void Client::call(const QString &method,
                  const QList<QVariant> &args,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QMap<QString, QString> metaData;

    if (d->mUrl.isEmpty()) {
        qCWarning(KXMLRPCCLIENT_LOG) << "Cannot execute call to" << method << ": empty server URL";
    }

    // Fill metadata, with userAgent and possible digest auth
    if (d->mUserAgent.isEmpty()) {
        metaData[QStringLiteral("UserAgent")] = QStringLiteral("KDE-XMLRPC");
    } else {
        metaData[QStringLiteral("UserAgent")] = d->mUserAgent;
    }

    if (d->mDigestAuth) {
        metaData[QStringLiteral("WWW-Authenticate:")] = QStringLiteral("Digest");
    }

    Query *query = Query::create(id, this);
    connect(query, SIGNAL(message(QList<QVariant>, QVariant)), msgObj, messageSlot);
    connect(query, SIGNAL(fault(int, QString, QVariant)), faultObj, faultSlot);
    connect(query, SIGNAL(finished(Query *)), this, SLOT(queryFinished(Query *)));
    d->mPendingQueries.append(query);

    query->call(d->mUrl, method, args, metaData);
}

void Client::call(const QString &method,
                  const QVariant &arg,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QList<QVariant> args;
    args << arg;
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method, int arg, QObject *msgObj, const char *messageSlot, QObject *faultObj, const char *faultSlot, const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method, bool arg, QObject *msgObj, const char *messageSlot, QObject *faultObj, const char *faultSlot, const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method, double arg, QObject *msgObj, const char *messageSlot, QObject *faultObj, const char *faultSlot, const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method,
                  const QString &arg,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method,
                  const QByteArray &arg,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method,
                  const QDateTime &arg,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QList<QVariant> args;
    args << QVariant(arg);
    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

void Client::call(const QString &method,
                  const QStringList &arg,
                  QObject *msgObj,
                  const char *messageSlot,
                  QObject *faultObj,
                  const char *faultSlot,
                  const QVariant &id)
{
    QList<QVariant> args;
    const int numArgs = arg.count();
    args.reserve(numArgs);
    for (int i = 0; i < numArgs; ++i) {
        args << QVariant(arg[i]);
    }

    call(method, args, msgObj, messageSlot, faultObj, faultSlot, id);
}

#include "moc_client.cpp"
