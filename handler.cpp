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
#include <QJsonArray>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "print_handler.h"
#include "debug.h"

#include "WindowsKeyboardCodes.h"
#include "keyevents.h"

namespace {
CefRefPtr<CefMessageRouterBrowserSide::Callback> takeCallback(QHash<int32, CefRefPtr<CefMessageRouterBrowserSide::Callback>>* callbacks,
                                                              const CefRefPtr<CefBrowser>& browser)
{
  auto it = callbacks->find(browser->GetIdentifier());
  if (it != callbacks->end()) {
    auto ret = it.value();
    callbacks->erase(it);
    return ret;
  }
  return {};
}

void initWindowInfo(CefWindowInfo& window_info, bool isPhantomMain)
{
#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
  window_info.SetAsPopup(NULL, "phantomjs");
#endif
  if (isPhantomMain || !qEnvironmentVariableIsSet("PHANTOMJS_CEF_SHOW_WINDOW")) {
    window_info.SetAsWindowless(0, true);
  }
}

void initBrowserSettings(CefBrowserSettings& browser_settings, bool isPhantomMain)
{
  // TODO: make this configurable
  if (isPhantomMain) {
    browser_settings.web_security = STATE_DISABLED;
    browser_settings.universal_access_from_file_urls = STATE_ENABLED;
    browser_settings.file_access_from_file_urls = STATE_ENABLED;
  }
}
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

CefRefPtr<CefBrowser> PhantomJSHandler::createBrowser(const CefString& url, bool isPhantomMain)
{
  CefWindowInfo window_info;
  initWindowInfo(window_info, isPhantomMain);

  CefBrowserSettings browser_settings;
  initBrowserSettings(browser_settings, isPhantomMain);

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

bool PhantomJSHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
  auto callback = m_browserSignals.value(browser->GetIdentifier());
  if (callback) {
    QJsonObject obj;
    obj[QStringLiteral("signal")] = QStringLiteral("onConsoleMessage");
    QJsonArray args;
    args.append(QString::fromStdString(message));
    args.append(QString::fromStdString(source));
    args.append(line);
    obj[QStringLiteral("args")] = args;
    callback->Success(QJsonDocument(obj).toJson().constData());
  } else {
    std::cerr << source << ':' << line << ": " << message << '\n';
  }
  return true;
}

void PhantomJSHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  m_browsers[browser->GetIdentifier()] = browser;

  qCDebug(handler) << browser->GetIdentifier();
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

  qCDebug(handler) << browser->GetIdentifier();

  m_messageRouter->OnBeforeClose(browser);

  m_browsers.remove(browser->GetIdentifier());

  if (m_browsers.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

bool PhantomJSHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                     const CefString& target_url, const CefString& target_frame_name,
                                     CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
                                     const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
                                     CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
                                     bool* no_javascript_access)
{
  qCDebug(handler) << browser->GetIdentifier() << frame->GetURL() << target_url << target_frame_name;
  initWindowInfo(windowInfo, false);
  initBrowserSettings(settings, false);
  client = this;
  return false;
}

void PhantomJSHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl)
{
  CEF_REQUIRE_UI_THREAD();

  qCDebug(handler) << browser->GetIdentifier() << frame->IsMain() << errorCode << errorText << failedUrl;

  if (frame->IsMain()) {
    handleLoadEnd(browser, errorCode, false);
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

void PhantomJSHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
  CEF_REQUIRE_UI_THREAD();

  qCDebug(handler) << browser->GetIdentifier() << frame->GetURL() << frame->IsMain();

  // filter out events from sub frames
  if (!frame->IsMain()) {
    return;
  }

  auto callback = m_browserSignals.value(browser->GetIdentifier());
  if (!callback) {
    return;
  }
  callback->Success("{\"signal\":\"onLoadStarted\"}");
}

void PhantomJSHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
  CEF_REQUIRE_UI_THREAD();

  // filter out events from sub frames or when loading about:blank
  if (!frame->IsMain() || httpStatusCode < 200) {
    return;
  }

  qCDebug(handler) << browser->GetIdentifier() << frame->GetURL() << httpStatusCode;

  /// TODO: is this OK?
  const bool success = httpStatusCode < 400;
  handleLoadEnd(browser, httpStatusCode, success);
}

