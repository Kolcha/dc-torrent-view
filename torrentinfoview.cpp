// SPDX-FileCopyrightText: 2024-2026 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "torrentinfoview.hpp"
#include "ui_torrentinfoview.h"

#include <QDateTime>
#include <QRegularExpression>

#include "torrentfilesmodel.hpp"
#include "torrenttrackersmodel.hpp"
#include "utils.hpp"

namespace {

QString format_comment(const lt::add_torrent_params& atp)
{
  static QRegularExpression re(R"(https?:\/\/(?:www\.)?\S+\.[a-zA-Z0-9]{1,6}\b\S*)");

  auto out = QString::fromStdString(atp.comment);

  auto iter = re.globalMatch(out);
  while (iter.hasNext()) {
    auto m = iter.next();
    auto url = m.captured();
    out.replace(url, QString(R"(<a href="%1">%1</a>)").arg(url));
  }

  return out;
}

QString format_created(const lt::add_torrent_params& atp)
{
  QString res;
  if (atp.creation_date != 0) {
    res += QDateTime::fromSecsSinceEpoch(atp.creation_date).toString();
  }
  if (atp.creation_date != 0 && !atp.created_by.empty()) {
    res += ", ";
  }
  if (!atp.created_by.empty()) {
    res += QString::fromStdString(atp.created_by);
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
  ui->files_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->files_view->header()->setSectionResizeMode(0, QHeaderView::Stretch);

  trackers = new TorrentTrackersModel(this);
  ui->trackers_list->setModel(trackers);
  ui->trackers_list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  ui->splitter->setSizes({ui->trackers_list->minimumSizeHint().height(), height()});
}

TorrentInfoView::~TorrentInfoView()
{
  delete ui;
}

void TorrentInfoView::setTorrentInfo(const lt::add_torrent_params& atp)
{
  ui->label_name_v->setText(QString::fromStdString(atp.ti->name()));
  ui->label_type_v->setText(atp.ti->priv() ? tr("private") : tr("public"));

  const bool has_comment = !atp.comment.empty();
  ui->label_comment_s->setVisible(has_comment);
  ui->label_comment_v->setVisible(has_comment);
  ui->label_comment_v->setText(format_comment(atp));

  const bool has_created = atp.creation_date !=0 || !atp.created_by.empty();
  ui->label_created_s->setVisible(has_created);
  ui->label_created_v->setVisible(has_created);
  ui->label_created_v->setText(format_created(atp));

  ui->label_info_hash_v->setText(format_hashes(*atp.ti));

  ui->label_pieces_v->setText(format_pieces(*atp.ti));
  ui->label_files_v->setText(format_files(*atp.ti));

  trackers->setTorrentInfo(atp);
  ui->trackers_list->setVisible(trackers->rowCount() > 0);

  files->setTorrentInfo(*atp.ti);
  if (ui->files_view->model()->rowCount(ui->files_view->rootIndex()) == 1) {
    ui->files_view->expand(ui->files_view->model()->index(0, 0, ui->files_view->rootIndex()));
  }
  ui->files_view->sortByColumn(0, Qt::AscendingOrder);
}
