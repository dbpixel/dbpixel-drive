/*
 * Conteúdo da etapa Boas-vindas do assistente do Drive (protótipo W1): três cartões com ícone e o servidor.
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
    id: welcome
    // ServerUrlSetupWizardPage: serverHost
    readonly property var page: ocContext
    color: "white"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 12

        Repeater {
            model: [
                { glyph: "sync", title: "Sempre igual", text: "Mudou aqui, muda na nuvem e nos outros computadores" },
                { glyph: "lock", title: "Seguro", text: "Conexão criptografada até o servidor da DBPixel" },
                { glyph: "whatsapp", title: "Suporte de verdade", text: "Equipe técnica no WhatsApp (13) 4042-3797" }
            ]
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                radius: 12
                color: DriveIcons.color.soft
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 14
                    Rectangle {
                        width: 40
                        height: 40
                        radius: 11
                        color: "white"
                        DriveGlyph {
                            anchors.centerIn: parent
                            name: modelData.glyph
                            size: 20
                            color: DriveIcons.color.navy
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label { text: modelData.title; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 15 }
                        Label { Layout.fillWidth: true; text: modelData.text; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 13; elide: Text.ElideRight }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: 10
            color: "white"
            border.color: "#D9DCE2"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                spacing: 10
                DriveGlyph { name: "cloud"; size: 18; color: DriveIcons.color.muted }
                Label { text: "Servidor"; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 13 }
                Item { Layout.fillWidth: true }
                Label { text: welcome.page ? welcome.page.serverHost : ""; color: DriveIcons.color.ink; font.family: "Montserrat"; font.weight: Font.DemiBold; font.pixelSize: 13 }
            }
        }
    }
}
