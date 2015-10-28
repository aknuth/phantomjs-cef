// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "app.h"

#include <QDir>
#include <QFile>

#include <string>
#include <iostream>
#include <fstream>

#include "handler.h"
#include "print_handler.h"
#include "debug.h"

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

  auto command_line = CefCommandLine::GetGlobalCommandLine();
  CefCommandLine::ArgumentList arguments;
  command_line->GetArguments(arguments);
  auto scriptFileInfo = QFileInfo(QString::fromStdString(arguments.front()));
  const auto scriptPath = scriptFileInfo.absoluteFilePath().toStdString();

  // now inject user provided js file and some meta data such as cli arguments
  // which are otherwise not available on the renderer process
  std::ostringstream content;
  content << "<html><head>\n";
  content << "<script type\"text/javascript\">\n";
  // forward extension code into global namespace
  content << "window.onerror = phantom.internal.propagateOnError;\n";
  content << "window.require = phantom.require;\n";
  // send arguments to script
  content << "phantom.args = [";
  for (const auto& arg : arguments) {
    content << '\"' << arg << "\",";
  }
  content << "];\n";
  // default initialize the library path to the folder of the script that will be executed
  content << "phantom.libraryPath = \"" << scriptFileInfo.absolutePath().toStdString() << "\";\n";
  content <<"</script>\n";
  // then load the actual script
  content << "<script type=\"text/javascript\" src=\"file://" << scriptPath << "\" onerror=\"phantom.internal.onScriptLoadError();\"></script>\n";
  content << "</head><body></body></html>";
  frame->LoadString(content.str(), "phantomjs://" + scriptPath);
}

CefRefPtr<CefPrintHandler> PhantomJSApp::GetPrintHandler()
{
  return m_printHandler;
}

namespace {

std::string findLibrary(const QString& filePath, const QString& libraryPath)
{
  for (const auto& path : {QDir::currentPath(), libraryPath}) {
    QFileInfo info(path + '/' + filePath);
    if (!info.isFile() || !info.isReadable()) {
      continue;
    }
    return info.absoluteFilePath().toStdString();
  }
  return {};
}

std::string readFile(const std::string& filePath)
{
  std::ifstream in(filePath);
  if (!in) {
    return {};
  }
  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  return contents;
}

class V8Handler : public CefV8Handler
{
public:
  bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exception) override
  {
    if (name == "exit") {
      CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, CefProcessMessage::Create("exit"));
      return true;
    } else if (name == "printError" && !arguments.empty()) {
      std::cerr << arguments.at(0)->GetStringValue() << '\n';
      return true;
    } else if (name == "findLibrary") {
      const auto filePath = QString::fromStdString(arguments.at(0)->GetStringValue());
      const auto libraryPath = QString::fromStdString(arguments.at(1)->GetStringValue());
      retval = CefV8Value::CreateString(findLibrary(filePath, libraryPath));
      return true;
    } else if (name == "readFile") {
      const auto file = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateString(readFile(file));
      return true;
    } else if (name == "executeJavaScript") {
      const auto code = arguments.at(0)->GetStringValue();
      const auto file = arguments.at(1)->GetStringValue();
      CefV8Context::GetCurrentContext()->GetFrame()->ExecuteJavaScript(code, "file://" + file.ToString(), 1);
      return true;
    }
    exception = std::string("Unknown PhantomJS function: ") + name.ToString();
    return true;
  }
private:
  IMPLEMENT_REFCOUNTING(V8Handler);
};
}

void PhantomJSApp::OnWebKitInitialized()
{
  CefRefPtr<CefV8Handler> handler = new V8Handler();

  const auto modules = QDir(":/phantomjs/modules").entryInfoList(QDir::NoFilter, QDir::Name);
  if (modules.isEmpty()) {
    qFatal("No modules found. This is a setup issue with the resource system - try to run CMake again.");
  }
  foreach (const auto& module, modules) {
    QFile file(module.absoluteFilePath());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    const auto extensionCode = file.readAll();
    if (extensionCode.isEmpty()) {
      qFatal("Module \"%s\" is empty. This is a setup issue with the resource system - try to run CMake again.", qPrintable(module.absoluteFilePath()));
    }
    CefRegisterExtension(file.fileName().toStdString(), std::string(extensionCode.constData(), extensionCode.size()), handler);
  }
}

void PhantomJSApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefV8Context> context)
{
  m_messageRouter->OnContextCreated(browser, frame, context);
}

void PhantomJSApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context)
{
  m_messageRouter->OnContextReleased(browser, frame, context);
}

bool PhantomJSApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                            CefRefPtr<CefProcessMessage> message)
{
  return m_messageRouter->OnProcessMessageReceived(browser, source_process, message);
}
