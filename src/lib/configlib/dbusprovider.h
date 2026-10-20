/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
// Modified by Fcitx5 Fusion contributors, 2026-10-20: expose an asynchronous
// settings transport interface implemented by the FusionHost product adapter.
#ifndef _DBUSPROVIDER_H_
#define _DBUSPROVIDER_H_

#include <QDBusMessage>
#include <QDBusVariant>
#include <QObject>
#include <QString>
#include <QStringList>
#include <fcitxqtdbustypes.h>

class QLocalSocket;
class QTimer;

namespace fcitx::kcm {

struct SettingsSelection {
    QString selected;
    QStringList available;
};

class PendingCall : public QObject {
    Q_OBJECT

public:
    explicit PendingCall(QObject *parent = nullptr) : QObject(parent) {}

    bool isFinished() const { return finished_; }
    void waitForFinished(); // retained only for upstream's synchronous helper path
    const QDBusMessage &reply() const { return reply_; }
    SettingsSelection selection() const { return selection_; }

    void complete(const QDBusMessage &reply, SettingsSelection selection = {});
    void fail(const QString &error);

Q_SIGNALS:
    void finished();

private:
    bool finished_ = false;
    QDBusMessage reply_;
    SettingsSelection selection_;
};

class DBusProvider;
class PipeRequest;

class ControllerProxy : public QObject {
    Q_OBJECT

public:
    enum class Operation { GetConfig, SetConfig, GetSelection, SelectInputMethod };

    ControllerProxy(const QString &pipeName, DBusProvider *provider, QObject *parent = nullptr);

    PendingCall *GetConfig(const QString &uri);
    PendingCall *SetConfig(const QString &uri, const QDBusVariant &value);
    PendingCall *GetSelection();
    PendingCall *SelectInputMethod(const QString &method);

private:
    friend class PipeRequest;
    void startRequest(Operation operation, const QString &uri, const QVariant &value,
                      PendingCall *call);
    QString pipeName_;
    DBusProvider *provider_;
};

class DBusProvider : public QObject {
    Q_OBJECT

public:
    DBusProvider(QObject *parent, const QString &pipeName);

    bool available() const { return available_; }
    bool canRestart() const { return false; }
    ControllerProxy *controller() const { return controller_; }

Q_SIGNALS:
    void availabilityChanged(bool avail);
    void canRestartChanged(bool canRestart);

private:
    friend class ControllerProxy;
    friend class PipeRequest;
    void setAvailable(bool available);

    ControllerProxy *controller_ = nullptr;
    bool available_ = false;
};

} // namespace fcitx::kcm

#endif // _DBUSPROVIDER_H_
