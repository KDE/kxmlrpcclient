/*
    This file is part of the kxmlrpc library.
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KXMLRPC_TEST_CLIENT_H_
#define _KXMLRPC_TEST_CLIENT_H_

#include <QObject>

class ClientTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
};

#endif

