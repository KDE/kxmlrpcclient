/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 - 2010 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "standardmailactionmanager.h"
#include "movetotrashcommand_p.h"
#include "markascommand_p.h"
#include "emptytrashcommand_p.h"
#include "removeduplicatescommand_p.h"

#include <akonadi/subscriptiondialog_p.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttypedialog.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/mimetypechecker.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/kmime/messagestatus.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QPointer>
#include <QtGui/QItemSelectionModel>

using namespace Akonadi;

class StandardMailActionManager::Private
{
  public:
    Private( KActionCollection *actionCollection, QWidget *parentWidget, StandardMailActionManager *parent )
      : mActionCollection( actionCollection ), mParentWidget( parentWidget ),
        mCollectionSelectionModel( 0 ), mItemSelectionModel( 0 ), mParent( parent )
    {
      mGenericManager = new StandardActionManager( actionCollection, parentWidget );
      mParent->connect( mGenericManager, SIGNAL( actionStateUpdated() ),
                        mParent, SIGNAL( actionStateUpdated() ) );
      mGenericManager->createAllActions();

      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Folder..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setWhatsThis( i18n( "Add a new folder to the currently selected account." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Folder", "Copy %1 Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CopyCollections )->setWhatsThis( i18n( "Copy the selected folders to the clipboard." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete Folder", "Delete %1 Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteCollections )->setWhatsThis( i18n( "Delete the selected folders from the account." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Update Folder", "Update Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setWhatsThis( i18n( "Update the content of the selected folders." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CutCollections, ki18np( "Cut Folder", "Cut %1 Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CutCollections )->setWhatsThis( i18n( "Cut the selected folders from the account." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Folder Properties..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CollectionProperties)->setWhatsThis( i18n( "Open a dialog to edit the properties of the selected folder." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems, ki18np( "Copy Email", "Copy %1 Emails" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CopyItems )->setWhatsThis( i18n( "Copy the selected emails to the clipboard." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete Email", "Delete %1 Emails" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteItems )->setWhatsThis( i18n( "Delete the selected emails from the folder." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CutItems, ki18np( "Cut Email", "Cut %1 Emails" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CutItems )->setWhatsThis( i18n( "Cut the selected emails from the folder." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateResource )->setText( i18n( "Add &Account..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateResource )->setWhatsThis( i18n( "Add a new account<p>You will be presented with a dialog where you can select the type of the account that shall be added.</p>" ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteResources, ki18np( "&Delete Account", "&Delete %1 Accounts" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteResources )->setWhatsThis( i18n( "Delete the selected accounts<p>The currently selected accounts will be deleted, along with all the emails they contain.</p>" ) );
      mGenericManager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Account Properties..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::ResourceProperties )->setWhatsThis( i18n( "Open a dialog to edit properties of the selected account." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Update Account", "Update %1 Accounts" ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeResources )->setWhatsThis( i18n( "Updates the content of all folders of the selected accounts." ) );

      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "New Folder" ) );
      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not create folder: %1" ) );
      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Folder creation failed" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxText,
                                       ki18np( "Do you really want to delete folder '%2' and all its sub-folders?",
                                               "Do you really want to delete %1 folders and all their sub-folders?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxTitle,
                                       ki18ncp( "@title:window", "Delete folder?", "Delete folders?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not delete folder: %1" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Folder deletion failed" ) );

      mGenericManager->setContextText( StandardActionManager::CollectionProperties, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "Properties of Folder %1" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::MessageBoxText,
                                       ki18np( "Do you really want to delete the selected email?", "Do you really want to delete %1 emails?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::MessageBoxTitle,
                                       ki18ncp( "@title:window", "Delete Email?", "Delete Emails?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not delete email: %1" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Email deletion failed" ) );

      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "Add Account" ) );
      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not create account: %1" ) );
      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Account creation failed" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteResources, StandardActionManager::MessageBoxText,
                                       ki18np( "Do you really want to delete account '%2'?", "Do you really want to delete %1 accounts?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteResources, StandardActionManager::MessageBoxTitle,
                                       ki18ncp( "@title:window", "Delete Account?", "Delete Accounts?" ) );

      mGenericManager->setContextText( StandardActionManager::Paste, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not paste email: %1" ) );
      mGenericManager->setContextText( StandardActionManager::Paste, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Paste failed" ) );

      mGenericManager->setMimeTypeFilter( QStringList() << QLatin1String( "message/rfc822" ) );
      mGenericManager->setCapabilityFilter( QStringList() << QLatin1String( "Resource" ) );
      mGenericManager->interceptAction( Akonadi::StandardActionManager::ManageLocalSubscriptions );
      connect( mGenericManager->action( StandardActionManager::ManageLocalSubscriptions ), SIGNAL( triggered( bool ) ), mParent, SLOT( slotMailLocalSubscription() ) );
    }

    ~Private()
    {
      delete mGenericManager;
    }

    static bool hasWritableCollection( const QModelIndex &index, const QString &mimeType )
    {
      const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() ) {
        if ( collection.contentMimeTypes().contains( mimeType ) && (collection.rights() & Akonadi::Collection::CanCreateItem) )
          return true;
      }

      const QAbstractItemModel *model = index.model();
      if ( !model )
        return false;

      for ( int row = 0; row < model->rowCount( index ); ++row ) {
        if ( hasWritableCollection( model->index( row, 0, index ), mimeType ) )
          return true;
      }

      return false;
    }

    bool hasWritableCollection( const QString &mimeType ) const
    {
      if ( !mCollectionSelectionModel )
        return false;

      const QAbstractItemModel *collectionModel = mCollectionSelectionModel->model();
      for ( int row = 0; row < collectionModel->rowCount(); ++row ) {
        if ( hasWritableCollection( collectionModel->index( row, 0, QModelIndex() ), mimeType ) )
          return true;
      }

      return false;
    }

    void updateActions()
    {
      //TODO
      emit mParent->actionStateUpdated();
    }

    Collection selectedCollection() const
    {
      if ( !mCollectionSelectionModel )
        return Collection();

      if ( mCollectionSelectionModel->selectedIndexes().isEmpty() )
        return Collection();

      const QModelIndex index = mCollectionSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return Collection();

      return index.data( EntityTreeModel::CollectionRole).value<Collection>();
    }

    AgentInstance selectedAgentInstance() const
    {
      const Collection collection = selectedCollection();
      if ( !collection.isValid() )
        return AgentInstance();

      const QString identifier = collection.resource();

      return AgentManager::self()->instance( identifier );
    }

    void slotMarkAs()
    {
      QAction *action = dynamic_cast<QAction*>( mParent->sender() );
      QByteArray typeStr = action->data().toByteArray();
      kDebug() << "Mark mail as: " << typeStr;

      StandardMailActionManager::Type type = MarkMailAsRead;
      if ( typeStr == "U" )
        type = MarkMailAsUnread;
      else if ( typeStr == "K" )
        type = MarkMailAsActionItem;
      else if ( typeStr == "G" )
        type = MarkMailAsImportant;

      if ( mInterceptedActions.contains( type ) )
        return;

      if ( mItemSelectionModel->selection().indexes().isEmpty() )
        return;

      const QModelIndex index = mItemSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return;

      const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( !item.isValid() )
        return;


      Akonadi::MessageStatus targetStatus;
      targetStatus.setStatusFromStr( QLatin1String( typeStr ) );

      MarkAsCommand *command = new MarkAsCommand( targetStatus, Akonadi::Item::List() << item, mParent );
      command->execute();
    }

    void slotMarkAllAs()
    {
      QAction *action = dynamic_cast<QAction*>( mParent->sender() );
      Q_ASSERT(action);

      QByteArray typeStr = action->data().toByteArray();
      kDebug() << "Mark all as: " << typeStr;

      StandardMailActionManager::Type type = MarkAllMailAsRead;
      if ( typeStr == "U" )
        type = MarkAllMailAsUnread;
      else if ( typeStr == "K" )
        type = MarkAllMailAsActionItem;
      else if ( typeStr == "G" )
        type = MarkAllMailAsImportant;

      if ( mInterceptedActions.contains( type ) )
        return;

      if ( mCollectionSelectionModel->selection().indexes().isEmpty() )
        return;


      const QModelIndex index = mCollectionSelectionModel->selection().indexes().at( 0 );
      Q_ASSERT( index.isValid() );
      const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
      Q_ASSERT( collection.isValid() );

      Akonadi::MessageStatus targetStatus;
      targetStatus.setStatusFromStr( QLatin1String( typeStr ) );

      MarkAsCommand *command = new MarkAsCommand( targetStatus, collection, mParent );
      command->execute();
    }

    void slotMoveToTrash()
    {
      QAction *action = dynamic_cast<QAction*>( mParent->sender() );
      qDebug() << Q_FUNC_INFO << action->data();
    }

    void slotMoveAllToTrash()
    {
      if ( mInterceptedActions.contains( StandardMailActionManager::MoveAllToTrash ) )
        return;

      if ( mCollectionSelectionModel->selection().indexes().isEmpty() )
        return;

      const QModelIndex index = mCollectionSelectionModel->selection().indexes().at( 0 );
      Q_ASSERT( index.isValid() );
      const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
      Q_ASSERT( collection.isValid() );

      MoveToTrashCommand *command = new MoveToTrashCommand( const_cast<QAbstractItemModel*>( mCollectionSelectionModel->model() ), collection, mParent );
      command->execute();
    }

    void slotRemoveDuplicates()
    {
      if ( mInterceptedActions.contains( StandardMailActionManager::RemoveDuplicates ) )
        return;

      if ( mCollectionSelectionModel->selection().indexes().isEmpty() )
        return;

      const QModelIndex index = mCollectionSelectionModel->selection().indexes().at( 0 );
      Q_ASSERT( index.isValid() );
      const Collection collection = index.data( CollectionModel::CollectionRole ).value<Collection>();
      Q_ASSERT( collection.isValid() );

      RemoveDuplicatesCommand *command = new RemoveDuplicatesCommand( const_cast<QAbstractItemModel*>( mCollectionSelectionModel->model() ), collection, mParent );
      command->execute();
    }

    void slotEmptyAllTrash()
    {
      if ( mInterceptedActions.contains( StandardMailActionManager::EmptyAllTrash ) )
        return;

      EmptyTrashCommand *command = new EmptyTrashCommand( const_cast<QAbstractItemModel*>( mCollectionSelectionModel->model() ), mParent );
      command->execute();
    }

    void slotMailLocalSubscription()
    {
#ifndef Q_OS_WINCE
      SubscriptionDialog* dlg = new SubscriptionDialog( QLatin1String( "message/rfc822" ), mParentWidget );
      dlg->show();
#endif

    }

    KActionCollection *mActionCollection;
    QWidget *mParentWidget;
    StandardActionManager *mGenericManager;
    QItemSelectionModel *mCollectionSelectionModel;
    QItemSelectionModel *mItemSelectionModel;
    QHash<StandardMailActionManager::Type, KAction*> mActions;
    QSet<StandardMailActionManager::Type> mInterceptedActions;
    StandardMailActionManager *mParent;
};

StandardMailActionManager::StandardMailActionManager( KActionCollection *actionCollection, QWidget *parent )
  : QObject( parent ), d( new Private( actionCollection, parent, this ) )
{
}

StandardMailActionManager::~StandardMailActionManager()
{
  delete d;
}

void StandardMailActionManager::setCollectionSelectionModel( QItemSelectionModel *selectionModel )
{
  d->mCollectionSelectionModel = selectionModel;
  d->mGenericManager->setCollectionSelectionModel( selectionModel );

  connect( selectionModel->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           SLOT( updateActions() ) );
  connect( selectionModel->model(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
           SLOT( updateActions() ) );
  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );

  d->updateActions();
}

void StandardMailActionManager::setItemSelectionModel( QItemSelectionModel* selectionModel )
{
  d->mItemSelectionModel = selectionModel;
  d->mGenericManager->setItemSelectionModel( selectionModel );

  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );

  d->updateActions();
}

KAction* StandardMailActionManager::createAction( Type type )
{
  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );

  KAction *action = 0;

  switch ( type ) {
    case MarkMailAsRead:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "mail-mark-read" ) ) );
      action->setText( i18n( "&Mark Mail as Read" ) );
      action->setWhatsThis( i18n( "Mark selected messages as read" ) );
      d->mActions.insert( MarkMailAsRead, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_as_read" ), action );
      action->setData( QByteArray("R") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAs() ) );
      break;
    case MarkMailAsUnread:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "&Mark Mail as Unread" ) );
      action->setIcon( KIcon( QLatin1String( "mail-mark-unread" ) ) );
      d->mActions.insert( MarkMailAsUnread, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_as_unread" ), action );
      action->setData( QByteArray("U") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAs() ) );
      break;
    case MarkMailAsImportant:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "mail-mark-important" ) ) );
      action->setText( i18n( "&Mark Mail as Important" ) );
      d->mActions.insert( MarkMailAsImportant, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_as_important" ), action );
      action->setData( QByteArray("G") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAs() ) );
      break;
    case MarkMailAsActionItem:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "&Mark Mail as Action Item" ) );
      action->setIcon( KIcon( QLatin1String( "mail-mark-task" ) ) );
      d->mActions.insert( MarkMailAsActionItem, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_as_action_item" ), action );
      action->setData( QByteArray("K") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAs() ) );
      break;
    case MarkAllMailAsRead:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "mail-mark-read" ) ) );
      action->setText( i18n( "Mark &All Mails as Read" ) );
      d->mActions.insert( MarkAllMailAsRead, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_all_as_read" ), action );
      action->setData( QByteArray("R") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAllAs() ) );
      break;
    case MarkAllMailAsUnread:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "Mark &All Mails as Unread" ) );
      action->setIcon( KIcon( QLatin1String( "mail-mark-unread" ) ) );
      d->mActions.insert( MarkAllMailAsUnread, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_all_as_unread" ), action );
      action->setData( QByteArray("U") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAllAs() ) );
      break;
    case MarkAllMailAsImportant:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "Mark &All Mails as Important" ) );
      action->setIcon( KIcon( QLatin1String( "mail-mark-important" ) ) );
      d->mActions.insert( MarkAllMailAsImportant, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_all_as_important" ), action );
      action->setData( QByteArray("G") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAllAs() ) );
      break;
    case MarkAllMailAsActionItem:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "Mark &All Mails as Action Item" ) );
      action->setIcon( KIcon( QLatin1String( "mail-mark-task" ) ) );
      d->mActions.insert( MarkAllMailAsActionItem, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_mark_all_as_action_item" ), action );
      action->setData( QByteArray("K") );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMarkAllAs() ) );
      break;
    case MoveToTrash:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "user-trash" ) ) );
      action->setText( i18n( "Move to &Trash" ) );
      action->setShortcut( QKeySequence( Qt::Key_Delete ) );
      action->setWhatsThis( i18n( "Move message to trashcan" ) );
      d->mActions.insert( MoveToTrash, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_move_to_trash" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMoveToTrash() ) );
      break;
    case MoveAllToTrash:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "user-trash" ) ) );
      action->setText( i18n( "Move All to &Trash" ) );
      d->mActions.insert( MoveAllToTrash, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_move_all_to_trash" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotMoveAllToTrash() ) );
      break;
    case RemoveDuplicates:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "Remove &Duplicate Mails" ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Asterisk ) );
      d->mActions.insert( RemoveDuplicates, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_remove_duplicates" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotRemoveDuplicates() ) );
      break;
    case EmptyAllTrash:
      action = new KAction( d->mParentWidget );
      action->setText( i18n( "Empty All &Trash Folders" ) );
      d->mActions.insert( EmptyAllTrash, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_empty_all_trash" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotEmptyAllTrash() ) );
      break;
    default:
      Q_ASSERT( false ); // should never happen
      break;
  }

  return action;
}

