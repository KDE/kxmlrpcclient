// $Id$

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "calformat.h"
//#include "koprefs.h"

#include "incidence.h"

using namespace KCal;

Incidence::Incidence() :
  mReadOnly(false), mRelatedTo(0), mSecrecy(SecrecyPublic), mPriority(3), mPilotId(0),
  mSyncStatus(SYNCMOD), mFloats(true), mDuration(0), mHasDuration(false)
{
  mAlarm = new KOAlarm(this);
  mRecurrence = new KORecurrence(this);

  recreate();

//  setOrganizer(KOPrefs::instance()->mEmail);
  setOrganizer("Fix me");
  if (organizer().isEmpty())
    setOrganizer("x-none");
  
  mSummary = "";
  mDescription = "";

  mExDates.setAutoDelete(true);
}

Incidence::Incidence(const Incidence &i) : QObject()
{
// TODO: reenable attributes currently commented out.
  mReadOnly = i.mReadOnly;
  kdDebug() << "Copying start date: " << i.mDtStart.toString() << endl;
  mDtStart = i.mDtStart;
  mDuration = i.mDuration;
  mHasDuration = i.mHasDuration;
  mOrganizer = i.mOrganizer;
  mVUID = i.mVUID;
  mRevision = i.mRevision;
//  QList<Attendee> mAttendees;     QList<Attendee> mAttendees;
  mLastModified = i.mLastModified;
  mCreated = i.mCreated;
  mDescription = i.mDescription;
  mSummary = i.mSummary;
  mCategories = i.mCategories;
//  Incidence *mRelatedTo;          Incidence *mRelatedTo;
  mRelatedTo = 0;
  mRelatedToVUID = i.mRelatedToVUID;
//  QList<Incidence> mRelations;    QList<Incidence> mRelations;
  mExDates = i.mExDates;
  mAttachments = i.mAttachments;
  mResources = i.mResources;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mPilotId = i.mPilotId;
  mSyncStatus = i.mSyncStatus;
  mFloats = i.mFloats;
//  KOAlarm *mAlarm;                KOAlarm *mAlarm;
  mAlarm = new KOAlarm(this);
//  KORecurrence *mRecurrence;      KORecurrence *mRecurrence;
  mRecurrence = new KORecurrence(this);
}

Incidence::~Incidence()
{
  Incidence *ev;
  for (ev=mRelations.first();ev;ev=mRelations.next()) {
    if (ev->relatedTo() == this) ev->setRelatedTo(0);
  }
  if (relatedTo()) relatedTo()->removeRelation(this);  

  delete mRecurrence;
  delete mAlarm;
}

void Incidence::recreate()
{
  setCreated(QDateTime::currentDateTime());

  setVUID(CalFormat::createUniqueId());

  setRevision(0);

  setLastModified(QDateTime::currentDateTime());
}

void Incidence::setReadOnly(bool readonly)
{
  mReadOnly = readonly;
  recurrence()->setRecurReadOnly(mReadOnly);
  alarm()->setAlarmReadOnly(mReadOnly);
}

void Incidence::setLastModified(const QDateTime &lm)
{
  // DON'T! emit eventUpdated because we call this from
  // Calendar::updateEvent().
  mLastModified = lm;
}

QDateTime Incidence::lastModified() const
{
  return mLastModified;
}

void Incidence::setCreated(QDateTime created)
{
  if (mReadOnly) return;
  mCreated = created;
}

QDateTime Incidence::created() const
{
  return mCreated;
}

void Incidence::setVUID(const QString &VUID)
{
  mVUID = VUID;
  emit eventUpdated(this);
}

QString Incidence::VUID() const
{
  return mVUID;
}

void Incidence::setRevision(int rev)
{
  if (mReadOnly) return;
  mRevision = rev;
  emit eventUpdated(this);
}

int Incidence::revision() const
{
  return mRevision;
}

void Incidence::setOrganizer(const QString &o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;
  if (mOrganizer.left(7).upper() == "MAILTO:")
    mOrganizer = mOrganizer.remove(0,7);

  emit eventUpdated(this);
}

QString Incidence::organizer() const
{
  return mOrganizer;
}


