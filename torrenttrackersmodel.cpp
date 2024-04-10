// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
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

void TorrentTrackersModel::setTorrentInfo(const libtorrent::torrent_info& ti)
{
  beginResetModel();

  _trackers.clear();
  _web_seeds.clear();

  _trackers.reserve(ti.trackers().size());
  for (const auto& e : ti.trackers()) {
    _trackers.emplace_back(e.tier, QString::fromStdString(e.url));
  }

  _web_seeds.reserve(ti.web_seeds().size());
  for (const auto& e : ti.web_seeds()) {
    _web_seeds.push_back(QString::fromStdString(e.url));
  }

  endResetModel();
}
