// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "handler.h"

#include <sstream>
#include <string>
#include <iostream>
#include <codecvt>
#include <locale>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "print_handler.h"

std::ostream& operator<<(std::ostream &stream, const wchar_t *input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return stream << utf8_conv.to_bytes(input);
}

namespace {

PhantomJSHandler* g_instance = NULL;

}  // namespace

PhantomJSHandler::PhantomJSHandler()
    : is_closing_(false)
{
  DCHECK(!g_instance);
  g_instance = this;
}

PhantomJSHandler::~PhantomJSHandler() {
  g_instance = NULL;
}

// static
PhantomJSHandler* PhantomJSHandler::GetInstance() {
  return g_instance;
}

void PhantomJSHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  std::cerr << "browser created\n";
  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
}

bool PhantomJSHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void PhantomJSHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void PhantomJSHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL " << std::string(failedUrl) <<
        " with error " << std::string(errorText) << " (" << errorCode <<
        ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

void PhantomJSHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
  CEF_REQUIRE_UI_THREAD();

  std::cerr << "load state change:" << isLoading << canGoBack << canGoForward << "\n";
  if (!isLoading) {
    std::cerr << "printing\n";
    std::string path("/tmp/test.pdf");
    CefPdfPrintSettings settings;
    settings.page_width = 100;
    settings.page_height = 100;

    browser->GetHost()->PrintToPDF(path, settings, new PdfPrintCallback);
  }
}

void PhantomJSHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&PhantomJSHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}
