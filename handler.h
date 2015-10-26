// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CEF_TESTS_PHANTOMJS_HANDLER_H_
#define CEF_TESTS_PHANTOMJS_HANDLER_H_

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

#include <vector>
#include <QHash>

class PhantomJSHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefRenderHandler,
                      public CefRequestHandler,
                      public CefMessageRouterBrowserSide::Handler
{
 public:
  PhantomJSHandler();
  ~PhantomJSHandler();

  // Provide access to the single global instance of this object.
  static CefMessageRouterConfig messageRouterConfig();

  CefRefPtr<CefBrowser> createBrowser(const CefString& url);

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
  {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
  {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
  {
    return this;
  }
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override
  {
    return this;
  }
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override
  {
    return this;
  }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) override;
  virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                             const CefString& message,
                             const CefString& source,
                             int line) override;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) override;
  void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;
  void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

  // CefRenderHandler methods:
  virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer, int width, int height) override;

  // CefRequestHandler methods:
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status) override;
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request, bool is_redirect) override;

  // CefMessageRouterBrowserSide::Handler methods:
  bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
               int64 query_id, const CefString & request, bool persistent,
               CefRefPtr<Callback> callback) override;
  void OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       int64 query_id) override;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

 private:
  // List of existing browser windows. Only accessed on the CEF UI thread.
  QHash<int, CefRefPtr<CefBrowser>> m_browsers;

  bool m_isClosing;

  CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
  // NOTE: using QHash prevents a strange ABI issue discussed here: http://www.magpcss.org/ceforum/viewtopic.php?f=6&t=13543
  QHash<int32, CefRefPtr<CefMessageRouterBrowserSide::Callback>> m_pendingOpenBrowserRequests;
  QHash<int64, CefRefPtr<CefMessageRouterBrowserSide::Callback>> m_pendingQueryCallbacks;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PhantomJSHandler);
};

#endif  // CEF_TESTS_PHANTOMJS_HANDLER_H_
