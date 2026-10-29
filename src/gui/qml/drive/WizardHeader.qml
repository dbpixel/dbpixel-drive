/*
 * Faixa de cima do assistente do Drive (protótipo W1 a W5): marca à esquerda, etapas numeradas à
 * direita (concluídas com check), canto chanfrado na cor do produto.
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
    id: header
    // SetupWizardWidget: stepIndex (0, 1, 2) e stepNames
    readonly property var wizard: ocContext
    color: DriveIcons.color.navy
    clip: true

    // canto chanfrado na cor base do produto (COR-16)
    Rectangle {
        width: 56
        height: 56
        rotation: 45
        x: header.width - 28
        y: -28
        color: DriveIcons.color.steel
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 28
        anchors.rightMargin: 40
        spacing: 10

        DriveMark { size: 30 }
        Item { Layout.fillWidth: true }

        Repeater {
            model: header.wizard ? header.wizard.stepNames : []
            RowLayout {
                spacing: 10
                readonly property bool done: index < header.wizard.stepIndex
                readonly property bool current: index === header.wizard.stepIndex
                Rectangle {
                    visible: index > 0
                    width: 18
                    height: 1
                    color: Qt.rgba(1, 1, 1, 0.25)
                }
                Rectangle {
                    width: 22
                    height: 22
                    radius: 11
                    color: done ? DriveIcons.color.okBright : current ? DriveIcons.color.action : "transparent"
                    border.color: done || current ? "transparent" : Qt.rgba(1, 1, 1, 0.3)
                    Label {
                        visible: !done
                        anchors.centerIn: parent
                        text: index + 1
                        color: current ? "white" : "#8D99AE"
                        font.family: "Montserrat"
                        font.weight: Font.Bold
                        font.pixelSize: 11
                    }
                    DriveGlyph {
                        visible: done
                        anchors.centerIn: parent
                        name: "check"
                        size: 12
                        stroke: 3
                        color: DriveIcons.color.navy
                    }
                }
                Label {
                    text: modelData
                    color: current ? "white" : done ? "#C9D2E0" : "#8D99AE"
                    font.family: "Montserrat"
                    font.weight: Font.DemiBold
                    font.pixelSize: 13
                }
            }
        }
    }
}
