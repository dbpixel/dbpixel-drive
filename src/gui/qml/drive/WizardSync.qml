/*
 * Conteúdo da etapa Sincronização do assistente do Drive (protótipo W3): pasta neste computador e o que
 * sincronizar, em cartões. Os valores ficam na página C++ (AccountConfiguredWizardPage), que o assistente lê.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "DriveIcons.js" as DriveIcons

Rectangle {
    id: sync
    // AccountConfiguredWizardPage: folder, freeSpaceText, syncEverything, chooseFolder()
    readonly property var page: ocContext
    color: "white"

    component OptionCard: AbstractButton {
        id: oc
        property string title
        property string detail
        Layout.fillWidth: true
        implicitHeight: 66
        hoverEnabled: true
        background: Rectangle {
            radius: 12
            color: oc.checked ? DriveIcons.color.steelSoft : (oc.hovered ? "#FAFBFC" : "white")
            border.color: oc.checked ? DriveIcons.color.action : "#D9DCE2"
            border.width: oc.checked ? 2 : 1
        }
        contentItem: RowLayout {
            spacing: 14
            Item { width: 2 }
            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: "white"
                border.color: oc.checked ? DriveIcons.color.action : "#7D8390"
                border.width: oc.checked ? 6 : 1.5
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label { text: oc.title; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 15 }
                Label { Layout.fillWidth: true; text: oc.detail; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 13; elide: Text.ElideRight }
            }
        }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 32
        spacing: 14

        Rectangle {
            Layout.preferredHeight: 30
            Layout.preferredWidth: connected.implicitWidth + 24
            radius: 15
            color: DriveIcons.color.okSoft
            RowLayout {
                id: connected
                anchors.centerIn: parent
                spacing: 6
                DriveGlyph { name: "check"; size: 14; stroke: 2.6; color: DriveIcons.color.ok }
                Label { text: "Conta conectada"; color: DriveIcons.color.ok; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 13 }
            }
        }

        Label {
            Layout.topMargin: 6
            text: "Pasta neste computador"
            color: DriveIcons.color.ink
            font.family: "Nunito Sans"
            font.weight: Font.Bold
            font.pixelSize: 13
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            radius: 10
            color: "white"
            border.color: "#7D8390"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 6
                spacing: 10
                DriveGlyph { name: "folder"; size: 18; color: DriveIcons.color.navy }
                Label {
                    Layout.fillWidth: true
                    text: sync.page ? sync.page.folder : ""
                    elide: Text.ElideMiddle
                    color: DriveIcons.color.ink
                    font.family: "Nunito Sans"
                    font.pixelSize: 14
                }
                DriveButton {
                    small: true
                    text: "Alterar"
                    enabled: sync.page ? sync.page.syncEverything : true
                    onClicked: sync.page.chooseFolder()
                }
            }
        }
        Label {
            visible: text !== ""
            text: sync.page ? sync.page.freeSpaceText : ""
            color: DriveIcons.color.muted
            font.family: "Nunito Sans"
            font.pixelSize: 12
        }

        ButtonGroup { id: mode }
        OptionCard {
            Layout.topMargin: 6
            ButtonGroup.group: mode
            checkable: true
            checked: sync.page ? sync.page.syncEverything : true
            title: "Sincronizar tudo"
            detail: "Todas as pastas da nuvem vêm para este computador · recomendado"
            onClicked: sync.page.syncEverything = true
        }
        OptionCard {
            ButtonGroup.group: mode
            checkable: true
            checked: sync.page ? !sync.page.syncEverything : false
            title: "Escolher as pastas"
            detail: "Só o que você marcar vem para este PC; o resto fica na nuvem"
            onClicked: sync.page.syncEverything = false
        }
    }
}
