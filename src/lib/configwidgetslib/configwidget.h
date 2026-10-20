/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20: expose load/apply result
// signals so the host application can report confirmed success or failure.
#ifndef _KCM_FCITX_CONFIGWIDGET_H_
#define _KCM_FCITX_CONFIGWIDGET_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QMap>
#include <QString>
#include <QWidget>
#include <fcitxqtdbustypes.h>

class QFormLayout;

namespace fcitx::kcm {

class DBusProvider;
class PendingCall;

class ConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConfigWidget(QString uri, DBusProvider *dbus, QWidget *parent = 0);

    explicit ConfigWidget(const QMap<QString, FcitxQtConfigOptionList> &desc,
                          QString mainType, DBusProvider *dbus,
                          QWidget *parent = 0);

    static QDialog *configDialog(QWidget *parent, DBusProvider *module,
                                 const QString &uri,
                                 const QString &title = QString());

    auto dbus() { return dbus_; }
    auto &description() const { return desc_; }

Q_SIGNALS:
    void changed();
    void configurationLoaded(bool success, const QString &message);
    void saveFinished(bool success, const QString &message);

public Q_SLOTS:
    void load();
    void save();
    void buttonClicked(QDialogButtonBox::StandardButton);

    QVariant value() const;
    void setValue(const QVariant &variant);

    QString extractOnlyExternalCommand() const;

    void requestConfig(bool sync = false);
private Q_SLOTS:
    void requestConfigFinished(PendingCall *call);
    void doChanged();

private:
    void setupWidget(QWidget *widget, const QString &type, const QString &path);
    void addOptionWidget(QFormLayout *layout, const FcitxQtConfigOption &option,
                         const QString &path);

    bool initialized_ = false;
    QString uri_;
    QMap<QString, FcitxQtConfigOptionList> desc_;
    QString mainType_;
    DBusProvider *dbus_;
    QWidget *mainWidget_;

    bool dontEmitChanged_ = false;
};

ConfigWidget *getConfigWidget(QWidget *widget);

} // namespace fcitx::kcm

#endif // _KCM_FCITX_CONFIGWIDGET_H_
