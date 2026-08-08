// SPDX-FileCopyrightText: 2024-2026 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrenttrackersmodel.hpp"

TorrentTrackersModel::TorrentTrackersModel(QObject* parent)
  : QAbstractTableModel(parent)
{
}

int TorrentTrackersModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return _trackers.size() + _web_seeds.size();
}

int TorrentTrackersModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return 2;
}

QVariant TorrentTrackersModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  switch (index.column()) {
    case 0: {
      switch (role) {
        case Qt::DisplayRole:
          return index.row() < _trackers.size() ?
                 QString::number(_trackers[index.row()].first) : QString("WEB");
        case Qt::TextAlignmentRole:
          return static_cast<int>(Qt::AlignVCenter | Qt::AlignRight);
        default:
          break;
      }
      break;
    }
    case 1: {
      switch (role) {
        case Qt::DisplayRole:
          return index.row() < _trackers.size() ?
                 _trackers[index.row()].second :
                 _web_seeds[index.row() - _trackers.size()];
        default:
          break;
      }
      break;
    }
    default:
      break;
  }

  return QVariant();
}

void TorrentTrackersModel::setTorrentInfo(const lt::add_torrent_params& atp)
{
  beginResetModel();

  _trackers.clear();
  _web_seeds.clear();

  _trackers.reserve(atp.trackers.size());
  int tier = 0;
  for (std::size_t i = 0; i < atp.trackers.size(); ++i) {
    if (i < atp.tracker_tiers.size()) tier = atp.tracker_tiers[i];
    _trackers.emplace_back(tier, QString::fromStdString(atp.trackers[i]));
  }

  _web_seeds.reserve(atp.url_seeds.size());
  for (const auto& url : atp.url_seeds) {
    _web_seeds.push_back(QString::fromStdString(url));
  }

  endResetModel();
}
