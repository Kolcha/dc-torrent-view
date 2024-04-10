// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>

#include <libtorrent/torrent_info.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class TorrentInfoView; }
QT_END_NAMESPACE

class TorrentFilesModel;
class TorrentTrackersModel;

class TorrentInfoView : public QWidget
{
  Q_OBJECT

public:
  TorrentInfoView(QWidget* parent = nullptr);
  ~TorrentInfoView();

  void setTorrentInfo(const lt::torrent_info& ti);

private:
  Ui::TorrentInfoView* ui;
  TorrentFilesModel* files;
  TorrentTrackersModel* trackers;
};
