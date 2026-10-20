/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20. launchExternalConfig is declared upstream in addonmodel.h, which
// drags in the whole addon-list model (model.h). The addon-list page is excluded
// from this subset, so the declaration is mirrored here and the definition is
// retained verbatim in launchexternal.cpp.
#ifndef _CONFIGLIB_LAUNCHEXTERNAL_H_
#define _CONFIGLIB_LAUNCHEXTERNAL_H_

#include <QString>
#include <QWindow>

namespace fcitx::kcm {

void launchExternalConfig(const QString &uri, WId wid);

} // namespace fcitx::kcm

#endif // _CONFIGLIB_LAUNCHEXTERNAL_H_
