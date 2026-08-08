// SPDX-FileCopyrightText: 2024-2026 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrentinfoview.hpp"

#include <QApplication>
#include <QCommandLineParser>

#include <libtorrent/load_torrent.hpp>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  QApplication::setApplicationName("torrent_view");
  QApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Torrent viewer");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "file to view");

  parser.process(app);

  TorrentInfoView wnd;
  if (const auto args = parser.positionalArguments(); !args.empty()) {
    auto ti = lt::load_torrent_file(args.first().toStdString());
    wnd.setTorrentInfo(ti);
  }
  wnd.show();
  return app.exec();
}
