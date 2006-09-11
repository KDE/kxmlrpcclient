/*
    This file is part of the kcal library.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdatetime.h>

#include "duration.h"

using namespace KCal;

Duration::Duration()
{
  mSeconds = 0;
}

Duration::Duration( const KDateTime &start, const KDateTime &end )
{
  mSeconds = start.secsTo( end );
}

Duration::Duration( int seconds )
{
  mSeconds = seconds;
}

bool KCal::operator==( const Duration& d1, const Duration& d2 )
{
  return ( d1.asSeconds() == d2.asSeconds() );
}

KDateTime Duration::end( const KDateTime &start ) const
{
  return start.addSecs( mSeconds );
}

int Duration::asSeconds() const
{
  return mSeconds;
}

// DEPRECATED methods
Duration::Duration( const QDateTime &start, const QDateTime &end )
{
  mSeconds = start.secsTo( end );
}

QDateTime Duration::end( const QDateTime &start ) const
{
  return start.addSecs( mSeconds );
}

