// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrentinfoview.hpp"
#include "ui_torrentinfoview.h"

#include <QDateTime>

#include "torrentfilesmodel.hpp"
#include "torrenttrackersmodel.hpp"
#include "utils.hpp"

namespace {

QString format_created(const lt::torrent_info& ti)
{
  QString res;
  if (ti.creation_date() != 0) {
    res += QDateTime::fromSecsSinceEpoch(ti.creation_date()).toString();
  }
  if (ti.creation_date() != 0 && !ti.creator().empty()) {
    res += ", ";
  }
  if (!ti.creator().empty()) {
    res += QString::fromStdString(ti.creator());
  }
  return res;
}

QString format_hashes(const lt::torrent_info& ti)
{
  QString res;
  const auto& h = ti.info_hashes();
  if (h.has_v1()) {
    res += QByteArray(h.v1.data(), h.v1.size()).toHex();
  }
  if (h.has_v1() && h.has_v2()) {
    res += "\n";
  }
  if (h.has_v2()) {
    res += QByteArray(h.v2.data(), h.v2.size()).toHex();
  }
  return res;
}

QString format_pieces(const lt::torrent_info& ti)
{
  return QString("%1 x %2 = %3")
         .arg(ti.num_pieces())
         .arg(human_size(ti.piece_length()))
         .arg(human_size(ti.total_size()));
}

QString format_files(const lt::torrent_info& ti)
{
  return QString::number(ti.num_files());
}

} // namespace

TorrentInfoView::TorrentInfoView(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::TorrentInfoView)
{
  ui->setupUi(this);

  files = new TorrentFilesModel(this);
  auto sort_model = new SortDirsFirstProxyModel(this);
  sort_model->setSourceModel(files);
  sort_model->setSortRole(Qt::EditRole);
  sort_model->setSortCaseSensitivity(Qt::CaseInsensitive);
  ui->files_view->setModel(sort_model);
  ui->files_view->header()->setStretchLastSection(false);
  ui->files_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->files_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);

  trackers = new TorrentTrackersModel(this);
  ui->trackers_list->setModel(trackers);
  ui->trackers_list->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->trackers_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  ui->trackers_list->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

TorrentInfoView::~TorrentInfoView()
{
  delete ui;
}

void TorrentInfoView::setTorrentInfo(const libtorrent::torrent_info& ti)
{
  ui->label_name_v->setText(QString::fromStdString(ti.name()));

  const bool has_comment = !ti.comment().empty();
  ui->label_comment_s->setVisible(has_comment);
  ui->label_comment_v->setVisible(has_comment);
  ui->label_comment_v->setText(QString::fromStdString(ti.comment()));

  const bool has_created = ti.creation_date() !=0 || !ti.creator().empty();
  ui->label_created_s->setVisible(has_created);
  ui->label_created_v->setVisible(has_created);
  ui->label_created_v->setText(format_created(ti));

  ui->label_info_hash_v->setText(format_hashes(ti));

  ui->label_pieces_v->setText(format_pieces(ti));
  ui->label_files_v->setText(format_files(ti));

  trackers->setTorrentInfo(ti);

  files->setTorrentInfo(ti);
  if (ui->files_view->model()->rowCount(ui->files_view->rootIndex()) == 1) {
    ui->files_view->expand(ui->files_view->model()->index(0, 0, ui->files_view->rootIndex()));
  }
  ui->files_view->sortByColumn(0, Qt::AscendingOrder);
}
