/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "collectionfetchjob.h"

#include "imapparser_p.h"
#include "job_p.h"
#include "protocol_p.h"
#include "protocolhelper_p.h"
#include "entity_p.h"
#include "collectionfetchscope.h"
#include "collectionutils_p.h"

#include <kdebug.h>

#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

using namespace Akonadi;

class Akonadi::CollectionFetchJobPrivate : public JobPrivate
{
  public:
    CollectionFetchJobPrivate( CollectionFetchJob *parent )
      : JobPrivate( parent )
    {
    }

    Q_DECLARE_PUBLIC( CollectionFetchJob )

    CollectionFetchJob::Type mType;
    Collection mBase;
    Collection::List mBaseList;
    Collection::List mCollections;
    CollectionFetchScope mScope;
    Collection::List mPendingCollections;
    QTimer *mEmitTimer;

    void timeout()
    {
      Q_Q( CollectionFetchJob );

      mEmitTimer->stop(); // in case we are called by result()
      if ( !mPendingCollections.isEmpty() ) {
        emit q->collectionsReceived( mPendingCollections );
        mPendingCollections.clear();
      }
    }
};

CollectionFetchJob::CollectionFetchJob( const Collection &collection, Type type, QObject *parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );

  d->mBase = collection;
  d->mType = type;

  d->mEmitTimer = new QTimer( this );
  d->mEmitTimer->setSingleShot( true );
  d->mEmitTimer->setInterval( 100 );
  connect( d->mEmitTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
  connect( this, SIGNAL(result(KJob*)), this, SLOT(timeout()) );
}

CollectionFetchJob::CollectionFetchJob( const Collection::List & cols, QObject * parent )
  : Job( new CollectionFetchJobPrivate( this ), parent )
{
  Q_D( CollectionFetchJob );

  Q_ASSERT( !cols.isEmpty() );
  if ( cols.size() == 1 ) {
    d->mBase = cols.first();
    d->mType = CollectionFetchJob::Base;
  } else {
    d->mBaseList = cols;
  }

  d->mEmitTimer = new QTimer( this );
  d->mEmitTimer->setSingleShot( true );
  d->mEmitTimer->setInterval( 100 );
  connect( d->mEmitTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
  connect( this, SIGNAL(result(KJob*)), this, SLOT(timeout()) );
}

CollectionFetchJob::~CollectionFetchJob()
{
}

Collection::List CollectionFetchJob::collections() const
{
  Q_D( const CollectionFetchJob );

  return d->mCollections;
}

void CollectionFetchJob::doStart()
{
  Q_D( CollectionFetchJob );

  if ( !d->mBaseList.isEmpty() ) {
    foreach ( const Collection &col, d->mBaseList ) {
      CollectionFetchJob *subJob = new CollectionFetchJob( col, CollectionFetchJob::Base, this );
      subJob->setFetchScope( fetchScope() );
    }
    return;
  }

  if ( !d->mBase.isValid() && d->mBase.remoteId().isEmpty() ) {
    setError( Unknown );
    setErrorText( QLatin1String( "Invalid collection given." ) );
    emitResult();
    return;
  }

  QByteArray command = d->newTag();
  if ( !d->mBase.isValid() ) {
    if ( CollectionUtils::hasValidHierarchicalRID( d->mBase ) )
      command += " HRID";
    else
      command += " " AKONADI_CMD_RID;
  }
  if ( d->mScope.includeUnubscribed() )
    command += " LIST ";
  else
    command += " LSUB ";

  if ( d->mBase.isValid() )
    command += QByteArray::number( d->mBase.id() );
  else if ( CollectionUtils::hasValidHierarchicalRID( d->mBase ) )
    command += '(' + ProtocolHelper::hierarchicalRidToByteArray( d->mBase ) + ')';
  else
    command += ImapParser::quote( d->mBase.remoteId().toUtf8() );

  command += ' ';
  switch ( d->mType ) {
    case Base:
      command += "0 (";
      break;
    case FirstLevel:
      command += "1 (";
      break;
    case Recursive:
      command += "INF (";
      break;
    default:
      Q_ASSERT( false );
  }

  QList<QByteArray> filter;
  if ( !d->mScope.resource().isEmpty() ) {
    filter.append( "RESOURCE" );
    filter.append( d->mScope.resource().toUtf8() );
  }

  if ( !d->mScope.contentMimeTypes().isEmpty() ) {
    filter.append( "MIMETYPE" );
    QList<QByteArray> mts;
    foreach ( const QString &mt, d->mScope.contentMimeTypes() )
      mts.append( mt.toUtf8() );
    filter.append( '(' + ImapParser::join( mts, " " ) + ')' );
  }

  QList<QByteArray> options;
  if ( d->mScope.includeStatistics() ) {
    options.append( "STATISTICS" );
    options.append( "true" );
  }
  if ( d->mScope.ancestorRetrieval() != CollectionFetchScope::None ) {
    options.append( "ANCESTORS" );
    switch ( d->mScope.ancestorRetrieval() ) {
      case CollectionFetchScope::None:
        options.append( "0" );
        break;
      case CollectionFetchScope::Parent:
        options.append( "1" );
        break;
      case CollectionFetchScope::All:
        options.append( "INF" );
        break;
      default:
        Q_ASSERT( false );
    }
  }

  command += ImapParser::join( filter, " " ) + ") (" + ImapParser::join( options, " " ) + ")\n";
  d->writeData( command );
}

void CollectionFetchJob::doHandleResponse( const QByteArray & tag, const QByteArray & data )
{
  Q_D( CollectionFetchJob );

  if ( tag == "*" ) {
    Collection collection;
    ProtocolHelper::parseCollection( data, collection );
    if ( !collection.isValid() )
      return;

    collection.d_ptr->resetChangeLog();
    d->mCollections.append( collection );
    d->mPendingCollections.append( collection );
    if ( !d->mEmitTimer->isActive() )
      d->mEmitTimer->start();
    return;
  }
  kDebug() << "Unhandled server response" << tag << data;
}

void CollectionFetchJob::setResource(const QString & resource)
{
  Q_D( CollectionFetchJob );

  d->mScope.setResource( resource );
}

void CollectionFetchJob::slotResult(KJob * job)
{
  Q_D( CollectionFetchJob );

  CollectionFetchJob *list = dynamic_cast<CollectionFetchJob*>( job );
  Q_ASSERT( job );
  d->mCollections += list->collections();
  Job::slotResult( job );
  if ( !job->error() && !hasSubjobs() )
    emitResult();
}

void CollectionFetchJob::includeUnsubscribed(bool include)
{
  Q_D( CollectionFetchJob );

  d->mScope.setIncludeUnsubscribed( include );
}

void CollectionFetchJob::includeStatistics(bool include)
{
  Q_D( CollectionFetchJob );

  d->mScope.setIncludeStatistics( include );
}

void CollectionFetchJob::setFetchScope( const CollectionFetchScope &scope )
{
  Q_D( CollectionFetchJob );
  d->mScope = scope;
}

CollectionFetchScope& CollectionFetchJob::fetchScope()
{
  Q_D( CollectionFetchJob );
  return d->mScope;
}

#include "collectionfetchjob.moc"
