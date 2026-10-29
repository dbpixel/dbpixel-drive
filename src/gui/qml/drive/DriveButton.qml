/*
 * Botão do Drive: principal no tom "botão" do produto (#47689A, texto branco, COR-14) ou secundário cinza.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "DriveIcons.js" as DriveIcons

AbstractButton {
    id: b
    property bool primary: false
    property bool small: false
    property string glyph: ""
    implicitHeight: small ? 34 : 42
    implicitWidth: row.implicitWidth + (small ? 24 : 36)
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    background: Rectangle {
        radius: 10
        color: b.primary ? (!b.enabled ? DriveIcons.color.actionDisabled : b.down ? DriveIcons.color.actionDown : b.hovered ? DriveIcons.color.actionHover : DriveIcons.color.action)
                         : (b.hovered ? "#E9EBEF" : DriveIcons.color.soft)
        border.color: b.visualFocus ? DriveIcons.color.action : (b.primary ? "transparent" : "#D9DCE2")
        border.width: b.visualFocus ? 2 : 1
    }

    contentItem: Item {
        RowLayout {
            id: row
            anchors.centerIn: parent
            spacing: 8
            DriveGlyph {
                visible: b.glyph !== ""
                name: b.glyph
                size: 16
                color: b.primary ? (b.enabled ? "white" : "#5E6B80") : DriveIcons.color.ink
            }
            Label {
                text: b.text
                color: b.primary ? (b.enabled ? "white" : "#5E6B80") : DriveIcons.color.ink
                font.family: "Montserrat"
                font.weight: Font.DemiBold
                font.pixelSize: b.small ? 13 : 14
            }
        }
    }
}
