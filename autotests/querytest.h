/*
    This file is part of the kxmlrpc library.
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KXMLRPC_TEST_QUERY_H_
#define _KXMLRPC_TEST_QUERY_H_

#include <QObject>

class QueryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();

    void testMarkupCall_data();
    void testMarkupCall();

    void testResponse_data();
    void testResponse();
};

#endif

