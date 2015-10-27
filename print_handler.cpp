// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "print_handler.h"

#include "debug.h"

CefSize PrintHandler::GetPdfPaperSize(int device_units_per_inch)
{
  qCDebug(print) << "size queried" << device_units_per_inch;
  return CefSize(8.5 * device_units_per_inch, 11.0 * device_units_per_inch);
}

bool PrintHandler::OnPrintDialog(bool has_selection, CefRefPtr<CefPrintDialogCallback> callback)
{
  qCDebug(print) << has_selection;
  return false;
}

bool PrintHandler::OnPrintJob(const CefString& document_name, const CefString& pdf_file_path, CefRefPtr<CefPrintJobCallback> callback)
{
  qCDebug(print) << document_name << pdf_file_path;
  return false;
}

void PrintHandler::OnPrintReset()
{
  qCDebug(print) << "!";
}

void PrintHandler::OnPrintSettings(CefRefPtr<CefPrintSettings> settings, bool get_defaults)
{
  qCDebug(print) << get_defaults;
}

#if CEF_COMMIT_NUMBER > 1332 // TODO: find correct commit number that adds this
void PrintHandler::OnPrintStart(CefRefPtr<CefBrowser> browser)
{
  qCDebug(print) << "!";
}
#endif
