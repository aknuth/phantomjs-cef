// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CEF_TESTS_PHANTOMJS_APP_H_
#define CEF_TESTS_PHANTOMJS_APP_H_

#include "include/cef_app.h"

class PrintHandler;

class PhantomJSApp : public CefApp,
                  public CefBrowserProcessHandler {
 public:
  PhantomJSApp();
  ~PhantomJSApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
      OVERRIDE { return this; }

  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() OVERRIDE;
  virtual CefRefPtr<CefPrintHandler> GetPrintHandler() OVERRIDE;

 private:
  CefRefPtr<PrintHandler> m_printHandler;
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PhantomJSApp);
};

#endif  // CEF_TESTS_PHANTOMJS_APP_H_
