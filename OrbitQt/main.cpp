// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QStyleFactory>

#include "App.h"
#include "ApplicationOptions.h"
#include "CrashHandler.h"
#include "CrashOptions.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitStartupWindow.h"
#include "Path.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "orbitmainwindow.h"
#include "version.h"

ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");

// SSH Tunneling via run_service_ssh.{ps1/sh} is the default for now. To make
// this work the remote is set here to localhost. This will also disable the
// StartUpWindow for now.
// TODO(antonrohr): replace "localhost" with "" as soon as ssh is implemented
ABSL_FLAG(std::string, remote, "localhost",
          "Connect to the specified remote on startup");
ABSL_FLAG(uint16_t, asio_port, 44766,
          "The service's Asio tcp_server port (use default value if unsure)");
ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");

// TODO: remove this once we deprecated legacy parameters
static void ParseLegacyCommandLine(int argc, char* argv[],
                                   ApplicationOptions* options) {
  for (size_t i = 0; i < static_cast<size_t>(argc); ++i) {
    const char* arg = argv[i];
    if (absl::StartsWith(arg, "gamelet:")) {
      std::cerr << "ERROR: the 'gamelet:<host>:<port>' option is deprecated "
                   "and will be removed soon, please use -remote <host> "
                   "instead."
                << std::endl;

      options->asio_server_address = arg + std::strlen("gamelet:");
    }
  }
}

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("CPU Profiler");
  absl::ParseCommandLine(argc, argv);
#if __linux__
  QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif

  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("Orbit Profiler");
  QCoreApplication::setApplicationVersion(OrbitQt::kVersionString);

  const std::string dump_path = Path::GetDumpPath();
#ifdef _WIN32
  const char* handler_name = "crashpad_handler.exe";
#else
  const char* handler_name = "crashpad_handler";
#endif
  const std::string handler_path = QDir(QCoreApplication::applicationDirPath())
                                       .absoluteFilePath(handler_name)
                                       .toStdString();
  const std::string crash_server_url = CrashServerOptions::GetUrl();
  CrashHandler crash_handler(dump_path, handler_path, crash_server_url);

  ApplicationOptions options;

  ParseLegacyCommandLine(argc, argv, &options);
  std::string remote = absl::GetFlag(FLAGS_remote);
  uint16_t asio_port = absl::GetFlag(FLAGS_asio_port);
  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);

  if (!remote.empty()) {
    if (absl::StrContains(remote, ":")) {
      // TODO: Replace this with grpc_address once everything migrated to grpc
      std::cerr
          << "It seems like you specified a port in your remote address: "
             "since the service is currently listening on two different ports, "
             "please use --asio_port/--grpc_port options to specify the "
             "corresponding ports and remove the port from --remote."
          << std::endl;
      return -1;
    }

    options.asio_server_address = absl::StrFormat("%s:%i", remote, asio_port);
    options.grpc_server_address = absl::StrFormat("%s:%i", remote, grpc_port);
  }

  app.setStyle(QStyleFactory::create("Fusion"));

  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  app.setPalette(darkPalette);
  app.setStyleSheet(
      "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid "
      "white; }");

  if (options.asio_server_address.empty()) {
    OrbitStartupWindow sw;
    OrbitSsh::Credentials ssh_credentials;
    int dialog_result = sw.Run(&ssh_credentials);
    if (dialog_result == 0) return 0;

#ifdef __linux__
    options.asio_server_address =
        absl::StrFormat("%s:%d", ssh_credentials.host, asio_port);
    options.grpc_server_address =
        absl::StrFormat("%s:%d", ssh_credentials.host, grpc_port);
#else
    // TODO(antonrohr) remove this ifdef as soon as the collector works on
    // windows
    if (ssh_credentials.host != "127.0.0.1") {
      options.asio_server_address =
          absl::StrFormat("%s:%d", ssh_credentials.host, asio_port);
      options.grpc_server_address =
          absl::StrFormat("%s:%d", ssh_credentials.host, grpc_port);
    }
#endif
  }

  OrbitMainWindow w(&app, std::move(options));

  if (!w.IsHeadless()) {
    w.showMaximized();
  } else {
    w.show();
    w.hide();
  }

  w.PostInit();

  int error_code = app.exec();

  GOrbitApp->OnExit();

  return error_code;
}
