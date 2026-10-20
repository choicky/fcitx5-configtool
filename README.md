fcitx5-configtool
==============================================

![Jenkins Build](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.fcitx-im.org%2Fjob%2Ffcitx5-configtool%2F)

fcitx5-configtool is a Qt-based GUI tool that provides a user-friendly configuration interface for Fcitx 5. It has two different implementations: one based on QWidget, which can be accessed via fcitx5-config-qt, and the other based on KCM/Kirigami2, which can be found in Plasma systemsettings.

## Installation

To install fcitx5-configtool, follow these steps:

Install Fcitx 5.
Install the fcitx5-configtool package.

## Usage

To use fcitx5-configtool, follow these steps:

Start fcitx5.
Run fcitx5-configtool.
Select and add your preferred input method from the “Available Input Methods” panel.

## Windows settings subset (Fcitx5 Fusion)

The `windows/fusion-settings` branch is a maintained Windows adaptation of the upstream QWidget configuration renderer for Fcitx5 Fusion. It retains the upstream schema-driven `ConfigWidget` and option widgets, replaces two KF6 controls with Qt Widgets, and defines a subset configlib/configwidgetslib build excluding the addon-list, Linux keyboard-layout, KCM and migrator surfaces.

The upstream configtool D-Bus source is intentionally not compiled by this Windows subset. The modified `DBusProvider`/`ControllerProxy` declarations are the asynchronous interface consumed by `ConfigWidget`; the Fusion product implements that interface in its own `fusion-windows/src/settings/dbusprovider.cpp` against FusionHost's user/session-scoped settings API. That boundary keeps the product's wire format and Host policy outside this upstream-derived fork. External/subconfiguration actions are disabled by the Fusion product until implemented.

This branch is based on `9bd6022c6f5f768d32b6dbe2012358e4cdb1116b` (5.1.16). Fusion's product build pins the exact reviewed commit and compiles these libraries with the product-owned adapter. Upstream copyright/license notices are preserved. The owner authorized GPL-2.0-or-later for the upstream-derived settings executable and derivative code only; this does not change licensing for unrelated Fusion components.