void PhantomJSHandler::handleLoadEnd(CefRefPtr<CefBrowser> browser, int statusCode, bool success)
{
  while (auto callback = takeCallback(&m_waitForLoadedCallbacks, browser)) {
    if (success) {
      callback->Success(std::to_string(statusCode));
    } else {
      callback->Failure(statusCode, "load error");
    }
  }

  if (auto callback = m_browserSignals.value(browser->GetIdentifier())) {
    if (success) {
      callback->Success("{\"signal\":\"onLoadFinished\",\"args\":[\"success\"]}");
    } else {
      callback->Success("{\"signal\":\"onLoadFinished\",\"args\":[\"fail\"]}");
    }
  }
}

bool PhantomJSHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
  qCDebug(handler) << browser->GetIdentifier() << m_viewRects.value(browser->GetIdentifier());
  const auto size = m_viewRects.value(browser->GetIdentifier(), qMakePair(800, 600));
  rect.Set(0, 0, size.first, size.second);
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
  qCDebug(handler) << force_close;

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
}

bool PhantomJSHandler::OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                               int64 query_id, const CefString& request, bool persistent,
                               CefRefPtr<Callback> callback)
{
  CEF_REQUIRE_UI_THREAD();

  const auto data = QByteArray(request.ToString().data());

  QJsonParseError error;
  const auto json = QJsonDocument::fromJson(data, &error).object();
  if (error.error) {
    qCWarning(handler) << error.errorString();
    return false;
  }

  const auto type = json.value(QStringLiteral("type")).toString();

  if (type == QLatin1String("createBrowser")) {
    auto subBrowser = createBrowser("about:blank", false);
    callback->Success(std::to_string(subBrowser->GetIdentifier()));
    return true;
  } else if (type == QLatin1String("webPageSignals")) {
    const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
    m_browserSignals[subBrowserId] = callback;
    Q_ASSERT(persistent);
    return true;
  } else if (type == QLatin1String("returnEvaluateJavaScript")) {
    auto otherQueryId = json.value(QStringLiteral("queryId")).toInt(-1);
    auto it = m_pendingQueryCallbacks.find(otherQueryId);
    if (it != m_pendingQueryCallbacks.end()) {
      auto exception = json.value(QStringLiteral("exception"));
      auto otherCallback = it.value();
      if (!exception.isUndefined()) {
        otherCallback->Failure(1, exception.toString().toStdString());
      } else {
        auto retval = json.value(QStringLiteral("retval")).toString();
        otherCallback->Success(retval.toStdString());
      }
      m_pendingQueryCallbacks.erase(it);
      callback->Success({});
      return true;
    }
  }

  const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
  CefRefPtr<CefBrowser> subBrowser = m_browsers.value(subBrowserId);
  if (!subBrowser) {
    qCWarning(handler) << "Unknown browser with id" << subBrowserId << "for request" << json;
    return false;
  }

  // below, all queries work on a browser
  if (type == QLatin1String("openWebPage")) {
    const auto url = json.value(QStringLiteral("url")).toString().toStdString();
    subBrowser->GetMainFrame()->LoadURL(url);
    m_waitForLoadedCallbacks.insert(subBrowser->GetIdentifier(), callback);
    return true;
  } else if (type == QLatin1String("waitForLoaded")) {
    m_waitForLoadedCallbacks.insert(subBrowser->GetIdentifier(), callback);
    return true;
  } else if (type == QLatin1String("stopWebPage")) {
    subBrowser->StopLoad();
    callback->Success({});
    return true;
  } else if (type == QLatin1String("closeWebPage")) {
    subBrowser->GetHost()->CloseBrowser(true);
    callback->Success({});
    return true;
  } else if (type == QLatin1String("evaluateJavaScript")) {
    auto code = json.value(QStringLiteral("code")).toString();
    auto url = json.value(QStringLiteral("url")).toString(QStringLiteral("phantomjs://evaluateJavaScript"));
    auto line = json.value(QStringLiteral("line")).toInt(1);
    auto args = json.value(QStringLiteral("args")).toString(QStringLiteral("[]"));
    m_pendingQueryCallbacks[query_id] = callback;
    code = "phantom.internal.handleEvaluateJavaScript(" + code + ", " + args + ", " + QString::number(query_id) + ")";
    subBrowser->GetMainFrame()->ExecuteJavaScript(code.toStdString(), url.toStdString(), line);
    return true;
  } else if (type == QLatin1String("setViewportSize")) {
    const auto width = json.value(QStringLiteral("width")).toInt(-1);
    const auto height = json.value(QStringLiteral("height")).toInt(-1);
    Q_ASSERT(width >= 0);
    Q_ASSERT(height >= 0);
    const auto newSize = qMakePair(width, height);
    auto& oldSize = m_viewRects[subBrowserId];
    if (newSize != oldSize) {
      m_viewRects[subBrowserId] = newSize;
      subBrowser->GetHost()->WasResized();
    }
    callback->Success({});
    return true;
  } else if (type == QLatin1String("renderPage")) {
    const auto path = json.value(QStringLiteral("path")).toString().toStdString();
    subBrowser->GetHost()->PrintToPDF(path, {}, makePdfPrintCallback([callback] (const CefString& path, bool success) {
      if (success) {
        callback->Success(path);
      } else {
        callback->Failure(1, std::string("failed to print to path ") + path.ToString());
      }
    }));
    return true;
  } else if (type == QLatin1String("sendEvent")) {
    const auto event = json.value(QStringLiteral("event")).toString();
    const auto modifiers = json.value(QStringLiteral("modifiers")).toInt();
    qCDebug(handler) << json << event;
    if (event == QLatin1String("keydown") || event == QLatin1String("keyup") || event == QLatin1String("keypress")) {
      CefKeyEvent keyEvent;
      keyEvent.modifiers = modifiers;
      auto arg1 = json.value(QStringLiteral("arg1"));
      if (arg1.isString()) {
        foreach (auto c, arg1.toString()) {
          keyEvent.character = c.unicode();
          keyEvent.windows_key_code = c.unicode();
          keyEvent.native_key_code = c.unicode();
          keyEvent.type = KEYEVENT_KEYDOWN;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
          keyEvent.type = KEYEVENT_CHAR;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
          keyEvent.type = KEYEVENT_KEYUP;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
        }
      } else {
        if (event == QLatin1String("keydown")) {
          keyEvent.type = KEYEVENT_KEYDOWN;
        } else if (event == QLatin1String("keyup")) {
          keyEvent.type = KEYEVENT_KEYUP;
        } else {
          keyEvent.type = KEYEVENT_CHAR;
        }
        keyEvent.windows_key_code = arg1.toInt();
        keyEvent.native_key_code = vkToNative(keyEvent.native_key_code);
        keyEvent.character = arg1.toInt();
        if (keyEvent.type != KEYEVENT_CHAR) {
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
        } else {
          keyEvent.type = KEYEVENT_KEYDOWN;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
          keyEvent.type = KEYEVENT_CHAR;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
          keyEvent.type = KEYEVENT_KEYUP;
          subBrowser->GetHost()->SendKeyEvent(keyEvent);
        }
      }
    } else if (event == QLatin1String("click") || event == QLatin1String("doubleclick")
            || event == QLatin1String("mousedown") || event == QLatin1String("mouseup"))
    {
      CefMouseEvent mouseEvent;
      mouseEvent.modifiers = modifiers;
      mouseEvent.x = json.value(QStringLiteral("arg1")).toDouble();
      mouseEvent.y = json.value(QStringLiteral("arg2")).toDouble();
      cef_mouse_button_type_t type = MBT_LEFT;
      const auto typeString = json.value(QStringLiteral("arg3")).toString();
      if (typeString == QLatin1String("right")) {
        type = MBT_RIGHT;
      } else if (typeString == QLatin1String("middle")) {
        type = MBT_MIDDLE;
      }
      if (event == QLatin1String("doubleclick")) {
        subBrowser->GetHost()->SendMouseClickEvent(mouseEvent, type, false, 2);
        subBrowser->GetHost()->SendMouseClickEvent(mouseEvent, type, true, 2);
      } else if (event == QLatin1String("click")) {
        subBrowser->GetHost()->SendMouseClickEvent(mouseEvent, type, false, 1);
        subBrowser->GetHost()->SendMouseClickEvent(mouseEvent, type, true, 1);
      } else {
        subBrowser->GetHost()->SendMouseClickEvent(mouseEvent, type, event == QLatin1String("mouseup"), 1);
      }
    } else {
      callback->Failure(1, "invalid event type passed to sendEvent: " + event.toStdString());
      return true;
    }
    callback->Success({});
    return true;
  }
  return false;
}

void PhantomJSHandler::OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                       int64 query_id)
{
  CEF_REQUIRE_UI_THREAD();

  m_waitForLoadedCallbacks.remove(browser->GetIdentifier());
  m_pendingQueryCallbacks.remove(query_id);
  m_browserSignals.remove(browser->GetIdentifier());
}
