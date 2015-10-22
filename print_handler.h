// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CEF_TESTS_PHANTOMJS_PRINT_HANDLER_H_
#define CEF_TESTS_PHANTOMJS_PRINT_HANDLER_H_

#include "include/cef_print_handler.h"

#include "include/cef_version.h"

class PdfPrintCallback : public CefPdfPrintCallback
{
public:
  // CefPdfPrintCallback methods:
  // TODO: move this out of the handler?
  virtual void OnPdfPrintFinished(const CefString& path, bool ok) override;

private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PdfPrintCallback);
};

class PrintHandler : public CefPrintHandler
{
 public:
  // CefPrintHandler methods:
  virtual CefSize GetPdfPaperSize(int device_units_per_inch) override;
  virtual bool OnPrintDialog(bool has_selection, CefRefPtr<CefPrintDialogCallback> callback) override;
  virtual bool OnPrintJob(const CefString& document_name, const CefString& pdf_file_path,
                          CefRefPtr<CefPrintJobCallback> callback) override;
  virtual void OnPrintReset() override;
  virtual void OnPrintSettings(CefRefPtr<CefPrintSettings> settings, bool get_defaults) override;
#if CEF_COMMIT_NUMBER > 1332 // TODO: find correct commit number that adds this
  virtual void OnPrintStart(CefRefPtr<CefBrowser> browser) override;
#endif

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PrintHandler);
};

#endif  // CEF_TESTS_PHANTOMJS_PRINT_HANDLER_H_

