// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "app.h"

#include <string>
#include <iostream>
#include <fstream>

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

void PhantomJSApp::OnContextInitialized()
{
  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "phantomjs");
#endif
  window_info.SetAsWindowless(0, true);

  // PhantomJSHandler implements browser-level callbacks.
  CefRefPtr<PhantomJSHandler> handler(new PhantomJSHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  // TODO: make this configurable
  browser_settings.web_security = STATE_DISABLED;

  // Create the first browser window.
  auto browser = CefBrowserHost::CreateBrowserSync(window_info, handler.get(), "about:blank",
                                                   browser_settings, NULL);
  if (!browser) {
    std::cerr << "Failed to create initial browser synchronously.\n";
    return;
  }

  auto frame = browser->GetMainFrame();

  // load empty html content
  frame->LoadString("<html><head></head><body></body></html>", "file:///phantom.html");

  // inject user provided js file
  auto command_line = CefCommandLine::GetGlobalCommandLine();
  if (command_line->HasArguments()) {
    CefCommandLine::ArgumentList arguments;
    command_line->GetArguments(arguments);
    for (auto argument : arguments) {
      std::ifstream file(argument);
      if (!file.is_open()) {
        std::cerr << "Failed to open input file: " << argument << '\n';
        // TODO: quit application
        return;
      }
      std::string line;
      std::string fileContents;
      while (std::getline(file, line)) {
        fileContents.append(line);
        fileContents.append("\n");
      }
      if (fileContents.empty()) {
        continue; // otherwise we'll assert in the execution below
      }
      frame->ExecuteJavaScript(fileContents, argument, 1);
    }
  }
}

CefRefPtr<CefPrintHandler> PhantomJSApp::GetPrintHandler()
{
  std::cerr << "print handler accessed\n";
  return m_printHandler;
}
