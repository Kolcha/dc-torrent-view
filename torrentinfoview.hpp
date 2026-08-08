// SPDX-FileCopyrightText: 2024-2026 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QWidget>

#include <libtorrent/add_torrent_params.hpp>

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

  void setTorrentInfo(const lt::add_torrent_params& atp);

private:
  Ui::TorrentInfoView* ui;
  TorrentFilesModel* files;
  TorrentTrackersModel* trackers;
};
