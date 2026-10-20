/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20: replace the KF6 font
// selector with the Qt Widgets font dialog for Windows builds.

#include "fontbutton.h"
#include "font.h"
#include <QFontDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <fcitx-utils/i18n.h>

namespace fcitx::kcm {

FontButton::FontButton(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    connect(fontSelectButton, &QPushButton::clicked, this,
            &FontButton::selectFont);
}

const QFont &FontButton::font() { return font_; }

QString FontButton::fontName() { return fontPreviewLabel->text(); }

void FontButton::setFont(const QFont &font) {
    font_ = font;
    if (font.family() != font_.family()) {
        Q_EMIT fontChanged(font_);
    }
    fontPreviewLabel->setText(fontToString(font_));
    fontPreviewLabel->setFont(font_);
}

void FontButton::selectFont() {
    // Windows adaptation: KFontChooser is a KF6 widget; QFontDialog is its Qt equivalent and keeps
    // the same behaviour (pick a font, apply it on accept).
    bool accepted = false;
    const QFont picked =
        QFontDialog::getFont(&accepted, font_, this, _("Select Font"));
    if (accepted) {
        setFont(picked);
    }
}

} // namespace fcitx::kcm
