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
/**
  @file

  This file is part of KXmlRpc and defines our internal classes.

  @author Frerich Raabe <raabe@kde.org>
  @author Tobias Koenig <tokoe@kde.org>
  @author Narayan Newton <narayannewton@gmail.com>
*/

#include "query.h"
#include "query_p.h"
#include "kxmlrpcclient_debug.h"

#include <KIO/Job>
#include <klocalizedstring.h>

using namespace KXmlRpc;

/**
  @file

  Implementation of Query
**/

KXmlRpc::Result::Result()
    : mSuccess(false)
    , mErrorCode(-1)
{
}

KXmlRpc::Result::~Result()
{
}

bool KXmlRpc::Result::success() const
{
    return mSuccess;
}

int KXmlRpc::Result::errorCode() const
{
    return mErrorCode;
}

QString KXmlRpc::Result::errorString() const
{
    return mErrorString;
}

QList<QVariant> KXmlRpc::Result::data() const
{
    return mData;
}

bool QueryPrivate::isMessageResponse(const QDomDocument &doc)
{
    return doc.documentElement().firstChild().toElement().tagName().toLower()
           == QLatin1String("params");
}

bool QueryPrivate::isFaultResponse(const QDomDocument &doc)
{
    return doc.documentElement().firstChild().toElement().tagName().toLower()
           == QLatin1String("fault");
}

Result QueryPrivate::parseMessageResponse(const QDomDocument &doc)
{
    Result response;
    response.mSuccess = true;

    QDomNode paramNode = doc.documentElement().firstChild().firstChild();
    while (!paramNode.isNull()) {
        response.mData << demarshal(paramNode.firstChild().toElement());
        paramNode = paramNode.nextSibling();
    }

    return response;
}

Result QueryPrivate::parseFaultResponse(const QDomDocument &doc)
{
    Result response;
    response.mSuccess = false;

    QDomNode errorNode = doc.documentElement().firstChild().firstChild();
    const QVariant errorVariant = demarshal(errorNode.toElement());
    response.mErrorCode = errorVariant.toMap()[QLatin1String("faultCode")].toInt();
    response.mErrorString = errorVariant.toMap()[QLatin1String("faultString")].toString();

    return response;
}

QByteArray QueryPrivate::markupCall(const QString &cmd,
                                      const QList<QVariant> &args)
{
    QByteArray markup = "<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n";

    markup += "<methodName>" + cmd.toLatin1() + "</methodName>\r\n";

    if (!args.isEmpty()) {

        markup += "<params>\r\n";
        for (auto it = args.constBegin(), end = args.constEnd(); it != end; ++it) {
            markup += "<param>\r\n" + marshal(*it) + "</param>\r\n";
        }
        markup += "</params>\r\n";
    }

    markup += "</methodCall>\r\n";

    return markup;
}

QByteArray QueryPrivate::marshal(const QVariant &arg)
{
    switch (arg.type()) {

    case QVariant::String:
        return "<value><string><![CDATA[" + arg.toString().toUtf8() + "]]></string></value>\r\n";
    case QVariant::StringList: {
        QStringList data = arg.toStringList();
        QStringListIterator dataIterator(data);
        QByteArray markup;
        markup += "<value><array><data>";
        while (dataIterator.hasNext()) {
            markup += "<value><string><![CDATA[" + dataIterator.next().toUtf8() + "]]></string></value>\r\n";
        }
        markup += "</data></array></value>";
        return markup;
    }
    case QVariant::Int:
        return "<value><int>" + QByteArray::number(arg.toInt()) + "</int></value>\r\n";
    case QVariant::Double:
        return "<value><double>" + QByteArray::number(arg.toDouble()) + "</double></value>\r\n";
    case QVariant::Bool:
        return "<value><boolean>" + QByteArray(arg.toBool() ? "1" : "0") + "</boolean></value>\r\n";
    case QVariant::ByteArray:
        return "<value><base64>" + arg.toByteArray().toBase64() + "</base64></value>\r\n";
    case QVariant::DateTime:
        return "<value><dateTime.iso8601>" + arg.toDateTime().toString(Qt::ISODate).toLatin1() + "</dateTime.iso8601></value>\r\n";
    case QVariant::List: {
        QByteArray markup = "<value><array><data>\r\n";
        const QList<QVariant> args = arg.toList();
        QList<QVariant>::ConstIterator it = args.begin();
        QList<QVariant>::ConstIterator end = args.end();
        for (; it != end; ++it) {
            markup += marshal(*it);
        }
        markup += "</data></array></value>\r\n";
        return markup;
    }
    case QVariant::Map: {
        QByteArray markup = "<value><struct>\r\n";
        const QMap<QString, QVariant> map = arg.toMap();
        for (auto it = map.constBegin(), end = map.constEnd(); it != end; ++it) {
            markup += "<member>\r\n";
            markup += "<name>" + it.key().toUtf8() + "</name>\r\n";
            markup += marshal(it.value());
            markup += "</member>\r\n";
        }
        markup += "</struct></value>\r\n";
        return markup;
    }
    default:
        qCWarning(KXMLRPCCLIENT_LOG) << "Failed to marshal unknown variant type:" << arg.type();
    };

    return QByteArray();
}

