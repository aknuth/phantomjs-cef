// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <QGuiApplication>

#include "app.h"

#include "include/base/cef_logging.h"

#include <iostream>

// Entry point function for all processes.
int main(int argc, char* argv[])
{
  // Provide CEF with command-line arguments.
  CefMainArgs main_args(argc, argv);

  // PhantomJSApp implements application-level callbacks. It will create the first
  // browser instance in OnContextInitialized() after CEF has initialized.
  CefRefPtr<PhantomJSApp> app(new PhantomJSApp);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, app, NULL);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  if (argc < 2) {
    std::cerr << "Missing script parameter.\n";
    return 1;
  }

  // NOTE: we do not run the Qt eventloop, so don't use signals/slots or similar
  // we mostly integrate Qt for its painting API and the resource system
  QGuiApplication qtApp(argc, argv);

  // Specify CEF global settings here.
  CefSettings settings;
  // TODO: make this configurable like previously in phantomjs
  settings.ignore_certificate_errors = true;
  settings.remote_debugging_port = 12345;
  settings.windowless_rendering_enabled = true;
  settings.no_sandbox = true;
//   settings.log_severity = LOGSEVERITY_VERBOSE;

  // Initialize CEF for the browser process.
  CefInitialize(main_args, settings, app, NULL);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}
