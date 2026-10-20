/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20: route asynchronous config
// reads and confirmed writes through the injected Host settings controller.
#include "configwidget.h"
#include "dbusprovider.h"
#include "launchexternal.h"
#include "logging.h"
#include "optionwidget.h"
#include "verticalscrollarea.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <fcitx-utils/i18n.h>
#include <fcitxqtdbustypes.h>
#include <utility>

namespace fcitx::kcm {

namespace {
QString joinPath(const QString &path, const QString &option) {
    if (path.isEmpty()) {
        return option;
    }
    return QStringLiteral("%1/%2").arg(path, option);
}
} // namespace

ConfigWidget::ConfigWidget(QString uri, DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), uri_(std::move(uri)), dbus_(dbus),
      mainWidget_(new QWidget(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    auto *layout = new QVBoxLayout;
    layout->addWidget(mainWidget_);
    setLayout(layout);
}

ConfigWidget::ConfigWidget(const QMap<QString, FcitxQtConfigOptionList> &desc,
                           QString mainType, DBusProvider *dbus,
                           QWidget *parent)
    : QWidget(parent), desc_(desc), mainType_(std::move(mainType)), dbus_(dbus),
      mainWidget_(new QWidget(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    auto *layout = new QVBoxLayout;
    layout->addWidget(mainWidget_);
    setLayout(layout);

    setupWidget(mainWidget_, mainType_, QString());
    initialized_ = true;
}

void ConfigWidget::requestConfig(bool sync) {
    if (!dbus_->controller() || uri_.isEmpty()) {
        Q_EMIT configurationLoaded(false, _("FusionHost is unavailable."));
        return;
    }
    // This is a local asynchronous reply over Fusion's user/session pipe. No pipe
    // I/O is performed on the GUI thread.
    auto *call = dbus_->controller()->GetConfig(uri_);
    connect(call, &PendingCall::finished, this,
            [this, call] { requestConfigFinished(call); });
    if (sync) {
        call->waitForFinished();
    }
}

void ConfigWidget::requestConfigFinished(PendingCall *call) {
    const QDBusMessage message = call->reply();
    call->deleteLater();
    // Read the reply's arguments directly: QtDBus has no public way to assign a
    // wire signature to a locally synthesized QDBusPendingCall.
    if (message.type() == QDBusMessage::ErrorMessage) {
        const QString error = message.errorMessage();
        qCWarning(KCM_FCITX5) << error;
        Q_EMIT configurationLoaded(false, error);
        return;
    }
    const QVariantList arguments = message.arguments();
    if (arguments.size() < 2) {
        const QString error = QStringLiteral("GetConfig reply carried %1 argument(s)")
                                  .arg(arguments.size());
        qCWarning(KCM_FCITX5) << error;
        Q_EMIT configurationLoaded(false, error);
        return;
    }
    const QVariant replyValue = arguments.at(0).value<QDBusVariant>().variant();
    const auto replyTypes = arguments.at(1).value<FcitxQtConfigTypeList>();

    if (!initialized_) {
        if (replyTypes.isEmpty()) {
            Q_EMIT configurationLoaded(false, _("Fcitx returned an empty configuration schema."));
            return;
        }
        for (const auto &type : replyTypes) {
            desc_[type.name()] = type.options();
        }
        mainType_ = replyTypes.front().name();
        setupWidget(mainWidget_, mainType_, QString());
        initialized_ = true;
    }

    setValue(replyValue);
    adjustSize();
    Q_EMIT configurationLoaded(true, QString());
}

QString ConfigWidget::extractOnlyExternalCommand() const {
    if (!desc_.contains(mainType_)) {
        return {};
    }
    auto options = desc_.value(mainType_);
    if (options.size() != 1) {
        return {};
    }

    if (options[0].type() != "External") {
        return {};
    }

    const auto &properties = options[0].properties();
    if (properties.contains("LaunchSubConfig") &&
        properties.value("LaunchSubConfig").toString() == "True") {
        return {};
    }

    return options[0].properties().value("External").toString();
}

void ConfigWidget::load() {
    if (uri_.isEmpty()) {
        return;
    }
    requestConfig();
}

void ConfigWidget::save() {
    if (!dbus_->controller() || uri_.isEmpty()) {
        Q_EMIT saveFinished(false, _("FusionHost is unavailable."));
        return;
    }
    auto *call = dbus_->controller()->SetConfig(uri_, QDBusVariant(value()));
    connect(call, &PendingCall::finished, this, [this, call] {
        const QDBusMessage message = call->reply();
        call->deleteLater();
        if (message.type() == QDBusMessage::ErrorMessage) {
            Q_EMIT saveFinished(false, message.errorMessage());
            return;
        }
        const QVariantList arguments = message.arguments();
        if (arguments.size() < 2) {
            Q_EMIT saveFinished(false, _("FusionHost did not return the applied settings."));
            return;
        }
        setValue(arguments.at(0).value<QDBusVariant>().variant());
        Q_EMIT saveFinished(true, QString());
    });
}

void ConfigWidget::setValue(const QVariant &value) {
    if (!initialized_) {
        return;
    }

    dontEmitChanged_ = true;
    auto optionWidgets = findChildren<OptionWidget *>();
    QVariantMap map;
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        argument >> map;
    } else {
        map = value.toMap();
    }
    for (auto optionWidget : optionWidgets) {
        if (optionWidget->skipConfig()) {
            continue;
        }
        optionWidget->readValueFrom(map);
    }
    dontEmitChanged_ = false;
}

QVariant ConfigWidget::value() const {
    QVariantMap map;
    auto optionWidgets = findChildren<OptionWidget *>();
    for (auto optionWidget : optionWidgets) {
        if (optionWidget->skipConfig()) {
            continue;
        }
        optionWidget->writeValueTo(map);
    }
    return map;
}

void ConfigWidget::buttonClicked(QDialogButtonBox::StandardButton button) {
    if (button == QDialogButtonBox::RestoreDefaults) {
        auto optionWidgets = findChildren<OptionWidget *>();
        for (auto optionWidget : optionWidgets) {
            if (optionWidget->skipConfig()) {
                continue;
            }
            optionWidget->restoreToDefault();
        }
    } else if (button == QDialogButtonBox::Ok) {
        save();
    }
}

void ConfigWidget::setupWidget(QWidget *widget, const QString &type,
                               const QString &path) {
    if (!desc_.contains(type)) {
        qCCritical(KCM_FCITX5) << type << " type does not exists.";
    }

    auto *layout = new QFormLayout(widget);
    const auto &options = desc_[type];
    for (auto &option : options) {
        addOptionWidget(layout, option, joinPath(path, option.name()));
    }

    widget->setLayout(layout);
}

void ConfigWidget::addOptionWidget(QFormLayout *layout,
                                   const FcitxQtConfigOption &option,
                                   const QString &path) {
    if (auto *optionWidget =
            OptionWidget::addWidget(layout, option, path, this)) {
        connect(optionWidget, &OptionWidget::valueChanged, this,
                &ConfigWidget::doChanged);
    } else if (desc_.contains(option.type())) {
        QGroupBox *box = new QGroupBox;
        box->setTitle(option.description());
        QVBoxLayout *innerLayout = new QVBoxLayout;
        QWidget *widget = new QWidget;
        setupWidget(widget, option.type(), path);
        innerLayout->addWidget(widget);
        box->setLayout(innerLayout);
        layout->addRow(box);
    } else {
        qCDebug(KCM_FCITX5) << "Unknown type: " << option.type();
    }
}

QDialog *ConfigWidget::configDialog(QWidget *parent, DBusProvider *dbus,
                                    const QString &uri, const QString &title) {
    auto configPage = new ConfigWidget(uri, dbus);
    configPage->requestConfig(true);
    if (auto command = configPage->extractOnlyExternalCommand();
        !command.isEmpty()) {
        WId wid = 0;
        if (QGuiApplication::platformName() == "xcb") {
            wid = parent->winId();
        }
        launchExternalConfig(command, wid);
        delete configPage;
        return nullptr;
    }
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::RestoreDefaults);

    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(_("&Cancel"));
    buttonBox->button(QDialogButtonBox::RestoreDefaults)
        ->setText(_("Restore &Defaults"));

    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage);
    dialogLayout->addWidget(configPageWrapper);
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::clicked, configPage,
            [configPage, buttonBox](QAbstractButton *button) {
                configPage->buttonClicked(buttonBox->standardButton(button));
            });

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    dialog->setWindowTitle(title);
    dialog->setLayout(dialogLayout);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    return dialog;
}

void ConfigWidget::doChanged() {
    if (dontEmitChanged_) {
        return;
    }
    Q_EMIT changed();
}

ConfigWidget *getConfigWidget(QWidget *widget) {
    widget = widget->parentWidget();
    ConfigWidget *configWidget;
    while (widget) {
        configWidget = qobject_cast<ConfigWidget *>(widget);
        if (configWidget) {
            break;
        }
        widget = widget->parentWidget();
    }
    return configWidget;
}

} // namespace fcitx::kcm
