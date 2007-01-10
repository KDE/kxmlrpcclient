/**************************************************************************
*   Copyright (C) 2006 by Narayan Newton <narayannewton@gmail.com>        *
*   Copyright (C) 2003 - 2004 by Frerich Raabe <raabe@kde.org>            *
*                                Tobias Koenig <tokoe@kde.org>            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <QVariant>
#include <qdom.h>

#include "query.h"

using namespace KXmlRpc;

/**
  @file

  Implementation of Result and Query
**/

class Result::Private
{
  public:
    bool mSuccess;
    int mErrorCode;
    QString mErrorString;
    QList<QVariant> mData;
};

Result::Result()
  : d( new Private )
{
}

Result::Result( const Result &other )
  : d( new Private )
{
  *d = *other.d;
}

Result &Result::operator=( const Result &other )
{
  if ( this == &other ) {
    return *this;
  }

  *d = *other.d;

  return *this;
}

Result::~Result()
{
  delete d;
}

bool Result::success() const
{
  return d->mSuccess;
}

int Result::errorCode() const
{
  return d->mErrorCode;
}

QString Result::errorString() const
{
  return d->mErrorString;
}

QList<QVariant> Result::data() const
{
  return d->mData;
}

//small macro taken from HTTP IOSlave
#define KIO_ARGS QByteArray packedArgs; QDataStream kioArgsStream( &packedArgs, QIODevice::WriteOnly ); kioArgsStream

class Query::Private
{
  public:
    Private( Query *parent )
      : mParent( parent )
    {
    }

    bool isMessageResponse( const QDomDocument &doc ) const;
    bool isFaultResponse( const QDomDocument &doc ) const;

    Result parseMessageResponse( const QDomDocument &doc ) const;
    Result parseFaultResponse( const QDomDocument &doc ) const;

    QString markupCall( const QString &method, const QList<QVariant> &args ) const;
    QString marshal( const QVariant &value ) const;
    QVariant demarshal( const QDomElement &element ) const;

    void slotData( KIO::Job *job, const QByteArray &data );
    void slotResult( KIO::Job *job );

    Query *mParent;
    QByteArray mBuffer;
    QVariant mId;
    QList<KIO::Job*> mPendingJobs;
};

bool Query::Private::isMessageResponse( const QDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().toLower() == "params";
}

bool Query::Private::isFaultResponse( const QDomDocument &doc ) const
{
  return doc.documentElement().firstChild().toElement().tagName().toLower() == "fault";
}

Result Query::Private::parseMessageResponse( const QDomDocument &doc ) const
{
  Result response;
  response.d->mSuccess = true;

  QDomNode paramNode = doc.documentElement().firstChild().firstChild();
  while ( !paramNode.isNull() ) {
    response.d->mData << demarshal( paramNode.firstChild().toElement() );
    paramNode = paramNode.nextSibling();
  }

  return response;
}

Result Query::Private::parseFaultResponse( const QDomDocument &doc ) const
{
  Result response;
  response.d->mSuccess = false;

  QDomNode errorNode = doc.documentElement().firstChild().firstChild();
  const QVariant errorVariant = demarshal( errorNode.toElement() );
  response.d->mErrorCode = errorVariant.toMap() [ "faultCode" ].toInt();
  response.d->mErrorString = errorVariant.toMap() [ "faultString" ].toString();

  return response;
}

QString Query::Private::markupCall( const QString &cmd, const QList<QVariant> &args ) const
{
  QString markup = "<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n";

  markup += "<methodName>" + cmd + "</methodName>\r\n";

  if ( !args.isEmpty() ) {

    markup += "<params>\r\n";
    QList<QVariant>::ConstIterator it = args.begin();
    QList<QVariant>::ConstIterator end = args.end();
    for ( ; it != end; ++it ) {
      markup += "<param>\r\n" + marshal( *it ) + "</param>\r\n";
    }
    markup += "</params>\r\n";
  }

  markup += "</methodCall>\r\n";

  return markup;
}

QString Query::Private::marshal( const QVariant &arg ) const
{
  switch ( arg.type() ) {

    case QVariant::String:
      return "<value><string>" + arg.toString() + "</string></value>\r\n";
    case QVariant::Int:
      return "<value><int>" + QString::number( arg.toInt() ) + "</int></value>\r\n";
    case QVariant::Double:
      return "<value><double>" + QString::number( arg.toDouble() ) + "</double></value>\r\n";
    case QVariant::Bool:
      {
        QString markup = "<value><boolean>";
        markup += arg.toBool() ? "1" : "0";
        markup += "</boolean></value>\r\n";
        return markup;
      }
    case QVariant::ByteArray:
      return "<value><base64>" + arg.toByteArray().toBase64() + "</base64></value>\r\n";
    case QVariant::DateTime:
      return "<value><datetime.iso8601>" + arg.toDateTime().toString( Qt::ISODate ) + "</datetime.iso8601></value>\r\n";
    case QVariant::List:
      {
        QString markup = "<value><array><data>\r\n";
        const QList<QVariant> args = arg.toList();
        QList<QVariant>::ConstIterator it = args.begin();
        QList<QVariant>::ConstIterator end = args.end();
        for ( ; it != end; ++it ) {
          markup += marshal( *it );
        }
        markup += "</data></array></value>\r\n";
        return markup;
      }
    case QVariant::Map:
      {
        QString markup = "<value><struct>\r\n";
        QMap<QString, QVariant> map = arg.toMap();
        QMap<QString, QVariant>::ConstIterator it = map.begin();
        QMap<QString, QVariant>::ConstIterator end = map.end();
        for ( ; it != end; ++it ) {
          markup += "<member>\r\n";
          markup += "<name>" + it.key() + "</name>\r\n";
          markup += marshal( it.value() );
          markup += "</member>\r\n";
        }
        markup += "</struct></value>\r\n";
        return markup;
      }
    default:
      kWarning() << "Failed to marshal unknown variant type: " << arg.type() << endl;
  };

  return QString();
}

