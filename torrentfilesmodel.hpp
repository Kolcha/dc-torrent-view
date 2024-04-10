// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include <libtorrent/torrent_info.hpp>

class FileNode
{
public:
  FileNode(QString filename, qint64 size)
    : _filename(filename)
    , _file_size(size)
  {}

  explicit FileNode(QString filename)
    : FileNode(filename, 0)
  {}

  FileNode() : FileNode(QString()) {}

  QString name() const { return _filename; }
  qint64 size() const { return _file_size; }

  FileNode* parent() const { return _parent; }
  const auto& nodes() const { return _nodes; }

  void setParent(FileNode* p) { _parent = p; }
  void addNode(std::unique_ptr<FileNode> n)
  {
    n->setParent(this);
    _nodes.push_back(std::move(n));
  }

  void setSize(qint64 size) { _file_size = size; }
  void updateSize();

private:
  QString _filename;
  qint64 _file_size;

  FileNode* _parent = nullptr;
  std::vector<std::unique_ptr<FileNode>> _nodes;
};

class TorrentFilesModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit TorrentFilesModel(QObject* parent = nullptr);

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  void setTorrentInfo(const lt::torrent_info& ti);

private:
  std::unique_ptr<FileNode> _root;
};

class SortDirsFirstProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  using QSortFilterProxyModel::QSortFilterProxyModel;

protected:
  bool lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const override;
};
