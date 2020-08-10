/*
    This file is part of the kxmlrpc library.
    SPDX-FileCopyrightText: 2006 Narayan Newton <narayannewton@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <qtest.h>

#include "clienttest.h"

QTEST_GUILESS_MAIN(ClientTest)

#include "client.h"

using namespace KXmlRpc;

void ClientTest::testValidity()
{
    Client *c = new Client();
    c->setUrl(QUrl(QLatin1String("http://test:pass@fake.com/rpc2")));
    c->setUserAgent(QLatin1String("Fake/1.0/MozillaCompat"));
    c->setDigestAuthEnabled(true);
    QVERIFY(c->url() == QUrl(QLatin1String("http://test:pass@fake.com/rpc2")));
    QVERIFY(c->userAgent() == QLatin1String("Fake/1.0/MozillaCompat"));
    QVERIFY(c->isDigestAuthEnabled() == true);

    Client *other = new Client(QUrl(QLatin1String("http://test:pass@fake.com/rpc2")));
    QVERIFY(c->url() == other->url());

    delete c;
    delete other;
}
