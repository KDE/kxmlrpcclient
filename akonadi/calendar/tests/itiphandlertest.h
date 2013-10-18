/*
    Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

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

#ifndef ITIPHANDLER_TEST_H
#define ITIPHANDLER_TEST_H

#include "../incidencechanger.h"
#include "../itiphandler.h"
#include "unittestbase.h"

#include <akonadi/collection.h>

#include <QObject>
#include <QHash>


class ITIPHandlerTest : public UnitTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    // Tests processing an incoming message
    void testProcessITIPMessage_data();
    void testProcessITIPMessage();

private:
    void waitForSignals();

public Q_SLOTS:
    void oniTipMessageProcessed(Akonadi::ITIPHandler::Result result,
                                const QString &errorMessage);

    void onLoadFinished(bool success, const QString &errorMessage);
private:
    int m_pendingItipMessageSignal;
    int m_pendingLoadedSignal;
    Akonadi::ITIPHandler::Result m_expectedResult;
};

#endif