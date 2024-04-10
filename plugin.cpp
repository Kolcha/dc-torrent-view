// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "wlxplugin.h"

#include "torrentinfoview.hpp"

HANDLE DCPCALL ListLoad(HANDLE parent_win, char* file_to_load, int show_flags)
{
  Q_UNUSED(show_flags);

  lt::torrent_info ti(file_to_load);
  if (!ti.is_valid())
    return nullptr;

  auto view = new TorrentInfoView(reinterpret_cast<QWidget*>(parent_win));
  view->setTorrentInfo(ti);
  view->show();
  return view;
}

void DCPCALL ListCloseWindow(HWND list_win)
{
  delete reinterpret_cast<TorrentInfoView*>(list_win);
}

void DCPCALL ListGetDetectString(char* detect_string, int maxlen)
{
  snprintf(detect_string, maxlen - 1, "EXT=\"TORRENT\"");
}
