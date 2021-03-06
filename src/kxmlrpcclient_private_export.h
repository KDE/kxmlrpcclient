/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KXMLRPCCLIENTPRIVATE_EXPORT_H
#define KXMLRPCCLIENTPRIVATE_EXPORT_H

#include "kxmlrpcclient_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef KXMLRPCCLIENT_TESTS_EXPORT
#define KXMLRPCCLIENT_TESTS_EXPORT KXMLRPCCLIENT_EXPORT
#endif
#else /* not compiling tests */
#define KXMLRPCCLIENT_TESTS_EXPORT
#endif

#endif
