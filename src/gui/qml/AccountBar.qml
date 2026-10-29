/*
 * Copyright (C) by Hannah von Reth <hannah.vonreth@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

// Drive (protótipo C1 a C5): menu lateral navy das configurações, no lugar da faixa de contas do topo.
// Mesmas ações de antes (conta, atividade/problemas, configurações, sair), mais Rede, Sobre e ajuda.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.ownCloud.gui 1.0
import org.ownCloud.libsync 1.0
import org.ownCloud.resources 1.0
import "drive/DriveIcons.js" as DriveIcons

Rectangle {
    id: bar
    readonly property SettingsDialog settingsDialog: ocContext
    readonly property OCQuickWidget widget: ocQuickWidget

    Accessible.name: "Menu"
    color: DriveIcons.color.navy
    clip: true

    component NavItem: AbstractButton {
        id: nav
        property string glyph
        property int badge: 0
        Layout.fillWidth: true
        implicitHeight: 42
        hoverEnabled: true
        Accessible.role: Accessible.PageTab
        background: Rectangle {
            radius: 10
            color: nav.checked ? Qt.rgba(1, 1, 1, 0.1) : (nav.hovered ? Qt.rgba(1, 1, 1, 0.05) : "transparent")
            border.color: nav.visualFocus ? DriveIcons.color.steelOnDark : "transparent"
        }
        contentItem: RowLayout {
            spacing: 12
            Item { width: 2 }
            DriveGlyph { name: nav.glyph; size: 18; color: nav.checked ? DriveIcons.color.steelOnDark : "#AEB4BF" }
            Label {
                Layout.fillWidth: true
                text: nav.text
                elide: Text.ElideRight
                color: nav.checked ? "white" : "#AEB4BF"
                font.family: "Nunito Sans"
                font.weight: nav.checked ? Font.Bold : Font.Normal
                font.pixelSize: 14
            }
            Rectangle {
                visible: nav.badge > 0
                width: 22; height: 22; radius: 11
                color: DriveIcons.color.action
                Label { anchors.centerIn: parent; text: nav.badge; color: "white"; font.family: "Montserrat"; font.weight: Font.Bold; font.pixelSize: 11 }
            }
            Item { width: 4 }
        }
    }

    // canto chanfrado na cor do produto
    Rectangle {
        width: 60; height: 60; rotation: 45
        x: -30; y: bar.height - 30
        color: DriveIcons.color.steel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        anchors.topMargin: 20
        spacing: 2

        DriveMark { size: 28; Layout.leftMargin: 10; Layout.bottomMargin: 22 }

        Repeater {
            id: accountButtons
            model: AccountManager.accounts
            delegate: NavItem {
                property AccountState accountState: modelData
                glyph: "user"
                text: AccountManager.accounts.length > 1 ? accountState.account.hostName : "Conta"
                checked: settingsDialog.currentAccount === accountState.account
                onClicked: settingsDialog.currentAccount = accountState.account
            }
        }
        NavItem {
            id: addAccountButton
            glyph: "plus"
            text: "Adicionar conta"
            visible: Theme.multiAccount || AccountManager.accounts.length === 0
            onClicked: settingsDialog.addAccount()
        }
        NavItem {
            glyph: "gear"
            text: "Geral"
            checked: settingsDialog.currentPage === SettingsDialog.Settings
            onClicked: settingsDialog.currentPage = SettingsDialog.Settings
        }
        NavItem {
            glyph: "wifi"
            text: "Rede"
            checked: settingsDialog.currentPage === SettingsDialog.Network
            onClicked: settingsDialog.currentPage = SettingsDialog.Network
        }
        NavItem {
            glyph: "alert"
            text: "Problemas"
            badge: settingsDialog.issueCount
            checked: settingsDialog.currentPage === SettingsDialog.Activity
            onClicked: settingsDialog.currentPage = SettingsDialog.Activity
        }
        NavItem {
            glyph: "info"
            text: "Sobre"
            checked: settingsDialog.currentPage === SettingsDialog.About
            onClicked: settingsDialog.currentPage = SettingsDialog.About
        }

        Item { Layout.fillHeight: true }

        // ajuda pelo WhatsApp da DBPixel
        AbstractButton {
            id: help
            Layout.fillWidth: true
            Layout.bottomMargin: 8
            implicitHeight: 58
            hoverEnabled: true
            onClicked: settingsDialog.openHelp()
            background: Rectangle { radius: 12; color: help.hovered ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.06) }
            contentItem: RowLayout {
                spacing: 10
                Item { width: 2 }
                DriveGlyph { name: "whatsapp"; size: 18; color: DriveIcons.color.okBright }
                ColumnLayout {
                    spacing: 0
                    Label { text: "Precisa de ajuda?"; color: "white"; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 13 }
                    Label { text: "WhatsApp (13) 4042-3797"; color: "#9AA0AC"; font.family: "Nunito Sans"; font.pixelSize: 12 }
                }
            }
        }
        NavItem {
            id: quitButton
            glyph: "power"
            text: "Sair do Drive"
            Layout.bottomMargin: 16
            onClicked: Qt.quit()
        }
    }

    Connections {
        target: widget
        function onFocusFirst() {
            if (accountButtons.count > 0) {
                accountButtons.itemAt(0).forceActiveFocus(Qt.TabFocusReason);
            } else {
                addAccountButton.forceActiveFocus(Qt.TabFocusReason);
            }
        }
        function onFocusLast() {
            quitButton.forceActiveFocus(Qt.TabFocusReason);
        }
    }
}
