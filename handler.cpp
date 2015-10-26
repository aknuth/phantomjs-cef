// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "handler.h"

#include <sstream>
#include <string>
#include <iostream>
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
    return stream << qPrintable(QString::fromWCharArray(input));
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
  // TODO: send a signal via persistent callback?
//   std::string titleStr(title);

//   std::cerr << "title changed to: " << title << '\n';
}

bool PhantomJSHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
  std::cerr << source << ':' << line << ": " << message << '\n';
  return true;
}

void PhantomJSHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  m_browsers[browser->GetIdentifier()] = browser;
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

  m_browsers.remove(browser->GetIdentifier());

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

void PhantomJSHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
  std::cerr << "load started " << frame->GetURL() << '\n';
}

void PhantomJSHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
  CEF_REQUIRE_UI_THREAD();

  std::cerr << "load end " << frame->GetURL() << ", status = " << httpStatusCode << '\n';

  auto it = m_pendingOpenBrowserRequests.find(browser->GetIdentifier());
  if (it != m_pendingOpenBrowserRequests.end()) {
    if (httpStatusCode >= 400) {
      it.value()->Failure(httpStatusCode, "load error");
    } else {
      it.value()->Success(std::to_string(browser->GetIdentifier()));
    }
    m_pendingOpenBrowserRequests.erase(it);
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

  foreach (const auto& browser, m_browsers) {
    browser->GetHost()->CloseBrowser(force_close);
  }
  LOG_ASSERT(m_browsers.empty());
}

bool PhantomJSHandler::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               int64 query_id, const CefString& request, bool persistent,
                               CefRefPtr<Callback> callback)
{
  CEF_REQUIRE_UI_THREAD();

  const auto data = QByteArray::fromStdString(request.ToString());

  QJsonParseError error;
  const auto json = QJsonDocument::fromJson(data, &error).object();
  if (error.error) {
    qWarning() << error.errorString();
    return false;
  }

  const auto type = json.value(QStringLiteral("type")).toString();

  if (type == QLatin1String("openWebPage")) {
    const auto url = json.value(QStringLiteral("url")).toString().toStdString();
    const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
    CefRefPtr<CefBrowser> subBrowser;
    if (subBrowserId == -1) {
      // first open request on a new web page creates a new browser instance
      subBrowser = createBrowser(url);
    } else {
      // otherwise reuse the existing browser instance
      subBrowser =  m_browsers.value(subBrowserId);
      if (!subBrowser) {
        qWarning() << "Cannot open web page in browser with unknown id" << subBrowserId;
        return false;
      }
      subBrowser->GetMainFrame()->LoadURL(url);
    }

    m_pendingOpenBrowserRequests[subBrowser->GetIdentifier()] = callback;
    return true;
  } else if (type == QLatin1String("closeWebPage")) {
    const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
    auto subBrowser = m_browsers.value(subBrowserId);
    if (subBrowser) {
      subBrowser->GetHost()->CloseBrowser(true);
      callback->Success({});
      return true;
    } else {
      qWarning() << "Cannot close unknown browser with id" << subBrowser;
    }
  } else if (type == QLatin1String("evaluateJavaScript")) {
    const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
    auto script = json.value(QStringLiteral("script")).toString();
    auto subBrowser = m_browsers.value(subBrowserId);
    if (subBrowser) {
      m_pendingQueryCallbacks[query_id] = callback;
      script = "phantom.handleEvaluateJavaScript(" + script + ", " + QString::number(query_id) + ")";
      subBrowser->GetMainFrame()->ExecuteJavaScript(script.toStdString(), "phantomjs://evaluateJavaScript", 1);
    } else {
      callback->Failure(1, "unknown browser id");
    }
    return true;
  } else if (type == QLatin1String("returnEvaluateJavaScript")) {
    auto otherQueryId = json.value(QStringLiteral("queryId")).toInt(-1);
    auto it = m_pendingQueryCallbacks.find(otherQueryId);
    if (it != m_pendingQueryCallbacks.end()) {
      auto retval = json.value(QStringLiteral("retval")).toString();
      auto exception = json.value(QStringLiteral("exception")).toString();
      auto otherCallback = it.value();
      if (exception.isEmpty()) {
        otherCallback->Success(retval.toStdString());
      } else {
        otherCallback->Failure(1, exception.toStdString());
      }
      m_pendingQueryCallbacks.erase(it);
      callback->Success({});
      return true;
    }
  }
  return false;
}

void PhantomJSHandler::OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       int64 query_id)
{
  CEF_REQUIRE_UI_THREAD();

  m_pendingOpenBrowserRequests.remove(browser->GetIdentifier());
  m_pendingQueryCallbacks.remove(query_id);
}
