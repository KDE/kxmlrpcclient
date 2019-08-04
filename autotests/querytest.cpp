/*
    This file is part of the kxmlrpc library.

    Copyright (c) 2006 Narayan Newton <narayannewton@gmail.com>

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

#include <QTest>

#include "querytest.h"

#include "query_p.h"

QTEST_MAIN(QueryTest)

using namespace KXmlRpc;

#define XML_CALL_HEAD(call) QByteArray("<?xml version=\"1.0\" ?>\r\n<methodCall>\r\n<methodName>" call "</methodName>\r\n<params>\r\n") +
#define XML_CALL_END "</params>\r\n</methodCall>\r\n"

#define XML_RESPONSE_HEAD QByteArray("<?xml version=\"1.0\" ?>\r\n<methodResponse>\r\n<params>\r\n") +
#define XML_RESPONSE_END "</params>\r\n</methodResponse>\r\n"

#define XML_FAULT_HEAD QByteArray("<?xml version=\"1.0\" ?>\r\n<methodResponse>\r\n<fault>\r\n") +
#define XML_FAULT_END "</fault>\r\n</methodResponse>\r\n"

void QueryTest::testValidity()
{
    Query *q = Query::create();
    QVERIFY(q != nullptr);
}

void QueryTest::testMarkupCall_data()
{
    QTest::addColumn<QString>("methodName");
    QTest::addColumn<QList<QVariant>>("arguments");
    QTest::addColumn<QByteArray>("xml");

    QTest::newRow("int") << QString::fromLatin1("MyMethod")
                         << (QVariantList() << 1)
                         << XML_CALL_HEAD("MyMethod")
                            "<param>\r\n"
                            "<value><int>1</int></value>\r\n"
                            "</param>\r\n"
                            XML_CALL_END;
    QTest::newRow("string") << QString::fromLatin1("MyMethod")
                            << (QVariantList() << QLatin1String("data"))
                            << XML_CALL_HEAD("MyMethod")
                               "<param>\r\n"
                               "<value><string><![CDATA[data]]></string></value>\r\n"
                               "</param>\r\n"
                               XML_CALL_END;
    QTest::newRow("string (utf8)") << QString::fromLatin1("MyMethod")
                                   // make sure it is UTF-8, so do *not* use QStringLiteral below
                                   << (QVariantList() << QString::fromUtf8("Žlutý kůň pěl ďábelské ódy"))
                                   << XML_CALL_HEAD("MyMethod")
                                      "<param>\r\n"
                                      "<value><string><![CDATA[Žlutý kůň pěl ďábelské ódy]]></string></value>\r\n"
                                      "</param>\r\n"
                                      XML_CALL_END;
    QTest::newRow("stringlist") << QString::fromLatin1("MyMethod")
                                << (QVariantList() << (QStringList() << QLatin1String("data1") << QLatin1String("data2")))
                                << XML_CALL_HEAD("MyMethod")
                                   "<param>\r\n"
                                   "<value><array><data><value><string><![CDATA[data1]]></string></value>\r\n"
                                   "<value><string><![CDATA[data2]]></string></value>\r\n"
                                   "</data></array></value>"
                                   "</param>\r\n"
                                   XML_CALL_END;
    QTest::newRow("double") << QString::fromLatin1("MyMethod")
                            << (QVariantList() << (double) 3.14)
                            << XML_CALL_HEAD("MyMethod")
                               "<param>\r\n"
                               "<value><double>3.14</double></value>\r\n"
                               "</param>\r\n"
                               XML_CALL_END;
    QTest::newRow("bool") << QString::fromLatin1("MyMethod")
                          << (QVariantList() << true)
                          << XML_CALL_HEAD("MyMethod")
                             "<param>\r\n"
                             "<value><boolean>1</boolean></value>\r\n"
                             "</param>\r\n"
                             XML_CALL_END;
    QTest::newRow("bytearray") << QString::fromLatin1("MyMethod")
                               << (QVariantList() << QByteArray("Hello World!"))
                               << XML_CALL_HEAD("MyMethod")
                                  "<param>\r\n"
                                  "<value><base64>SGVsbG8gV29ybGQh</base64></value>\r\n"
                                  "</param>\r\n"
                                  XML_CALL_END;
    QTest::newRow("datetime") << QString::fromLatin1("MyMethod")
                              << (QVariantList() << QDateTime(QDate(2015, 01, 05), QTime(17, 16, 32)))
                              << XML_CALL_HEAD("MyMethod")
                                 "<param>\r\n"
                                 "<value><dateTime.iso8601>2015-01-05T17:16:32</dateTime.iso8601></value>\r\n"
                                 "</param>\r\n"
                                 XML_CALL_END;
    QTest::newRow("list") << QString::fromLatin1("MyMethod")
                          << (QVariantList() << QVariant(QVariantList() << QLatin1String("data") << 42 << true << 5.678))
                          << XML_CALL_HEAD("MyMethod")
                             "<param>\r\n"
                             "<value><array><data>\r\n"
                             "<value><string><![CDATA[data]]></string></value>\r\n"
                             "<value><int>42</int></value>\r\n"
                             "<value><boolean>1</boolean></value>\r\n"
                             "<value><double>5.678</double></value>\r\n"
                             "</data></array></value>\r\n"
                             "</param>\r\n"
                             XML_CALL_END;

    QVariantMap map;
    map[QLatin1String("key1")] = 13;
    map[QLatin1String("key2")] = QLatin1String("data");
    map[QLatin1String("key3")] = false;
    QTest::newRow("map") << QString::fromLatin1("MyMethod")
                         << (QVariantList() << map)
                         << XML_CALL_HEAD("MyMethod")
                            "<param>\r\n"
                            "<value><struct>\r\n"
                            "<member>\r\n<name>key1</name>\r\n<value><int>13</int></value>\r\n</member>\r\n"
                            "<member>\r\n<name>key2</name>\r\n<value><string><![CDATA[data]]></string></value>\r\n</member>\r\n"
                            "<member>\r\n<name>key3</name>\r\n<value><boolean>0</boolean></value>\r\n</member>\r\n"
                            "</struct></value>\r\n"
                            "</param>\r\n"
                            XML_CALL_END;
}

void QueryTest::testMarkupCall()
{
    QFETCH(QString, methodName);
    QFETCH(QList<QVariant>, arguments);
    QFETCH(QByteArray, xml);

    const QByteArray markup = QueryPrivate::markupCall(methodName, arguments);
    QCOMPARE(markup, xml);
}

void QueryTest::testResponse_data()
{
    QTest::addColumn<bool>("isSuccess");
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<QVariantList>("arguments");

    QVariantMap map;
    map[QLatin1String("faultCode")] = 10;
    map[QLatin1String("faultString")] = QLatin1String("Fatal Server Error");
    QTest::newRow("fault") << false
                           << XML_FAULT_HEAD
                              "<value><struct>"
                              "<member><name>faultCode</name><value><int>10</int></value></member>"
                              "<member><name>faultString</name><value><string><![CDATA[Fatal Server Error]]></string></value></member>"
                              "</struct></value>"
                              XML_FAULT_END
                           << (QVariantList() << map);

    QTest::newRow("string") << true
                            << XML_RESPONSE_HEAD
                               "<param>"
                               "<value><STRING><![CDATA[result]]></STRING></value>"
                               "</param>"
                               XML_RESPONSE_END
                            << (QVariantList() << QLatin1String("result"));
    QTest::newRow("int") << true
                         << XML_RESPONSE_HEAD
                            "<param>"
                            "<value><INT>1</INT></value>"
                            "</param>"
                            XML_RESPONSE_END
                         << (QVariantList() << 1);
    QTest::newRow("i4") << true
                        << XML_RESPONSE_HEAD
                           "<param><value><I4>42</I4></value></param>"
                           XML_RESPONSE_END
                        << (QVariantList() << 42);
    QTest::newRow("boolean (num)") << true
                                   << XML_RESPONSE_HEAD
                                      "<param><value><BOOLEAN>1</BOOLEAN></value></param>"
                                      XML_RESPONSE_END
                                   << (QVariantList() << true);
    QTest::newRow("boolean (str)") << true
                                   << XML_RESPONSE_HEAD
                                      "<param><value><BOOLEAN>TruE</BOOLEAN></value></param>"
                                      XML_RESPONSE_END
                                   << (QVariantList() << true);
    QTest::newRow("base64") << true
                            << XML_RESPONSE_HEAD
                               "<param><value><BASE64>VmFsaWQgcmVzcG9uc2UK</BASE64></value></param>"
                               XML_RESPONSE_END
                            << (QVariantList() << QByteArray("Valid response\n"));
    QTest::newRow("datetime") << true
                              << XML_RESPONSE_HEAD
                                 "<param><value><DATETIME>2015-01-05T18:02:22</DATETIME></value></param>"
                                 XML_RESPONSE_END
                              << (QVariantList() << QDateTime(QDate(2015, 01, 05), QTime(18, 02, 22)));
    QTest::newRow("datetime 2") << true
                                << XML_RESPONSE_HEAD
                                   "<param><value><dateTime.iso8601>2015-01-05T17:03:15Z</dateTime.iso8601></value></param>"
                                   XML_RESPONSE_END
                                << (QVariantList() << QDateTime(QDate(2015, 01, 05), QTime(17, 03, 15), Qt::UTC));
    QTest::newRow("array") << true
                           << XML_RESPONSE_HEAD
                              "<param><value><ARRAY><data>"
                              "<value><STRING><![CDATA[item 1]]></STRING></value>"
                              "<value><STRING><![CDATA[item 2]]></STRING></value>"
                              "</data></ARRAY></value></param>"
                              XML_RESPONSE_END
                           << (QVariantList() << QVariant(QVariantList() << QLatin1String("item 1")
                                                                         << QLatin1String("item 2")));

    map.clear();
    map[QLatin1String("Key 1")] = 15;
    map[QLatin1String("Key 2")] = QLatin1String("Value");
    QTest::newRow("struct") << true
                            << XML_RESPONSE_HEAD
                               "<param><value><STRUCT>"
                               "<member><name>Key 1</name><value><I4>15</I4></value></member>"
                               "<member><name>Key 2</name><value><string><![CDATA[Value]]></string></value></member>"
                               "</STRUCT></value></param>"
                               XML_RESPONSE_END
                            << (QVariantList() << map);
}

void QueryTest::testResponse()
{
    QFETCH(bool, isSuccess);
    QFETCH(QByteArray, xml);
    QFETCH(QVariantList, arguments);

    QDomDocument doc;
    QVERIFY(doc.setContent(xml));

    QCOMPARE(QueryPrivate::isMessageResponse(doc), isSuccess);
    QCOMPARE(QueryPrivate::isFaultResponse(doc), !isSuccess);

    if (isSuccess) {
        const KXmlRpc::Result result = QueryPrivate::parseMessageResponse(doc);
        QVERIFY(result.success());
        QCOMPARE(result.errorCode(), -1);
        QVERIFY(result.errorString().isEmpty());
        QCOMPARE(result.data(), arguments);
    } else {
        const KXmlRpc::Result result = QueryPrivate::parseFaultResponse(doc);
        QVERIFY(!result.success());
        QCOMPARE(result.errorCode(), arguments[0].toMap()[QLatin1String("faultCode")].toInt());
        QCOMPARE(result.errorString(), arguments[0].toMap()[QLatin1String("faultString")].toString());
        QVERIFY(result.data().isEmpty());
    }
}