QVariant Query::Private::demarshal( const QDomElement &element ) const
{
  Q_ASSERT( element.tagName().toLower() == "value" );

  const QDomElement typeElement = element.firstChild().toElement();
  const QString typeName = typeElement.tagName().toLower();

  if ( typeName == "string" ) {
    return QVariant( typeElement.text() );
  } else if ( typeName == "i4" || typeName == "int" ) {
    return QVariant( typeElement.text().toInt() );
  } else if ( typeName == "double" ) {
    return QVariant( typeElement.text().toDouble() );
  } else if ( typeName == "boolean" ) {

    if ( typeElement.text().toLower() == "true" || typeElement.text() == "1" ) {
      return QVariant( true );
    } else {
      return QVariant( false );
    }
  } else if ( typeName == "base64" ) {
    return QVariant( QByteArray::fromBase64( typeElement.text().toLatin1() ) );
  } else if ( typeName == "datetime" || typeName == "datetime.iso8601" ) {
    return QVariant( QDateTime::fromString( typeElement.text(), Qt::ISODate ) );
  } else if ( typeName == "array" ) {
    QList<QVariant> values;
    QDomNode valueNode = typeElement.firstChild().firstChild();
    while ( !valueNode.isNull() ) {
      values << demarshal( valueNode.toElement() );
      valueNode = valueNode.nextSibling();
    }
    return QVariant( values );
  } else if ( typeName == "struct" ) {

    QMap<QString, QVariant> map;
    QDomNode memberNode = typeElement.firstChild();
    while ( !memberNode.isNull() ) {
      const QString key = memberNode.toElement().elementsByTagName( "name" ).item( 0 ).toElement().text();
      const QVariant data = demarshal( memberNode.toElement().elementsByTagName( "value" ).item( 0 ).toElement() );
      map[ key ] = data;
      memberNode = memberNode.nextSibling();
    }
    return QVariant( map );
  } else {
    kWarning() << "Cannot demarshal unknown type " << typeName << endl;
  }
  return QVariant();
}

void Query::Private::slotData( KIO::Job *, const QByteArray &data )
{
  unsigned int oldSize = mBuffer.size();
  mBuffer.resize( oldSize + data.size() );
  memcpy( mBuffer.data() + oldSize, data.data(), data.size() );
}

void Query::Private::slotResult( KIO::Job *job )
{
  mPendingJobs.removeAll( job );

  if ( job->error() != 0 ) {
    emit mParent->fault( job->error(), job->errorString(), mId );
    emit mParent->finished( mParent );
    return;
  }

  const QString data = QString::fromUtf8( mBuffer.data(), mBuffer.size() );

  QDomDocument doc;
  QString errMsg;
  int errLine, errCol;
  if ( !doc.setContent( data, false, &errMsg, &errLine, &errCol ) ) {
    emit mParent->fault( -1, i18n( "Received invalid XML markup: %1 at %2:%3" )
                       .arg( errMsg ).arg( errLine ).arg( errCol ), mId );
    emit mParent->finished( mParent );
    return;
  }

  mBuffer.truncate( 0 );

  if ( isMessageResponse( doc ) ) {
    emit mParent->message( parseMessageResponse( doc ).data(), mId );
  } else if ( isFaultResponse( doc ) ) {
    emit mParent->fault( parseFaultResponse( doc ).errorCode(), parseFaultResponse( doc ).errorString(), mId );
  } else {
    emit mParent->fault( 1, i18n( "Unknown type of XML markup received" ), mId );
  }

  emit mParent->finished( mParent );
}

Query *Query::create( const QVariant &id, QObject *parent )
{
  return new Query( id, parent );
}

void Query::call( const QString &server,
                  const QString &method,
                  const QList<QVariant> &args,
                  const QMap<QString, QString> &jobMetaData )
{
  const QString xmlMarkup = d->markupCall( method, args );
  QMap<QString, QString>::const_iterator mapIter;
  QByteArray postData;
  QDataStream stream( &postData, QIODevice::WriteOnly );
  stream.writeRawData( xmlMarkup.toUtf8(), xmlMarkup.toUtf8().length() );

  KIO_ARGS << (int)1 << KUrl( server );
  KIO::TransferJob *job = new KIO::TransferJob( KUrl( server ), KIO::CMD_SPECIAL, packedArgs, postData, false );

  if ( !job ) {
    kWarning() << "Unable to create KIO job for " << server << endl;
    return;
  }

  job->addMetaData( "content-type", "Content-Type: text/xml; charset=utf-8" );
  job->addMetaData( "ConnectTimeout", "50" );

  for (mapIter = jobMetaData.begin(); mapIter != jobMetaData.end(); mapIter++) {
    job->addMetaData( mapIter.key(), mapIter.value() );
  }

  connect( job, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
           this, SLOT( slotData( KIO::Job *, const QByteArray & ) ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotResult( KIO::Job * ) ) );

  d->mPendingJobs.append( job );
}

Query::Query( const QVariant &id, QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->mId = id;
}

Query::~Query()
{
  QList<KIO::Job*>::Iterator it;
  for ( it = d->mPendingJobs.begin(); it != d->mPendingJobs.end(); ++it ) {
    (*it)->kill();
  }
}

#include "query.moc"

