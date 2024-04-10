// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrentfilesmodel.hpp"

#include <QIcon>

#include "utils.hpp"

namespace {

FileNode* add_path(FileNode& root, QStringView path)
{
  FileNode* node = &root;
  const auto parts = path.split('/');
  for (auto name : parts) {
    if (auto iter = std::ranges::find(node->nodes(), name, &FileNode::name);
        iter != node->nodes().end()) {
      auto idx = std::distance(node->nodes().begin(), iter);
      node = node->nodes()[idx].get();
    } else {
      node->addNode(std::make_unique<FileNode>(name.toString()));
      node = node->nodes().back().get();
    }
  }
  return node;
}

} // namespace

void FileNode::updateSize()
{
  if (_nodes.empty()) {
    return;
  }

  std::ranges::for_each(_nodes, &FileNode::updateSize);
  auto get_size = [](auto&& n) { return n->size(); };
  _file_size = std::transform_reduce(_nodes.begin(), _nodes.end(),
                                     0LL, std::plus<>(), get_size);
}

TorrentFilesModel::TorrentFilesModel(QObject* parent)
  : QAbstractItemModel(parent)
  , _root(std::make_unique<FileNode>())
{
}

QVariant TorrentFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal) {
    return QVariant();
  }

  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
    case 0:
      return tr("Name");
    case 1:
      return tr("Size");
    default:
      break;
  }

  return QVariant();
}

QModelIndex TorrentFilesModel::index(int row, int column, const QModelIndex& parent) const
{
  auto p = parent.isValid() ?
           reinterpret_cast<FileNode*>(parent.internalPointer()) :
           _root.get();   // top-level items have no parent index
  Q_ASSERT(p);
  if (row < 0 || row >= p->nodes().size()) { // out of range
    return QModelIndex();
  }

  return createIndex(row, column, p->nodes()[row].get());
}

QModelIndex TorrentFilesModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) { // invalid index
    return QModelIndex();
  }

  // get parent node
  auto p = reinterpret_cast<FileNode*>(index.internalPointer())->parent();
  if (!p) { // root node has no parent
    Q_ASSERT(reinterpret_cast<FileNode*>(index.internalPointer()) == _root.get());
    return QModelIndex();
  }

  // top-level items should not have parent index
  if (p == _root.get()) {
    return QModelIndex();
  }

  // find the row index in parent's parent
  auto pp = p->parent();
  Q_ASSERT(pp);   // all nodes at this point must have parent
  auto iter = std::ranges::find(pp->nodes(), p, &std::unique_ptr<FileNode>::get);
  Q_ASSERT(iter != pp->nodes().end());
  int row = std::distance(pp->nodes().begin(), iter);
  return createIndex(row, 0, p);
}

int TorrentFilesModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) {          // top-level items
    return _root->nodes().size();
  }

  auto p = reinterpret_cast<FileNode*>(parent.internalPointer());
  Q_ASSERT(p);
  return p->nodes().size();
}

int TorrentFilesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant TorrentFilesModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  auto node = reinterpret_cast<FileNode*>(index.internalPointer());
  Q_ASSERT(node);

  switch (index.column()) {
    case 0: {
      switch (role) {
        case Qt::DecorationRole:
          return QIcon::fromTheme(node->nodes().empty() ? "unknown" : "folder");
        case Qt::DisplayRole:
        case Qt::EditRole:
          return node->name();
        default:
          break;
      }
      break;
    }

    case 1: {
      switch (role) {
        case Qt::DisplayRole:
          return human_size(node->size(), 1000);
        case Qt::EditRole:
          return node->size();
        case Qt::TextAlignmentRole:
          return static_cast<int>(Qt::AlignVCenter | Qt::AlignRight);
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

void TorrentFilesModel::setTorrentInfo(const libtorrent::torrent_info& ti)
{
  beginResetModel();
  _root = std::make_unique<FileNode>();
  const auto& fst = ti.files();
  for (auto i : fst.file_range()) {
    add_path(*_root, QString::fromStdString(fst.file_path(i)))->setSize(fst.file_size(i));
  }
  _root->updateSize();
  endResetModel();
}

bool SortDirsFirstProxyModel::lessThan(const QModelIndex& lhs, const QModelIndex& rhs) const
{
  auto lnode = reinterpret_cast<FileNode*>(lhs.internalPointer());
  auto rnode = reinterpret_cast<FileNode*>(rhs.internalPointer());
  Q_ASSERT(lnode);
  Q_ASSERT(rnode);

  if (lnode->nodes().empty() != rnode->nodes().empty()) {
    if (lnode->nodes().empty())
      return sortOrder() != Qt::AscendingOrder;
    if (rnode->nodes().empty())
      return sortOrder() == Qt::AscendingOrder;
  }

  return QSortFilterProxyModel::lessThan(lhs, rhs);
}
