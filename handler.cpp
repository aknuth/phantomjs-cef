// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "handler.h"

#include <sstream>
#include <string>
#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "print_handler.h"

std::ostream& operator<<(std::ostream& stream, const wchar_t *input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return stream << utf8_conv.to_bytes(input);
}

PhantomJSHandler::PhantomJSHandler()
    : m_messageRouter(CefMessageRouterBrowserSide::Create(messageRouterConfig()))
{
  m_messageRouter->AddHandler(this, false);
}

PhantomJSHandler::~PhantomJSHandler()
{
}

CefMessageRouterConfig PhantomJSHandler::messageRouterConfig()
{
  CefMessageRouterConfig config;
  config.js_cancel_function = "cancelPhantomJsQuery";
  config.js_query_function = "startPhantomJsQuery";
  return config;
}

CefRefPtr<CefBrowser> PhantomJSHandler::createBrowser(const CefString& url)
{
  // Information used when creating the native window.
  CefWindowInfo window_info;
#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "phantomjs");
#endif
  window_info.SetAsWindowless(0, true);

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;
  // TODO: make this configurable
  browser_settings.web_security = STATE_DISABLED;
  browser_settings.universal_access_from_file_urls = STATE_ENABLED;
  browser_settings.file_access_from_file_urls = STATE_ENABLED;

  return CefBrowserHost::CreateBrowserSync(window_info, this, url, browser_settings,
                                           NULL);
}

bool PhantomJSHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message)
{
  std::cerr << "handler got message: " << message->GetName() << '\n';
  if (m_messageRouter->OnProcessMessageReceived(browser, source_process, message)) {
    return true;
  }
  if (message->GetName() == "exit") {
    CloseAllBrowsers(true);
    return true;
  }
  return false;
}

void PhantomJSHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title)
{
  CEF_REQUIRE_UI_THREAD();
  std::string titleStr(title);

  std::cerr << "title changed to: " << title << '\n';
}

bool PhantomJSHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
  std::cerr << "Console message from " << source << ':' << line << ": " << message << '\n';
  return true;
}

void PhantomJSHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  std::cerr << "browser created\n";
  // Add to the list of existing browsers.
  m_browsers.push_back(browser);
}

bool PhantomJSHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void PhantomJSHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  m_messageRouter->OnBeforeClose(browser);

  // Remove from the list of existing browsers.
  auto it = remove_if(m_browsers.begin(), m_browsers.end(), [browser] (const CefRefPtr<CefBrowser>& other) {
    return other->IsSame(browser);
  });
  m_browsers.erase(it, m_browsers.end());

  if (m_browsers.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void PhantomJSHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl)
{
  CEF_REQUIRE_UI_THREAD();

  // notify about load error
  if (m_callback) {
    m_callback->Failure(errorCode, errorText);
  }
  if (false) { // FIXME: why is this not working?!
    auto it = m_pendingOpenBrowserRequests.find(browser->GetIdentifier());
    if (it != m_pendingOpenBrowserRequests.end()) {
      it->second->Failure(errorCode, errorText);
      m_pendingOpenBrowserRequests.erase(it);
    }
  }

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

  std::cerr << "load state change: " << isLoading << canGoBack << canGoForward << ", url = " << browser->GetMainFrame()->GetURL() << "\n";

  if (!isLoading) { // notify about load success
    if (m_callback) {
      m_callback->Success(std::to_string(browser->GetIdentifier()));
    }
    if (false) { // FIXME: why is this not working?!
      auto it = m_pendingOpenBrowserRequests.find(browser->GetIdentifier());
      if (it != m_pendingOpenBrowserRequests.end()) {
        it->second->Success(std::to_string(browser->GetIdentifier()));
        m_pendingOpenBrowserRequests.erase(it);
      }
    }
  }
}

bool PhantomJSHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
  // TODO: make this configurable
  rect.Set(0, 0, 800, 600);
  return true;
}

void PhantomJSHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
  // TODO: grab screenshots?
  // do nothing
}

void PhantomJSHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
  m_messageRouter->OnRenderProcessTerminated(browser);
}

bool PhantomJSHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)
{
  m_messageRouter->OnBeforeBrowse(browser, frame);
  return false;
}

void PhantomJSHandler::CloseAllBrowsers(bool force_close)
{
  m_messageRouter->CancelPending(nullptr, nullptr);

  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI,
        base::Bind(&PhantomJSHandler::CloseAllBrowsers, this, force_close));
    return;
  }

  for (auto browser: m_browsers) {
    browser->GetHost()->CloseBrowser(force_close);
  }
  LOG_ASSERT(m_browsers.empty());
}

bool PhantomJSHandler::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               int64 query_id, const CefString& request, bool persistent,
                               CefRefPtr<Callback> callback)
{
  const auto data = QByteArray::fromStdString(request.ToString());
  QJsonParseError error;
  const auto json = QJsonDocument::fromJson(data, &error).object();
  const auto type = json.value(QStringLiteral("type")).toString();
//   qDebug() << data << json << error.errorString() << type;
  if (type == QLatin1String("openWebPage")) {
    auto subBrowser = createBrowser(json.value(QStringLiteral("url")).toString().toStdString());
    std::cerr << "sub browser created\n" << std::endl;
    // FIXME: why is this not working?!
//     m_pendingOpenBrowserRequests[subBrowser->GetIdentifier()] = callback;
    m_callback = callback;
    return true;
  }
  return false;
}

void PhantomJSHandler::OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       int64 query_id)
{
  // TODO: remove callback
}