KAction* StandardMailActionManager::createAction( StandardActionManager::Type type )
{
  return d->mGenericManager->createAction( type );
}

void StandardMailActionManager::createAllActions()
{
  createAction( MarkMailAsRead );
  createAction( MarkMailAsUnread );
  createAction( MarkMailAsImportant );
  createAction( MarkMailAsActionItem );
  createAction( MarkAllMailAsRead );
  createAction( MarkAllMailAsUnread );
  createAction( MarkAllMailAsImportant );
  createAction( MarkAllMailAsActionItem );
  createAction( MoveToTrash );
  createAction( MoveAllToTrash );
  createAction( RemoveDuplicates );
  createAction( EmptyAllTrash );

  d->mGenericManager->createAllActions();

  d->updateActions();
}

KAction* StandardMailActionManager::action( Type type ) const
{
  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );

  return 0;
}

KAction* StandardMailActionManager::action( StandardActionManager::Type type ) const
{
  return d->mGenericManager->action( type );
}

void StandardMailActionManager::setActionText( StandardActionManager::Type type, const KLocalizedString &text )
{
  d->mGenericManager->setActionText( type, text );
}

void StandardMailActionManager::interceptAction( Type type, bool intercept )
{
  if ( intercept )
    d->mInterceptedActions.insert( type );
  else
    d->mInterceptedActions.remove( type );
}

void StandardMailActionManager::interceptAction( StandardActionManager::Type type, bool intercept )
{
  d->mGenericManager->interceptAction( type, intercept );
}

Akonadi::Collection::List StandardMailActionManager::selectedCollections() const
{
  return d->mGenericManager->selectedCollections();
}

Akonadi::Item::List StandardMailActionManager::selectedItems() const
{
  return d->mGenericManager->selectedItems();
}

void StandardMailActionManager::setFavoriteCollectionsModel( FavoriteCollectionsModel *favoritesModel )
{
  d->mGenericManager->setFavoriteCollectionsModel( favoritesModel );
}

void StandardMailActionManager::setFavoriteSelectionModel( QItemSelectionModel *selectionModel )
{
  d->mGenericManager->setFavoriteSelectionModel( selectionModel );
}



#include "standardmailactionmanager.moc"