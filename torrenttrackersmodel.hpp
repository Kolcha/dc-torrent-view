// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QAbstractTableModel>

#include <libtorrent/torrent_info.hpp>

class TorrentTrackersModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit TorrentTrackersModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  void setTorrentInfo(const lt::torrent_info& ti);

private:
  std::vector<std::pair<int, QString>> _trackers;
  std::vector<QString> _web_seeds;
};
