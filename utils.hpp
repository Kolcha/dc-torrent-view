// SPDX-FileCopyrightText: 2024 Nick Korotysh <nick.korotysh@gmail.com>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QString>

QString human_size(qint64 size, int unit_threshold = 1024);