QVariant QueryPrivate::demarshal(const QDomElement &element)
{
    Q_ASSERT(element.tagName().toLower() == QLatin1String("value"));

    const QDomElement typeElement = element.firstChild().toElement();
    const QString typeName = typeElement.tagName().toLower();

    if (typeName == QLatin1String("string")) {
        return QVariant(typeElement.text());
    } else if (typeName == QLatin1String("i4") || typeName == QLatin1String("int")) {
        return QVariant(typeElement.text().toInt());
    } else if (typeName == QLatin1String("double")) {
        return QVariant(typeElement.text().toDouble());
    } else if (typeName == QLatin1String("boolean")) {

        if (typeElement.text().toLower() == QLatin1String("true") || typeElement.text() == QLatin1String("1")) {
            return QVariant(true);
        } else {
            return QVariant(false);
        }
    } else if (typeName == QLatin1String("base64")) {
        return QVariant(QByteArray::fromBase64(typeElement.text().toLatin1()));
    } else if (typeName == QLatin1String("datetime") || typeName == QLatin1String("datetime.iso8601")) {
        QDateTime date;
        QString dateText = typeElement.text();
        // Test for broken use of Basic ISO8601 date and extended ISO8601 time
        if (17 <= dateText.length() && dateText.length() <= 18 &&
                dateText.at(4) !=  QLatin1Char('-') && dateText.at(11) ==  QLatin1Char(':')) {
            if (dateText.endsWith(QLatin1Char('Z'))) {
                date = QDateTime::fromString(dateText, QLatin1String("yyyyMMddTHH:mm:ssZ"));
            } else {
                date = QDateTime::fromString(dateText, QLatin1String("yyyyMMddTHH:mm:ss"));
            }
        } else {
            date = QDateTime::fromString(dateText, Qt::ISODate);
        }
        return QVariant(date);
    } else if (typeName == QLatin1String("array")) {
        QList<QVariant> values;
        QDomNode valueNode = typeElement.firstChild().firstChild();
        while (!valueNode.isNull()) {
            values << demarshal(valueNode.toElement());
            valueNode = valueNode.nextSibling();
        }
        return QVariant(values);
    } else if (typeName == QLatin1String("struct")) {

        QMap<QString, QVariant> map;
        QDomNode memberNode = typeElement.firstChild();
        while (!memberNode.isNull()) {
            const QString key = memberNode.toElement().elementsByTagName(
                                    QLatin1String("name")).item(0).toElement().text();
            const QVariant data = demarshal(memberNode.toElement().elementsByTagName(
                                                QLatin1String("value")).item(0).toElement());
            map[key] = data;
            memberNode = memberNode.nextSibling();
        }
        return QVariant(map);
    } else {
        qCWarning(KXMLRPCCLIENT_LOG) << "Cannot demarshal unknown type" << typeName;
    }
    return QVariant();
}

void QueryPrivate::slotData(KIO::Job *, const QByteArray &data)
{
    unsigned int oldSize = mBuffer.size();
    mBuffer.resize(oldSize + data.size());
    memcpy(mBuffer.data() + oldSize, data.data(), data.size());
}

void QueryPrivate::slotResult(KJob *job)
{
    mPendingJobs.removeAll(job);

    if (job->error() != 0) {
        emit mParent->fault(job->error(), job->errorString(), mId);
        emit mParent->finished(mParent);
        return;
    }

    QDomDocument doc;
    QString errMsg;
    int errLine, errCol;
    if (!doc.setContent(mBuffer, false, &errMsg, &errLine, &errCol)) {
        emit mParent->fault(-1, i18n("Received invalid XML markup: %1 at %2:%3",
                                     errMsg, errLine, errCol), mId);
        emit mParent->finished(mParent);
        return;
    }

    mBuffer.truncate(0);

    if (isMessageResponse(doc)) {
        emit mParent->message(parseMessageResponse(doc).data(), mId);
    } else if (isFaultResponse(doc)) {
        const Result fault = parseFaultResponse(doc);
        emit mParent->fault(fault.errorCode(), fault.errorString(), mId);
    } else {
        emit mParent->fault(1, i18n("Unknown type of XML markup received"),
                            mId);
    }

    emit mParent->finished(mParent);
}

Query *Query::create(const QVariant &id, QObject *parent)
{
    return new Query(id, parent);
}

void Query::call(const QUrl &server,
                 const QString &method,
                 const QList<QVariant> &args,
                 const QMap<QString, QString> &jobMetaData)
{

    const QByteArray xmlMarkup = d->markupCall(method, args);
    KIO::TransferJob *job = KIO::http_post(server, xmlMarkup, KIO::HideProgressInfo);

    if (!job) {
        qCWarning(KXMLRPCCLIENT_LOG) << "Unable to create KIO job for" << server.url();
        return;
    }

    job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: text/xml; charset=utf-8"));
    job->addMetaData(QLatin1String("ConnectTimeout"), QLatin1String("50"));

    for (auto it = jobMetaData.begin(), end = jobMetaData.end(); it != end; ++it) {
        job->addMetaData(it.key(), it.value());
    }

    connect(job, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(slotData(KIO::Job*,QByteArray)));
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotResult(KJob*)));

    d->mPendingJobs.append(job);
}

Query::Query(const QVariant &id, QObject *parent)
    : QObject(parent), d(new QueryPrivate(this))
{
    d->mId = id;
}

Query::~Query()
{
    for (auto it = d->mPendingJobs.begin(), end = d->mPendingJobs.end(); it != end; ++it) {
        (*it)->kill();
    }
    delete d;
}

#include "moc_query.cpp"

