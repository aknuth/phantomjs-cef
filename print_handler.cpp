// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "print_handler.h"

#include <iostream>

void PdfPrintCallback::OnPdfPrintFinished(const CefString& path, bool ok)
{
    std::cerr << "printing finished " << path << '\t' << ok << "\n";
}

CefSize PrintHandler::GetPdfPaperSize(int device_units_per_inch)
{
    std::cerr << "size queried" << device_units_per_inch << "\n";
    return CefSize(8.5 * device_units_per_inch, 11.0 * device_units_per_inch);
}

bool PrintHandler::OnPrintDialog(bool has_selection, CefRefPtr<CefPrintDialogCallback> callback)
{
    std::cerr << __FUNCTION__ << has_selection << "\n";
    return false;
}

bool PrintHandler::OnPrintJob(const CefString& document_name, const CefString& pdf_file_path, CefRefPtr<CefPrintJobCallback> callback)
{
    std::cerr << __FUNCTION__ << document_name << "\t" << pdf_file_path << "\n";
    return false;
}

void PrintHandler::OnPrintReset()
{
    std::cerr << __FUNCTION__ << "\n";
}

void PrintHandler::OnPrintSettings(CefRefPtr<CefPrintSettings> settings, bool get_defaults)
{
    std::cerr << __FUNCTION__ << get_defaults << "\n";
}
