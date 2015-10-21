// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "app.h"
#include <QDir>

#include <QFile>
#include <QUrl>
#include <string>
#include <iostream>
#include <fstream>

#include "handler.h"
#include "print_handler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"

PhantomJSApp::PhantomJSApp()
  : m_printHandler(new PrintHandler)
  , m_messageRouter(CefMessageRouterRendererSide::Create(PhantomJSHandler::messageRouterConfig()))
{
}

PhantomJSApp::~PhantomJSApp()
{
}

void PhantomJSApp::OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
{
  // overwrite settings of the file scheme to allow extended access to it
  // without this, the about:blank page cannot load the user provided script
  // TODO: check whether that allso allows remote files access to local ones...
  registrar->AddCustomScheme("file", false, true, true);
}

void PhantomJSApp::OnContextInitialized()
{
  CEF_REQUIRE_UI_THREAD();

  // PhantomJSHandler implements browser-level callbacks.
  CefRefPtr<PhantomJSHandler> handler(new PhantomJSHandler());

  // Create the first browser window with empty content to get our hands on a frame
  auto browser = handler->createBrowser("about:blank");
  auto frame = browser->GetMainFrame();

  // now inject user provided js file
  std::ostringstream content;
  content << "<html><head>";
  auto command_line = CefCommandLine::GetGlobalCommandLine();
  if (command_line->HasArguments()) {
    CefCommandLine::ArgumentList arguments;
    command_line->GetArguments(arguments);
    for (auto argument : arguments) {
      auto url = QUrl::fromUserInput(QString::fromStdString(argument), QDir::currentPath(), QUrl::AssumeLocalFile);
      content << "<script type=\"text/javascript\" src=\"" << url.toString().toStdString() << "\"></script>";
    }
  }
  content << "</head><body></body></html>";
  frame->LoadString(content.str(), "phantomjs://initialize");
}

CefRefPtr<CefPrintHandler> PhantomJSApp::GetPrintHandler()
{
  std::cerr << "print handler accessed\n";
  return m_printHandler;
}

namespace {
class V8Handler : public CefV8Handler
{
public:
  bool Execute(const CefString &name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList &arguments, CefRefPtr<CefV8Value> &retval,
               CefString &exception) OVERRIDE
  {
    if (name == "exit") {
      CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, CefProcessMessage::Create("exit"));
      return true;
    }
    exception = std::string("Unknown PhantomJS function: ") + name.ToString();
    return true;
    std::cerr << "V8Handler: " << name << '\t' << object << '\t' << arguments.size() << '\t' << exception;
    return false;
  }
private:
  IMPLEMENT_REFCOUNTING(V8Handler);
};
}

void PhantomJSApp::OnWebKitInitialized()
{
  QFile file(":/webkit_extension.js");
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  std::string extensionCode = file.readAll().toStdString();

  // Create an instance of my CefV8Handler object.
  CefRefPtr<CefV8Handler> handler = new V8Handler();

  // Register the extension.
  CefRegisterExtension("v8/phantomjs", extensionCode, handler);
}

void PhantomJSApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context)
{
  auto global = context->GetGlobal();
  auto phantom = global->GetValue("phantom");

  // forward extension code into global namespace
  global->SetValue("require", phantom->GetValue("require"), V8_PROPERTY_ATTRIBUTE_NONE);

  m_messageRouter->OnContextCreated(browser, frame, context);
}

void PhantomJSApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context)
{
  m_messageRouter->OnContextReleased(browser, frame, context);
}

bool PhantomJSApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
  std::cerr << "app got message: " << message->GetName() << '\n';
  return m_messageRouter->OnProcessMessageReceived(browser, source_process, message);
}