void Incidence::setDtStart(const QDateTime &dtStart)
{
  /*int diffsecs = mDtStart.secsTo(dtStart);
  
  if (mReadOnly) return;
  if (alarm()->enabled())
  alarm()->setTime(alarm()->time().addSecs(diffsecs));*/

  mDtStart = dtStart;

  recurrence()->setRecurStart(mDtStart);
  //alarm()->setAlarmStart(mDtStart);

  emit eventUpdated(this);
}

QDateTime Incidence::dtStart() const
{
  return mDtStart;
}

QString Incidence::dtStartTimeStr() const
{
  return KGlobal::locale()->formatTime(dtStart().time());
}

QString Incidence::dtStartDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtStart().date(),shortfmt);
}

QString Incidence::dtStartStr() const
{
  return KGlobal::locale()->formatDateTime(dtStart());
}


bool Incidence::doesFloat() const
{
  return mFloats;
}

void Incidence::setFloats(bool f)
{
  if (mReadOnly) return;
  mFloats = f;
  emit eventUpdated(this);
}


void Incidence::addAttendee(Attendee *a)
{
//  kdDebug() << "Incidence::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug() << "Incidence::addAttendee() weiter" << endl;
  if (a->name().left(7).upper() == "MAILTO:")
    a->setName(a->name().remove(0,7));

  mAttendees.append(a);
  emit eventUpdated(this);
}

#if 0
void Incidence::removeAttendee(Attendee *a)
{
  if (mReadOnly) return;
  mAttendees.removeRef(a);
  emit eventUpdated(this);
}

void Incidence::removeAttendee(const char *n)
{
  Attendee *a;

  if (mReadOnly) return;
  for (a = mAttendees.first(); a; a = mAttendees.next())
    if (a->getName() == n) {
      mAttendees.remove();
      break;
    }
}
#endif

void Incidence::clearAttendees()
{
  if (mReadOnly) return;
  mAttendees.clear();
}

#if 0
Attendee *Incidence::getAttendee(const char *n) const
{
  QListIterator<Attendee> qli(mAttendees);

  qli.toFirst();
  while (qli) {
    if (qli.current()->getName() == n)
      return qli.current();
    ++qli;
  }
  return 0L;
}
#endif

void Incidence::setDescription(const QString &description)
{
  if (mReadOnly) return;
  mDescription = description;
  emit eventUpdated(this);
}

QString Incidence::description() const
{
  return mDescription;
}


void Incidence::setSummary(const QString &summary)
{
  if (mReadOnly) return;
  mSummary = summary;
  emit eventUpdated(this);
}

QString Incidence::summary() const
{
  return mSummary;
}

void Incidence::setCategories(const QStringList &categories)
{
  if (mReadOnly) return;
  mCategories = categories;
  emit eventUpdated(this);
}

// TODO: remove setCategories(QString) function
void Incidence::setCategories(const QString &catStr)
{
  if (mReadOnly) return;

  if (catStr.isEmpty()) return;

  mCategories = QStringList::split(",",catStr);

  QStringList::Iterator it;
  for(it = mCategories.begin();it != mCategories.end(); ++it) {
    *it = (*it).stripWhiteSpace();
  }

  emit eventUpdated(this);
}

QStringList Incidence::categories() const
{
  return mCategories;
}

QString Incidence::categoriesStr()
{
  return mCategories.join(",");
}

void Incidence::setRelatedToVUID(const QString &relatedToVUID)
{
  if (mReadOnly) return;
  mRelatedToVUID = relatedToVUID;
}

QString Incidence::relatedToVUID() const
{
  return mRelatedToVUID;
}

void Incidence::setRelatedTo(Incidence *relatedTo)
{
  if (mReadOnly) return;
  Incidence *oldRelatedTo = mRelatedTo;
  if(oldRelatedTo) {
    oldRelatedTo->removeRelation(this);
  }
  mRelatedTo = relatedTo;
  if (mRelatedTo) mRelatedTo->addRelation(this);
}

Incidence *Incidence::relatedTo() const
{
  return mRelatedTo;
}

QList<Incidence> Incidence::relations() const
{
  return mRelations;
}

void Incidence::addRelation(Incidence *event)
{
  mRelations.append(event);
  emit eventUpdated(this);
}

void Incidence::removeRelation(Incidence *event)
{
  mRelations.removeRef(event);
//  if (event->getRelatedTo() == this) event->setRelatedTo(0);
}

