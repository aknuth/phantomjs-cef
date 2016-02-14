// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "app.h"

#include <QDir>
#include <QFile>
#include <QDateTime>

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
  auto browser = handler->createBrowser("about:blank", true);
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
  contents.assign(std::istreambuf_iterator<char>(in),
                  std::istreambuf_iterator<char>());
  return contents;
}

bool writeFile(const std::string& filePath, const std::string& contents, const std::string& m)
{
  std::ios_base::openmode mode = std::ios_base::out;
  if (m.find('b') != std::string::npos || m.find('B') != std::string::npos) {
    mode |= std::ios_base::binary;
  }
  if (m.find('a') != std::string::npos || m.find('A') != std::string::npos || m.find('+') != std::string::npos) {
    mode |= std::ios_base::app;
  } else {
    mode |= std::ios_base::trunc;
  }
  std::ofstream out(filePath, mode);
  if (out) {
    out << contents;
    return true;
  }
  return false;
}

bool remove(const QString& path)
{
  QFileInfo info(path);
  if (info.isDir()){
    QDir dir(path);
    return dir.removeRecursively();
  }
  QFile f(path);
  return f.remove();
}

bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
  QFileInfo srcFileInfo(srcFilePath);
  if (srcFileInfo.isDir()) {
    if (!QDir().mkpath(tgtFilePath)) {
      return false;
    }

    QDir sourceDir(srcFilePath);
    QDir::Filters sourceDirFilter = QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks | QDir::Hidden;
    foreach (const QFileInfo& entry, sourceDir.entryInfoList(sourceDirFilter, QDir::DirsFirst)) {
      const QString destination = tgtFilePath + '/' + entry.fileName();
      if (!copyRecursively(entry.absoluteFilePath(), destination)) {
        return false;
      }
    }
    return true;
  } else {
    QFileInfo tgtFileInfo(tgtFilePath);
    QFileInfo srcFileInfo(srcFilePath);
    if (tgtFileInfo.isDir()){
      return copyRecursively(srcFilePath, tgtFilePath + '/' + srcFileInfo.fileName());
    }
    return QFile::copy(srcFilePath, tgtFilePath);
  }
}

class V8Handler : public CefV8Handler
{
public:
  bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
               const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval,
               CefString& exception) override
  {
    auto context = CefV8Context::GetCurrentContext();
    static const std::string phantomjs_scheme = "phantomjs://";
    const auto frameURL = context->GetFrame()->GetURL().ToString();
    if (context->GetBrowser()->GetIdentifier() != 1 || frameURL.compare(0, phantomjs_scheme.size(), phantomjs_scheme)) {
      exception = "Access to PhantomJS function \"" + name.ToString() + "\" not allowed from URL \"" + frameURL + "\".";
      return true;
    }
    if (name == "exit") {
      context->GetBrowser()->SendProcessMessage(PID_BROWSER, CefProcessMessage::Create("exit"));
      return true;
    } else if (name == "printError" && !arguments.empty()) {
      std::cerr << arguments.at(0)->GetStringValue() << '\n';
      return true;
    } else if (name == "findLibrary") {
      const auto filePath = QString::fromStdString(arguments.at(0)->GetStringValue());
      const auto libraryPath = QString::fromStdString(arguments.at(1)->GetStringValue());
      retval = CefV8Value::CreateString(findLibrary(filePath, libraryPath));
      return true;
    } else if (name == "executeJavaScript") {
      const auto code = arguments.at(0)->GetStringValue();
      const auto file = arguments.at(1)->GetStringValue();
      context->GetFrame()->ExecuteJavaScript(code, "file://" + file.ToString(), 1);
      return true;
    } else if (name == "write") {
      const auto filename = arguments.at(0)->GetStringValue();
      const auto contents = arguments.at(1)->GetStringValue();
      const auto mode = arguments.at(2)->GetStringValue();
      retval = CefV8Value::CreateBool(writeFile(filename, contents, mode));
      return true;
    } else if (name == "read" || name == "readFile") {
      const auto file = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateString(readFile(file));
      return true;
    } else if (name == "touch"){
      const auto filename = arguments.at(0)->GetStringValue();
      QFile f(QString::fromStdString(filename.ToString()));
      retval = CefV8Value::CreateBool(!f.open(QFile::WriteOnly));
      return true;
    } else if (name == "makeDirectory"){
      const auto path = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateBool(QDir().mkdir(QString::fromStdString(path.ToString())));
      return true;
    } else if (name == "makeTree"){
      const auto path = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateBool(QDir().mkpath(QString::fromStdString(path.ToString())));
      return true;
    } else if (name == "tempPath"){
      retval = CefV8Value::CreateString(QDir::tempPath().toStdString());
      return true;
    } else if (name == "lastModified") {
      const auto filename = arguments.at(0)->GetStringValue();
      auto lastModified = QFileInfo(QString::fromStdString(filename.ToString())).lastModified();
      retval = CefV8Value::CreateString(lastModified.toUTC().toString(Qt::ISODate).toStdString());
      return true;
    } else if (name == "exists") {
      const auto filename = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateBool(QFile::exists(QString::fromStdString(filename.ToString())));
      return true;
    } else if (name == "isFile") {
      const auto filename = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateBool(QFileInfo(QString::fromStdString(filename.ToString())).isFile());
      return true;
    } else if (name == "isDirectory") {
      const auto filename = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateBool(QFileInfo(QString::fromStdString(filename.ToString())).isDir());
      return true;
    } else if (name == "copy") {
      const auto src = arguments.at(0)->GetStringValue();
      const auto dest = arguments.at(1)->GetStringValue();
      bool r = copyRecursively(QString::fromStdString(src), QString::fromStdString(dest));
      retval = CefV8Value::CreateBool(r);
      return true;
    } else if (name == "remove") {
      const auto src = arguments.at(0)->GetStringValue();
      bool r = remove(QString::fromStdString(src));
      retval = CefV8Value::CreateBool(r);
      return true;
    } else if (name == "size") {
      const auto filename = arguments.at(0)->GetStringValue();
      retval = CefV8Value::CreateInt(QFileInfo(QString::fromStdString(filename.ToString())).size());
      return true;
    } else if (name == "list") {
      const auto path = arguments.at(0)->GetStringValue();
      const auto entries = QDir(QString::fromStdString(path)).entryList();
      CefRefPtr<CefV8Value> arr = CefV8Value::CreateArray(entries.size());
      for (int n = 0; n < entries.size(); n++) {
        arr->SetValue(n, CefV8Value::CreateString(entries.at(n).toStdString()));
      }
      retval = arr;
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
