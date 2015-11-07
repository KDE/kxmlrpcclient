/******************************************************************************
 *   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>               *
 *                                Tobias Koenig <tokoe@kde.org>               *
 *   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING.BSD'.                        *
 *****************************************************************************/

#include "query.h"

#include <KIO/Job>

#include <QtCore/QUrl>
#include <QtCore/QDateTime>
#include <QtCore/QVariant>
#include <QtXml/QDomDocument>

namespace KXmlRpc
{

/**
  @brief
  Result is an internal class that represents a response
  from a XML-RPC server.

  This is an internal class and is only used by Query.
  @internal
 */
class Result
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

class QueryPrivate
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
