// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "utils.hpp"

#include <array>

QString human_size(qint64 size, int unit_threshold)
{
  // *INDENT-OFF*
  constexpr std::array<char, 7> units = {'B', 'K', 'M', 'G', 'T', 'P', 'E'};
  constexpr std::array<char, 7> uprec = { 0,   1 ,  1 ,  2 ,  2 ,  3 ,  3 };
  // *INDENT-ON*
  size_t i = 0;
  auto value = static_cast<qreal>(size);

  while ((qAbs(value) >= unit_threshold) && (i < units.size() - 1)) {
    value /= 1024;
    ++i;
  }
  return QString("%1 %2").arg(value, 0, 'f', uprec[i]).arg(units[i]);
}