bool Incidence::recursOn(const QDate &qd) const
{
  if (recurrence()->recursOnPure(qd) && !isException(qd)) return true;
  else return false;
}

void Incidence::setExDates(const QDateList &exDates)
{
  if (mReadOnly) return;
  mExDates.clear();
  mExDates = exDates;

  recurrence()->setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

void Incidence::setExDates(const char *dates)
{
  if (mReadOnly) return;
  mExDates.clear();
  QString tmpStr = dates;
  int index = 0;
  int index2 = 0;

  while ((index2 = tmpStr.find(',', index)) != -1) {
    QDate *tmpDate = new QDate;
    *tmpDate = strToDate(tmpStr.mid(index, (index2-index)));
    
    mExDates.append(tmpDate);
    index = index2 + 1;
  }
  QDate *tmpDate = new QDate;
  *tmpDate = strToDate(tmpStr.mid(index, (tmpStr.length()-index)));
  mExDates.inSort(tmpDate);

  recurrence()->setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

void Incidence::addExDate(const QDate &date)
{
  if (mReadOnly) return;
  QDate *addDate = new QDate(date);
  mExDates.inSort(addDate);

  recurrence()->setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
}

QDateList Incidence::exDates() const
{
  return mExDates;
}

bool Incidence::isException(const QDate &qd) const
{
  QDateList tmpList(FALSE); // we want a shallow copy

  tmpList = mExDates;

  QDate *datePtr;
  for (datePtr = tmpList.first(); datePtr;
       datePtr = tmpList.next()) {
    if (qd == *datePtr) {
      return TRUE;
    }
  }
  return FALSE;
}

void Incidence::setAttachments(const QStringList &attachments)
{
  if (mReadOnly) return;
  mAttachments = attachments;
  emit eventUpdated(this);
}

QStringList Incidence::attachments() const
{
  return mAttachments;
}

void Incidence::setResources(const QStringList &resources)
{
  if (mReadOnly) return;
  mResources = resources;
  emit eventUpdated(this);
}

QStringList Incidence::resources() const
{
  return mResources;
}


void Incidence::setPriority(int priority)
{
  if (mReadOnly) return;
  mPriority = priority;
  emit eventUpdated(this);
}

int Incidence::priority() const
{
  return mPriority;
}

void Incidence::setSecrecy(int sec)
{
  if (mReadOnly) return;
  mSecrecy = sec;
  emit eventUpdated(this);
}

int Incidence::secrecy() const
{
  return mSecrecy;
}

QString Incidence::secrecyStr() const
{
  return secrecyName(mSecrecy);
}

QString Incidence::secrecyName(int secrecy)
{
  switch (secrecy) {
    case SecrecyPublic:
      return i18n("Public");
      break;
    case SecrecyPrivate:
      return i18n("Private");
      break;
    case SecrecyConfidential:
      return i18n("Confidential");
      break;
    default:
      return i18n("Undefined");
      break;
  }
}

QStringList Incidence::secrecyList()
{
  QStringList list;
  list << secrecyName(SecrecyPublic);
  list << secrecyName(SecrecyPrivate);
  list << secrecyName(SecrecyConfidential);

  return list;
}



void Incidence::setPilotId(int id)
{
  if (mReadOnly) return;
  mPilotId = id;
  //emit eventUpdated(this);
}

int Incidence::pilotId() const
{
  return mPilotId;
}

void Incidence::setSyncStatus(int stat)
{
  if (mReadOnly) return;
  mSyncStatus = stat;
  //  emit eventUpdated(this);
}

int Incidence::syncStatus() const
{
  return mSyncStatus;
}

KOAlarm *Incidence::alarm() const
{
  return mAlarm;
}

KORecurrence *Incidence::recurrence() const
{
  return mRecurrence;
}

QDate Incidence::strToDate(const QString &dateStr)
{

  int year, month, day;

  year = dateStr.left(4).toInt();
  month = dateStr.mid(4,2).toInt();
  day = dateStr.mid(6,2).toInt();
  return(QDate(year, month, day));
}

void Incidence::setDuration(int seconds)
{
  mDuration = seconds;
  setHasDuration(true);
}

int Incidence::duration() const
{
  return mDuration;
}

void Incidence::setHasDuration(bool)
{
  mHasDuration = true;
}

bool Incidence::hasDuration() const
{
  return mHasDuration;
}

#include "incidence.moc"
