// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef CEF_TESTS_PHANTOMJS_HANDLER_H_
#define CEF_TESTS_PHANTOMJS_HANDLER_H_

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

#include <list>
#include <unordered_map>

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
  static PhantomJSHandler* GetInstance();
  static CefMessageRouterConfig messageRouterConfig();

  CefRefPtr<CefBrowser> createBrowser(const CefString& url);

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE
  {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
  {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE
  {
    return this;
  }
  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE
  {
    return this;
  }
  virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE
  {
    return this;
  }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;
  virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                             const CefString& message,
                             const CefString& source,
                             int line) OVERRIDE;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;
  virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                    bool isLoading,
                                    bool canGoBack,
                                    bool canGoForward) OVERRIDE;

  // CefRenderHandler methods:
  virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
  virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                       PaintElementType type,
                       const RectList& dirtyRects,
                       const void* buffer, int width, int height) OVERRIDE;

  // CefRequestHandler methods:
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status) OVERRIDE;
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request, bool is_redirect) OVERRIDE;

  // CefMessageRouterBrowserSide::Handler methods:
  bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
               int64 query_id, const CefString & request, bool persistent,
               CefRefPtr<Callback> callback) OVERRIDE;
  void OnQueryCanceled(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       int64 query_id) OVERRIDE;

  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

 private:
  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
  std::unordered_map<int, CefRefPtr<CefMessageRouterBrowserSide::Callback>> m_pendingOpenBrowserRequests;
  CefRefPtr<CefMessageRouterBrowserSide::Callback> m_callback;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(PhantomJSHandler);
};

#endif  // CEF_TESTS_PHANTOMJS_HANDLER_H_
