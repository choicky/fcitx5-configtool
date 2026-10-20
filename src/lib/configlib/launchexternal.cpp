/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20: launchExternalConfig is extracted from addonmodel.cpp so the
// "External" option type keeps its upstream behaviour without the addon-list model.
// On this subset build FCITX5_QT_GUI_WRAPPER is set to the upstream qt6 wrapper
// name; when the wrapper binary is absent (as on Windows) the addon-uri branch
// returns without launching, and plain program-path uris still startDetached.
#include "launchexternal.h"
#include "config.h"
#include <QDir>
#include <QFileInfo>
#include <QLatin1String>
#include <QProcess>
#include <QStringList>
#include <fcitx-utils/standardpaths.h>
#include <filesystem>
#include <functional>

namespace fcitx::kcm {

void launchExternalConfig(const QString &uri, WId wid) {
    if (uri.isEmpty()) {
        return;
    }
    QFileInfo pathToWrapper(FCITX5_QT_GUI_WRAPPER);
    QDir dirToWrapper = pathToWrapper.dir();
    if (uri.startsWith("fcitx://config/addon/")) {
        QLatin1String qt5Wrapper("fcitx5-qt5-gui-wrapper");
        QLatin1String qt6Wrapper("fcitx5-qt6-gui-wrapper");

        QString qt5WrapperPath = dirToWrapper.filePath(qt5Wrapper);
        QString qt6WrapperPath = dirToWrapper.filePath(qt6Wrapper);

        for (QString &wrapperPath :
             {std::ref(qt6WrapperPath), std::ref(qt5WrapperPath)}) {
            if (!QFileInfo(wrapperPath).isExecutable()) {
                // Windows portability adaptation (same filesystem path conversion issue as fcitx5-qt
                // fromPath fix): std::filesystem::path converts to std::string
                // implicitly only where path::string_type is std::string (POSIX).
                const std::filesystem::path candidate =
                    StandardPaths::fcitxPath("libexecdir") /
                    QFileInfo(wrapperPath).fileName().toStdString();
#if defined(_WIN32)
                wrapperPath = QString::fromStdWString(candidate.wstring());
#else
                wrapperPath = QString::fromStdString(candidate.string());
#endif
            }
        }

        QString wrapperToUse;
        for (const QString &wrapperPath :
             {std::cref(qt6WrapperPath), std::cref(qt5WrapperPath)}) {
            QStringList args;
            args << QLatin1String("--test");
            args << uri;
            int exit_status = QProcess::execute(wrapperPath, args);
            if (exit_status == 0) {
                wrapperToUse = wrapperPath;
                break;
            }
        }

        if (wrapperToUse.isEmpty()) {
            return;
        }

        QStringList args;
        if (wid) {
            args << "-w";
            args << QString::number(wid);
        }
        args << uri;
        QProcess::startDetached(wrapperToUse, args);
    } else {
        // Assume this is a program path.
        QStringList args = QProcess::splitCommand(uri);
        if (args.isEmpty()) {
            return;
        }
        QString program = args.takeFirst();
        QProcess::startDetached(program, args);
    }
}

} // namespace fcitx::kcm
