// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "app.h"

#include <string>
#include <iostream>

#include "handler.h"
#include "print_handler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

PhantomJSApp::PhantomJSApp()
  : m_printHandler(new PrintHandler)
{
}

PhantomJSApp::~PhantomJSApp()
{
}

void PhantomJSApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "phantomjs");
#endif

  // PhantomJSHandler implements browser-level callbacks.
  CefRefPtr<PhantomJSHandler> handler(new PhantomJSHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  // TODO: make this configurable
  browser_settings.web_security = STATE_DISABLED;

  std::string url;

  // Check if a "--url=" value was provided via the command-line. If so, use
  // that instead of the default URL.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  url = command_line->GetSwitchValue("url");
  if (url.empty())
    url = "http://www.google.com";

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, handler.get(), url,
                                browser_settings, NULL);
}

CefRefPtr<CefPrintHandler> PhantomJSApp::GetPrintHandler()
{
  std::cerr << "print handler accessed\n";
  return m_printHandler;
}
