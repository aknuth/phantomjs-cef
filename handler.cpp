// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "handler.h"

#include <sstream>
#include <string>
#include <iostream>
#include <locale>
#include <algorithm>
#include <iostream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPrinter>
#include <QRect>
#include <QImage>
#include <QBuffer>
#include <QImageWriter>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "print_handler.h"
#include "debug.h"

#include "WindowsKeyboardCodes.h"
#include "keyevents.h"

namespace {

template<typename T, typename K>
T takeCallback(QHash<K, T>* callbacks, K key)
{
  auto it = callbacks->find(key);
  if (it != callbacks->end()) {
    auto ret = it.value();
    callbacks->erase(it);
    return ret;
  }
  return {};
}

template<typename T>
T takeCallback(QHash<int32, T>* callbacks, const CefRefPtr<CefBrowser>& browser)
{
  return takeCallback(callbacks, browser->GetIdentifier());
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

cef_state_t toState(const QJsonValue& value)
{
  if (value.isBool()) {
    return value.toBool() ? STATE_ENABLED : STATE_DISABLED;
  } else if (value.isString()) {
    const auto stringValue = value.toString();
    if (!stringValue.compare(QLatin1String("on"), Qt::CaseInsensitive)
        || stringValue.compare(QLatin1String("yes"), Qt::CaseInsensitive))
    {
      return STATE_ENABLED;
    }
  }

  return STATE_DEFAULT;
}

void initBrowserSettings(CefBrowserSettings& browser_settings, bool isPhantomMain,
                         const QJsonObject& config)
{
  // TODO: make this configurable
  if (isPhantomMain) {
    browser_settings.web_security = STATE_DISABLED;
    browser_settings.universal_access_from_file_urls = STATE_ENABLED;
    browser_settings.file_access_from_file_urls = STATE_ENABLED;
  } else {
    browser_settings.web_security = toState(config.value(QStringLiteral("webSecurityEnabled")));
    browser_settings.universal_access_from_file_urls = toState(config.value(QStringLiteral("localToRemoteUrlAccessEnabled")));
    browser_settings.image_loading = toState(config.value(QStringLiteral("loadImages")));
    browser_settings.javascript = toState(config.value(QStringLiteral("javascriptEnabled")));
    browser_settings.javascript_open_windows = toState(config.value(QStringLiteral("javascriptOpenWindows")));
    browser_settings.javascript_close_windows = toState(config.value(QStringLiteral("javascriptCloseWindows")));
    /// TODO: extend
  }
}

const bool PRINT_SETTINGS = false;

void printValue(const CefString& key, const CefRefPtr<CefValue>& value) {
  switch (value->GetType()) {
    case VTYPE_INVALID:
      qDebug() << key << "invalid";
      break;
    case VTYPE_NULL:
      qDebug() << key << "null";
      break;
    case VTYPE_BOOL:
      qDebug() << key << value->GetBool();
      break;
    case VTYPE_INT:
      qDebug() << key << value->GetInt();
      break;
    case VTYPE_DOUBLE:
      qDebug() << key << value->GetDouble();
      break;
    case VTYPE_STRING:
      qDebug() << key << value->GetString();
      break;
    case VTYPE_BINARY:
      qDebug() << key << "binary";
      break;
    case VTYPE_DICTIONARY: {
      qDebug() << key << "dictionary";
      auto dict = value->GetDictionary();
      CefDictionaryValue::KeyList keys;
      dict->GetKeys(keys);
      for (const auto& subKey : keys) {
        printValue(key.ToString() + "." + subKey.ToString(), dict->GetValue(subKey));
      }
      break;
    }
    case VTYPE_LIST:
      qDebug() << key << "list";
      auto list = value->GetList();
      for (int i = 0; i < static_cast<int>(list->GetSize()); ++i) {
        printValue(key.ToString() + "[" + std::to_string(i) + "]", list->GetValue(i));
      }
      break;
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

CefRefPtr<CefBrowser> PhantomJSHandler::createBrowser(const CefString& url, bool isPhantomMain,
                                                      const QJsonObject& config)
{
  CefWindowInfo window_info;
  initWindowInfo(window_info, isPhantomMain);

  CefBrowserSettings browser_settings;
  initBrowserSettings(browser_settings, isPhantomMain, config);

  qCDebug(handler) << url << isPhantomMain << config;

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
  auto callback = m_browsers.value(browser->GetIdentifier()).signalCallback;
  if (callback) {
    QJsonArray jsonArgs;
    jsonArgs.append(QString::fromStdString(message));
    jsonArgs.append(QString::fromStdString(source));
    jsonArgs.append(line);
    QJsonObject obj;
    obj[QStringLiteral("signal")] = QStringLiteral("onConsoleMessage");
    obj[QStringLiteral("args")] = jsonArgs;
    callback->Success(QJsonDocument(obj).toJson().constData());
  } else {
    std::cerr << source << ':' << line << ": " << message << '\n';
  }
  return true;
}

void PhantomJSHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  m_browsers[browser->GetIdentifier()].browser = browser;

  if (PRINT_SETTINGS) {
    auto prefs = browser->GetHost()->GetRequestContext()->GetAllPreferences(true);
    CefDictionaryValue::KeyList keys;
    prefs->GetKeys(keys);
    for (const auto& key : keys) {
      printValue(key, prefs->GetValue(key));
    }
  }

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
  initBrowserSettings(settings, false, {});
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
    handleLoadEnd(browser, errorCode, failedUrl, false);
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

  auto callback = m_browsers.value(browser->GetIdentifier()).signalCallback;
  if (!callback) {
    return;
  }
  callback->Success("{\"signal\":\"onLoadStarted\"}");
}

void PhantomJSHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
  CEF_REQUIRE_UI_THREAD();

  qCDebug(handler) << browser->GetIdentifier() << frame->GetURL() << frame->IsMain() << httpStatusCode;

  // filter out events from sub frames
  if (!frame->IsMain()) {
    return;
  }

  /// TODO: is this OK?
  const bool success = httpStatusCode < 400;
  handleLoadEnd(browser, httpStatusCode, frame->GetURL(), success);
}

void PhantomJSHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
  qCDebug(handler) << browser->GetIdentifier() << isLoading;
}

void PhantomJSHandler::handleLoadEnd(CefRefPtr<CefBrowser> browser, int statusCode, const CefString& url, bool success)
{
  auto& browserInfo = m_browsers[browser->GetIdentifier()];
  if (!browserInfo.firstLoadFinished) {
    browserInfo.firstLoadFinished = true;
    return;
  }
  if (auto callback = browserInfo.signalCallback) {
    QJsonArray jsonArgs;
    jsonArgs.append(QString::fromStdString(url));
    jsonArgs.append(success);
    QJsonObject data;
    data[QStringLiteral("signal")] = QStringLiteral("onLoadEnd");
    data[QStringLiteral("internal")] = true;
    data[QStringLiteral("args")] = jsonArgs;
    callback->Success(QJsonDocument(data).toJson().constData());
  }

  while (auto callback = takeCallback(&m_waitForLoadedCallbacks, browser)) {
    if (success) {
      callback->Success(std::to_string(statusCode));
    } else {
      callback->Failure(statusCode, "Failed to load URL: " + url.ToString());
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
  qCDebug(handler) << browser->GetIdentifier() << type << width << height;
  auto info = takeCallback(&m_paintCallbacks, browser);
  if (info.callback) {
    QImage image(reinterpret_cast<const uchar*>(buffer), width, height, QImage::Format_ARGB32);
    if (info.clipRect.isValid()) {
      image = image.copy(info.clipRect);
    }
    if (info.format.isEmpty()) {
      if (image.save(info.path)) {
        info.callback->Success({});
      } else {
        info.callback->Failure(1, QStringLiteral("Failed to render page to \"%1\".").arg(info.path).toStdString());
      }
    } else {
      QByteArray ba;
      QBuffer buffer(&ba);
      buffer.open(QIODevice::WriteOnly);
      if (image.save(&buffer, info.format.toUtf8().constData())) {
        const auto data = ba.toBase64();
        info.callback->Success(std::string(data.constData(), data.size()));
      } else {
        std::string error = "Failed to render page into Base64 encoded buffer of format \"";
        error += qPrintable(info.format);
        error += "\". Available formats are: ";
        bool first = true;
        foreach (const auto& format, QImageWriter::supportedImageFormats()) {
          if (!first) {
            error += ", ";
          }
          error += std::string(format);
          first = false;
        }
        info.callback->Failure(1, error);
      }
    }
  }
  if (auto signalCallback = m_browsers.value(browser->GetIdentifier()).signalCallback) {
    QJsonArray jsonDirtyRects;
    for (const auto& rect : dirtyRects) {
      QJsonObject jsonRect;
      jsonRect[QStringLiteral("x")] = rect.x;
      jsonRect[QStringLiteral("y")] = rect.y;
      jsonRect[QStringLiteral("width")] = rect.width;
      jsonRect[QStringLiteral("height")] = rect.height;
      jsonDirtyRects.push_back(jsonRect);
    }
    QJsonArray jsonArgs;
    jsonArgs.append(jsonDirtyRects);
    jsonArgs.append(width);
    jsonArgs.append(height);
    jsonArgs.append(type);
    QJsonObject data;
    data[QStringLiteral("signal")] = QStringLiteral("onPaint");
    data[QStringLiteral("args")] = jsonArgs;
    signalCallback->Success(QJsonDocument(data).toJson().constData());
  }
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

template<typename T>
QJsonObject headerMapToJson(const CefRefPtr<T>& r)
{
  QJsonObject jsonHeaders;
  CefRequest::HeaderMap headers;
  r->GetHeaderMap(headers);
  for (const auto& header : headers) {
    jsonHeaders[QString::fromStdString(header.first)] = QString::fromStdString(header.second);
  }
  return jsonHeaders;
}

CefRequestHandler::ReturnValue PhantomJSHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                   CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback)
{
  auto signalCallback = m_browsers.value(browser->GetIdentifier()).signalCallback;
  if (!signalCallback) {
    return RV_CONTINUE;
  }

  QJsonArray jsonPost;
  if (const auto post = request->GetPostData()) {
    CefPostData::ElementVector elements;
    post->GetElements(elements);
    for (const auto& element : elements) {
      QJsonObject elementJson;
      elementJson[QStringLiteral("type")] = element->GetType();
      switch (element->GetType()) {
        case PDE_TYPE_BYTES: {
          QByteArray bytes;
          bytes.resize(static_cast<int>(element->GetBytesCount()));
          element->GetBytes(bytes.size(), bytes.data());
          const auto STRING_BYTES = QStringLiteral("bytes");
          elementJson[STRING_BYTES] = QString::fromUtf8(bytes.toBase64());
          break;
        }
        case PDE_TYPE_FILE: {
          const auto STRING_FILE = QStringLiteral("file");
          elementJson[STRING_FILE] = QString::fromStdString(element->GetFile().ToString());
          break;
        }
        case PDE_TYPE_EMPTY:
          break;
      }
      jsonPost.append(elementJson);
    }
  }

  QJsonObject jsonRequest;
  jsonRequest[QStringLiteral("headers")] = headerMapToJson(request);
  jsonRequest[QStringLiteral("post")] = jsonPost;
  jsonRequest[QStringLiteral("url")] = QString::fromStdString(request->GetURL().ToString());
  jsonRequest[QStringLiteral("method")] = QString::fromStdString(request->GetMethod().ToString());
  jsonRequest[QStringLiteral("flags")] = request->GetFlags();
  jsonRequest[QStringLiteral("resourceType")] = static_cast<int>(request->GetResourceType());
  jsonRequest[QStringLiteral("transitionType")] = static_cast<int>(request->GetTransitionType());

  m_requestCallbacks[request->GetIdentifier()] = {request, callback};

  QJsonArray jsonArgs;
  jsonArgs.append(jsonRequest);
  jsonArgs.append(QString::number(request->GetIdentifier()));
  QJsonObject data;
  data[QStringLiteral("signal")] = QStringLiteral("onBeforeResourceLoad");
  data[QStringLiteral("args")] = jsonArgs;
  data[QStringLiteral("internal")] = true;
  signalCallback->Success(QJsonDocument(data).toJson().constData());
  return RV_CONTINUE_ASYNC;
}

bool PhantomJSHandler::OnResourceResponse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefRequest> request, CefRefPtr<CefResponse> response)
{
  auto signalCallback = m_browsers.value(browser->GetIdentifier()).signalCallback;
  if (signalCallback) {
    QJsonObject jsonResponse;
    jsonResponse[QStringLiteral("status")] = response->GetStatus();
    jsonResponse[QStringLiteral("statusText")] = QString::fromStdString(response->GetStatusText());
    jsonResponse[QStringLiteral("contentType")] = QString::fromStdString(response->GetMimeType());
    jsonResponse[QStringLiteral("headers")] = headerMapToJson(response);
    jsonResponse[QStringLiteral("url")] = QString::fromStdString(request->GetURL());
    jsonResponse[QStringLiteral("id")] = QString::number(request->GetIdentifier());
      /// TODO: time, stage, bodySize, redirectUrl
    QJsonArray jsonArgs;
    jsonArgs.append(jsonResponse);
    QJsonObject data;
    data[QStringLiteral("signal")] = QStringLiteral("onResourceReceived");
    data[QStringLiteral("args")] = jsonArgs;
    signalCallback->Success(QJsonDocument(data).toJson().constData());
  }
  return false;
}

bool PhantomJSHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                          bool isProxy, const CefString& host, int port, const CefString& realm, const CefString& scheme,
                                          CefRefPtr<CefAuthCallback> callback)
{
  const auto& info = m_browsers.value(browser->GetIdentifier());
  if (info.authName.empty() || info.authPassword.empty()) {
    return false;
  }
  // TODO: this old PhantomJS API is really bad, we should rather delegate that to the script and delay the callback execution
  callback->Continue(info.authName, info.authPassword);
  return true;
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

  // iterate over list of values to ensure we really close all browsers
  foreach (const auto& info, m_browsers.values()) {
    info.browser->GetHost()->CloseBrowser(force_close);
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
    const auto& settings = json.value(QStringLiteral("settings")).toObject();
    auto subBrowser = createBrowser("about:blank", false, settings);
    auto& info = m_browsers[subBrowser->GetIdentifier()];
    info.authName = settings.value(QStringLiteral("userName")).toString().toStdString();
    info.authPassword = settings.value(QStringLiteral("password")).toString().toStdString();
    callback->Success(std::to_string(subBrowser->GetIdentifier()));
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
  } else if (type == QLatin1String("beforeResourceLoadResponse")) {
    const auto requestId = static_cast<uint64>(json.value(QStringLiteral("requestId")).toString().toULongLong());
    auto callback = takeCallback(&m_requestCallbacks, requestId);
    if (!callback.callback || !callback.request) {
      qCWarning(handler) << "Unknown request with id" << requestId << "for query" << json;
      return false;
    }
    const auto allow = json.value(QStringLiteral("allow")).toBool();
    if (!allow) {
      callback.callback->Continue(allow);
      return true;
    }
    auto requestData = json.value(QStringLiteral("request")).toObject();
    callback.request->SetURL(requestData.value(QStringLiteral("url")).toString().toStdString());
    CefRequest::HeaderMap headers;
    const auto& jsonHeaders = requestData.value(QStringLiteral("headers")).toObject();
    for (auto it = jsonHeaders.begin(); it != jsonHeaders.end(); ++it) {
      headers.insert(std::make_pair(it.key().toStdString(), it.value().toString().toStdString()));
    }
    callback.request->SetHeaderMap(headers);
    // TODO: post support
    callback.callback->Continue(true);
    return true;
  }

  const auto subBrowserId = json.value(QStringLiteral("browser")).toInt(-1);
  auto& subBrowserInfo = m_browsers[subBrowserId];
  const auto& subBrowser = subBrowserInfo.browser;
  if (!subBrowser) {
    qCWarning(handler) << "Unknown browser with id" << subBrowserId << "for request" << json;
    return false;
  }

  // below, all queries work on a browser
  if (type == QLatin1String("webPageSignals")) {
    subBrowserInfo.signalCallback = callback;
    Q_ASSERT(persistent);
    return true;
  } else if (type == QLatin1String("openWebPage")) {
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
  } else if (type == QLatin1String("setProperty")) {
    const auto name = json.value(QStringLiteral("name")).toString();
    const auto value = json.value(QStringLiteral("value"));
    if (name == QLatin1String("viewportSize")) {
      const auto width = value.toObject().value(QStringLiteral("width")).toInt(-1);
      const auto height = value.toObject().value(QStringLiteral("height")).toInt(-1);
      if (width < 0 || height < 0) {
        callback->Failure(1, "Invalid viewport size.");
        return true;
      } else {
        const auto newSize = qMakePair(width, height);
        auto& oldSize = m_viewRects[subBrowserId];
        if (newSize != oldSize) {
          m_viewRects[subBrowserId] = newSize;
          subBrowser->GetHost()->WasResized();
        }
      }
    } else if (name == QLatin1String("zoomFactor")) {
      const auto value = json.value(QStringLiteral("value")).toDouble(1.);
      /// TODO: this doesn't seem to work
      subBrowser->GetHost()->SetZoomLevel(value);
    } else {
      callback->Failure(1, "unknown property: " + name.toStdString());
      return true;
    }
    callback->Success({});
    return true;
  } else if (type == QLatin1String("renderImage")) {
    const auto path = json.value(QStringLiteral("path")).toString();
    const auto format = json.value(QStringLiteral("format")).toString();
    const auto clipRectJson = json.value(QStringLiteral("clipRect")).toObject();
    const auto clipRect = QRect(
      clipRectJson.value(QStringLiteral("left")).toDouble(),
      clipRectJson.value(QStringLiteral("top")).toDouble(),
      clipRectJson.value(QStringLiteral("width")).toDouble(),
      clipRectJson.value(QStringLiteral("height")).toDouble()
    );
    m_paintCallbacks[subBrowserId] = {path, format, clipRect, callback};
    subBrowser->GetHost()->Invalidate(PET_VIEW);
    return true;
  } else if (type == QLatin1String("printPdf")) {
    const auto path = json.value(QStringLiteral("path")).toString().toStdString();
    CefPdfPrintSettings settings;
    const auto paperSize = json.value(QStringLiteral("paperSize")).toObject();
    if (!paperSize.value(QStringLiteral("orientation")).toString().compare("landscape", Qt::CaseInsensitive)) {
      settings.landscape = true;
    }
    QPrinter printer;
    if (paperSize.contains(QStringLiteral("format"))) {
      printer.setPaperSize(paperSizeForName(paperSize.value(QStringLiteral("format")).toString()));
    } else if (paperSize.contains(QStringLiteral("width")) && paperSize.contains(QStringLiteral("height"))) {
      auto width = stringToPointSize(paperSize.value(QStringLiteral("width")).toString());
      auto height = stringToPointSize(paperSize.value(QStringLiteral("height")).toString());
      printer.setPaperSize({width, height}, QPrinter::Point);
    }
    auto rect = printer.paperSize(QPrinter::Millimeter);
    settings.page_height = rect.height() * 1000;
    settings.page_width = rect.width() * 1000;

    const auto margin = paperSize.value(QStringLiteral("margin"));
    if (margin.isString()) {
      const auto marginString = margin.toString();
      if (marginString == QLatin1String("default")) {
        settings.margin_type = PDF_PRINT_MARGIN_DEFAULT;
      } else if (marginString == QLatin1String("minimum")) {
        settings.margin_type = PDF_PRINT_MARGIN_MINIMUM;
      } else if (marginString == QLatin1String("none")) {
        settings.margin_type = PDF_PRINT_MARGIN_NONE;
      } else {
        settings.margin_type = PDF_PRINT_MARGIN_CUSTOM;
        int intMargin = stringToMillimeter(marginString);
        settings.margin_left = intMargin;
        settings.margin_top = intMargin;
        settings.margin_right = intMargin;
        settings.margin_bottom = intMargin;
      }
    } else if (margin.isObject()) {
      auto marginObject = margin.toObject();
      settings.margin_type = PDF_PRINT_MARGIN_CUSTOM;
      settings.margin_left = stringToMillimeter(marginObject.value(QStringLiteral("left")).toString());
      settings.margin_top = stringToMillimeter(marginObject.value(QStringLiteral("top")).toString());
      settings.margin_right = stringToMillimeter(marginObject.value(QStringLiteral("right")).toString());
      settings.margin_bottom = stringToMillimeter(marginObject.value(QStringLiteral("bottom")).toString());
    }
    qCDebug(print) << paperSize << printer.paperSize() << settings.page_height << settings.page_width << "landscape:" << settings.landscape
                    << "margins:"<< settings.margin_bottom << settings.margin_left << settings.margin_top << settings.margin_right << "margin type:" << settings.margin_type;
    subBrowser->GetHost()->PrintToPDF(path, settings, makePdfPrintCallback([callback] (const CefString& path, bool success) {
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
  m_paintCallbacks.remove(browser->GetIdentifier());
}
