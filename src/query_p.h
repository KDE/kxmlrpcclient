/*
    SPDX-FileCopyrightText: 2003-2004 Frerich Raabe <raabe@kde.org>
    SPDX-FileCopyrightText: 2003-2004 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include "query.h"
#include "kxmlrpcclient_private_export.h"
#include <KIO/Job>

#include <QVariant>
#include <QDomDocument>

namespace KXmlRpc
{

/**
  @brief
  Result is an internal class that represents a response
  from a XML-RPC server.

  This is an internal class and is only used by Query.
  @internal
 */
class KXMLRPCCLIENT_TESTS_EXPORT Result
{
    friend class Query;
    friend class QueryPrivate;

public:
    /**
      Constructs a result.
     */
    Result();


    /**
      Returns true if the method call succeeded, false
      if there was an XML-RPC fault.

      @see errorCode(), errorString()
     */
    bool success() const;

    /**
      Returns the error code of the fault.

      @see success(), errorString()
     */
    int errorCode() const;

    /**
      Returns the error string that describes the fault.

      @see success, errorCode()
     */
    QString errorString() const;

    /**
      Returns the data sent to us from the server.
     */
    QList<QVariant> data() const;

private:
    bool mSuccess;
    int mErrorCode;
    QString mErrorString;
    QList<QVariant> mData;
};

class KXMLRPCCLIENT_TESTS_EXPORT QueryPrivate
{
public:
    QueryPrivate(Query *parent)
        : mParent(parent)
    {
    }

    static bool isMessageResponse(const QDomDocument &doc);
    static bool isFaultResponse(const QDomDocument &doc);

    static Result parseMessageResponse(const QDomDocument &doc);
    static Result parseFaultResponse(const QDomDocument &doc);

    static QByteArray markupCall(const QString &method, const QList<QVariant> &args);
    static QByteArray marshal(const QVariant &value);
    static QVariant demarshal(const QDomElement &element);

    void slotData(KIO::Job *job, const QByteArray &data);
    void slotResult(KJob *job);

    Query *mParent;
    QByteArray mBuffer;
    QVariant mId;
    QList<KJob *> mPendingJobs;
};

} // namespace KXmlRpcClient
