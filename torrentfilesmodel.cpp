// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrentfilesmodel.hpp"

#include <QIcon>
#include <QMimeDatabase>

#include "utils.hpp"

namespace {

class FileIconProvider
{
public:
  static FileIconProvider& instance();

  QIcon icon(const QString& filename) const;

  QIcon folder() const { return _folder_icon; }
  QIcon unknown() const { return _unknown_icon; }

  QString mimetype(const QString& filename) const;

protected:
  FileIconProvider();

private:
  QIcon _folder_icon;
  QIcon _unknown_icon;

  QMimeDatabase _mdb;
  mutable QHash<QString, QIcon> _icons_cache;
};

QIcon pick_fallback_icon(const QString& name)
{
  if (name == "application/vnd.efi.iso")
    return QIcon::fromTheme("application-x-iso");
  if (name.startsWith("application"))
    return QIcon::fromTheme("application-octet-stream");
  if (name.startsWith("audio"))
    return QIcon::fromTheme("audio-x-generic");
  if (name.startsWith("image"))
    return QIcon::fromTheme("image-x-generic");
  if (name.startsWith("package"))
    return QIcon::fromTheme("package-x-generic");
  if (name.startsWith("text"))
    return QIcon::fromTheme("text-x-generic");
  if (name.startsWith("video"))
    return QIcon::fromTheme("video-x-generic");
  return FileIconProvider::instance().unknown();
}

FileIconProvider& FileIconProvider::instance()
{
  static FileIconProvider inst;
  return inst;
}

QIcon FileIconProvider::icon(const QString& filename) const
{
  auto ext = _mdb.suffixForFileName(filename);
  if (ext.isEmpty()) {
    return _unknown_icon;
  }

  if (auto iter = _icons_cache.find(ext); iter != _icons_cache.end()) {
    return *iter;
  }

  auto types = _mdb.mimeTypesForFileName(filename);
  QIcon ires;
  if (types.empty()) {
    ires = _unknown_icon;
  } else {
    const auto& mtype = types.first();
    ires = QIcon::fromTheme(mtype.iconName(), pick_fallback_icon(mtype.name()));
  }
  _icons_cache[ext] = ires;
  return ires;
}

QString FileIconProvider::mimetype(const QString& filename) const
{
  auto types = _mdb.mimeTypesForFileName(filename);
  return types.isEmpty() ? QString("unknown") : types.first().name();
}

FileIconProvider::FileIconProvider()
  : _folder_icon(QIcon::fromTheme("folder"))
  , _unknown_icon(QIcon::fromTheme("unknown"))
{}

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
    case 2:
      return tr("Mime Type");
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
  return 3;
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
          return node->nodes().empty() ?
                 FileIconProvider::instance().icon(node->name()) :
                 FileIconProvider::instance().folder();
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

    case 2: {
      switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
          return node->nodes().empty() ?
                 FileIconProvider::instance().mimetype(node->name()) :
                 QString("-");
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
