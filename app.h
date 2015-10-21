// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CEF_TESTS_PHANTOMJS_APP_H_
#define CEF_TESTS_PHANTOMJS_APP_H_

#include "include/cef_app.h"

class PrintHandler;

class PhantomJSApp : public CefApp,
                     public CefBrowserProcessHandler,
                     public CefRenderProcessHandler
{
 public:
  PhantomJSApp();
  ~PhantomJSApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE
  {
    return this;
  }

  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE
  {
    return this;
  }
  void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar) OVERRIDE;

  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() OVERRIDE;
  virtual CefRefPtr<CefPrintHandler> GetPrintHandler() OVERRIDE;

  // CefRenderProcessHandler methods:
  void OnWebKitInitialized() OVERRIDE;
  void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) OVERRIDE;

 private:
  CefRefPtr<PrintHandler> m_printHandler;
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PhantomJSApp);
};

#endif  // CEF_TESTS_PHANTOMJS_APP_H_
