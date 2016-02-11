// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "debug.h"

#include <QString>
#include <QDebug>

#include <iostream>

std::ostream& operator<<(std::ostream& stream, const wchar_t *input)
{
    return stream << qPrintable(QString::fromWCharArray(input));
}

QDebug operator<<(QDebug stream, const CefString& string)
{
  return stream << QString::fromStdString(string.ToString());
}

Q_LOGGING_CATEGORY(handler, "phantomjs.handler", QtWarningMsg)
Q_LOGGING_CATEGORY(print, "phantomjs.print", QtWarningMsg)
Q_LOGGING_CATEGORY(app, "phantomjs.app", QtWarningMsg)
Q_LOGGING_CATEGORY(keyevents, "phantomjs.keyevents", QtWarningMsg)
