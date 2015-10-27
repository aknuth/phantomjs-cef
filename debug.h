// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef PHANTOMJS_DEBUG_H
#define PHANTOMJS_DEBUG_H

#include <QLoggingCategory>
#include <include/internal/cef_string.h>

Q_DECLARE_LOGGING_CATEGORY(handler)
Q_DECLARE_LOGGING_CATEGORY(app)
Q_DECLARE_LOGGING_CATEGORY(print)

class QDebug;

QDebug operator<<(QDebug stream, const CefString& string);

#endif // PHANTOMJS_DEBUG_H
