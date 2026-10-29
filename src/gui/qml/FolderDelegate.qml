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

// Drive (protótipo C1): página da conta. Nome e e-mail, situação da conexão, espaço na nuvem e as pastas
// sincronizadas em cartão. Mesmo modelo e mesmas ações de antes (menu da pasta, adicionar pasta).

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.ownCloud.gui 1.0
import org.ownCloud.libsync 1.0
import org.ownCloud.resources 1.0
import "drive/DriveIcons.js" as DriveIcons

Rectangle {
    id: folderSyncPanel
    readonly property AccountSettings accountSettings: ocContext
    readonly property OCQuickWidget widget: ocQuickWidget
    readonly property bool connected: accountSettings.accountState.state === AccountState.Connected

    Accessible.role: Accessible.List
    Accessible.name: "Pastas sincronizadas"
    color: DriveIcons.color.page

    Connections {
        target: widget
        function onFocusFirst() {
            manageButton.forceActiveFocus(Qt.TabFocusReason);
        }
        function onFocusLast() {
            addSyncButton.forceActiveFocus(Qt.TabFocusReason);
        }
    }

    // situação da pasta a partir do ícone de estado do modelo
    function folderPill(icon, errors, busy) {
        const s = String(icon || "");
        if ((errors && errors.length > 0) || s.indexOf("error") >= 0 || s.indexOf("problem") >= 0)
            return { text: "Com problema", fg: DriveIcons.color.warn, bg: "#FFF1E6" };
        if (busy || s.indexOf("sync") >= 0)
            return { text: "Sincronizando", fg: "#1D4E89", bg: "#E8EFF8" };
        if (s.indexOf("pause") >= 0)
            return { text: "Pausada", fg: DriveIcons.color.muted, bg: "#EEEEEC" };
        if (s.indexOf("offline") >= 0)
            return { text: "Sem conexão", fg: DriveIcons.color.muted, bg: "#EEEEEC" };
        return { text: "Em dia", fg: DriveIcons.color.ok, bg: DriveIcons.color.okSoft };
    }

    component Pill: Rectangle {
        property string text
        property color fg
        implicitWidth: pillText.implicitWidth + 20
        implicitHeight: 24
        radius: 12
        Label { id: pillText; anchors.centerIn: parent; text: parent.text; color: parent.fg; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 12 }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: page.implicitHeight + 60
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        ColumnLayout {
            id: page
            x: 30
            y: 28
            width: parent.width - 60
            spacing: 16

            // cabeçalho: quem está conectado
            RowLayout {
                Layout.fillWidth: true
                spacing: 14
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        text: accountSettings.accountState.account.davDisplayName || accountSettings.accountState.account.davUser
                        color: DriveIcons.color.ink
                        font.family: "Montserrat"
                        font.weight: Font.DemiBold
                        font.pixelSize: 22
                    }
                    Label {
                        Layout.fillWidth: true
                        text: accountSettings.accountState.account.davUser + " · " + accountSettings.accountState.account.hostName
                        elide: Text.ElideRight
                        color: DriveIcons.color.muted
                        font.family: "Nunito Sans"
                        font.pixelSize: 13
                    }
                }
                Pill {
                    text: folderSyncPanel.connected ? "Conectado" : (accountSettings.accountState.state === AccountState.SignedOut ? "Desconectado" : "Conectando…")
                    fg: folderSyncPanel.connected ? DriveIcons.color.ok : DriveIcons.color.muted
                    color: folderSyncPanel.connected ? DriveIcons.color.okSoft : "#EEEEEC"
                }
                DriveButton {
                    id: manageButton
                    small: true
                    text: "Gerenciar conta"
                    onClicked: accountSettings.openManageMenu()
                }
            }

            // espaço na nuvem
            Rectangle {
                visible: accountSettings.quotaText !== ""
                Layout.fillWidth: true
                Layout.preferredHeight: quotaCol.implicitHeight + 36
                radius: 14
                color: "white"
                border.color: "#E3E6EB"
                ColumnLayout {
                    id: quotaCol
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 10
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Espaço na nuvem"; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.pixelSize: 14 }
                        Item { Layout.fillWidth: true }
                        Label { text: accountSettings.quotaText; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 14 }
                    }
                    Rectangle {
                        visible: accountSettings.quotaFraction >= 0
                        Layout.fillWidth: true
                        height: 10; radius: 5
                        color: DriveIcons.color.soft
                        Rectangle {
                            width: parent.width * Math.max(0, accountSettings.quotaFraction)
                            height: parent.height; radius: 5
                            color: accountSettings.quotaFraction > 0.9 ? DriveIcons.color.warn : DriveIcons.color.navy
                        }
                    }
                    Label { visible: text !== ""; text: accountSettings.quotaDetail; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 13 }
                }
            }

            // pastas sincronizadas
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: foldersCol.implicitHeight + 36
                radius: 14
                color: "white"
                border.color: "#E3E6EB"
                ColumnLayout {
                    id: foldersCol
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.bottomMargin: 6
                        Label { text: "Pastas sincronizadas"; color: DriveIcons.color.ink; font.family: "Montserrat"; font.weight: Font.DemiBold; font.pixelSize: 15 }
                        Item { Layout.fillWidth: true }
                        DriveButton {
                            id: addSyncButton
                            small: true
                            glyph: "plus"
                            text: "Adicionar pasta"
                            enabled: folderSyncPanel.connected && (accountSettings.unsyncedSpaces || !accountSettings.accountState.supportsSpaces)
                            visible: accountSettings.accountState.supportsSpaces ? !Theme.syncNewlyDiscoveredSpaces : (listView.count === 0 || !Theme.singleSyncFolder)
                            onClicked: accountSettings.slotAddFolder()
                        }
                    }

                    ListView {
                        id: listView
                        Layout.fillWidth: true
                        Layout.preferredHeight: contentHeight
                        interactive: false
                        model: accountSettings.model
                        spacing: 0

                        delegate: Item {
                            id: folderDelegate
                            width: ListView.view.width
                            implicitHeight: rowCol.implicitHeight + 24

                            required property string displayName
                            required property var errorMsg
                            required property Folder folder
                            required property string itemText
                            required property string overallText
                            required property double progress
                            required property string quota
                            required property string accessibleDescription
                            required property string statusIcon
                            required property string subtitle
                            required property int index

                            readonly property var pill: folderSyncPanel.folderPill(statusIcon, errorMsg, overallText !== "" || itemText !== "")

                            Accessible.name: accessibleDescription
                            Accessible.role: Accessible.ListItem

                            Rectangle { visible: folderDelegate.index > 0; width: parent.width; height: 1; color: "#F0F1F3" }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.RightButton
                                onClicked: accountSettings.slotCustomContextMenuRequested(folderDelegate.folder)
                            }

                            ColumnLayout {
                                id: rowCol
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 6
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 14
                                    DriveGlyph { name: "folder"; size: 22; color: DriveIcons.color.navy }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 1
                                        Label { Layout.fillWidth: true; text: folderDelegate.displayName; elide: Text.ElideRight; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 14 }
                                        Label { Layout.fillWidth: true; text: folderDelegate.subtitle; elide: Text.ElideMiddle; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 12 }
                                    }
                                    Pill { text: folderDelegate.pill.text; fg: folderDelegate.pill.fg; color: folderDelegate.pill.bg }
                                    DriveButton {
                                        small: true
                                        glyph: "gear"
                                        text: ""
                                        Accessible.name: "Opções da pasta " + folderDelegate.displayName
                                        onClicked: accountSettings.slotCustomContextMenuRequested(folderDelegate.folder)
                                    }
                                }
                                // andamento da sincronização
                                ColumnLayout {
                                    visible: folderDelegate.overallText !== "" || folderDelegate.itemText !== ""
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 36
                                    spacing: 4
                                    Rectangle {
                                        Layout.fillWidth: true
                                        height: 4; radius: 2; color: DriveIcons.color.line
                                        Rectangle { width: parent.width * Math.max(0, folderDelegate.progress); height: parent.height; radius: 2; color: DriveIcons.color.action }
                                    }
                                    Label { Layout.fillWidth: true; text: folderDelegate.overallText; elide: Text.ElideMiddle; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 12 }
                                    Label { Layout.fillWidth: true; visible: text !== ""; text: folderDelegate.itemText; elide: Text.ElideMiddle; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 12 }
                                }
                                FolderError {
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 36
                                    errorMessages: folderDelegate.errorMsg
                                }
                            }
                        }
                    }

                    Label {
                        visible: listView.count === 0
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        text: "Nenhuma pasta sincronizada ainda. Use Adicionar pasta."
                        color: DriveIcons.color.muted
                        font.family: "Nunito Sans"
                        font.pixelSize: 13
                    }
                }
            }
        }
    }
}
