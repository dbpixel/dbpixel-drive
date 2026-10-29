/*
 * Drive (protótipo C5): página "Sobre" dentro das configurações. Marca, versão, suporte, atualização
 * automática, relatório de problema e a menção à licença.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import org.ownCloud.gui 1.0
import "DriveIcons.js" as DriveIcons

Rectangle {
    id: page
    readonly property DriveSupport support: ocContext
    color: DriveIcons.color.page

    // relato enviado: limpa o campo para não mandar o mesmo duas vezes
    Connections {
        target: page.support
        function onReportChanged() {
            if (page.support.reportState === DriveSupport.ReportSent)
                description.text = "";
        }
    }

    component Card: Rectangle {
        default property alias content: inner.data
        Layout.fillWidth: true
        Layout.preferredHeight: inner.implicitHeight + 44
        radius: 14
        color: "white"
        border.color: "#E3E6EB"
        ColumnLayout {
            id: inner
            anchors.fill: parent
            anchors.margins: 22
            spacing: 10
        }
    }
    component CardTitle: Label {
        color: DriveIcons.color.ink
        font.family: "Montserrat"
        font.weight: Font.DemiBold
        font.pixelSize: 16
    }
    component Body: Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: DriveIcons.color.muted
        font.family: "Nunito Sans"
        font.pixelSize: 14
        lineHeight: 1.15
    }

    Flickable {
        id: flick
        anchors.fill: parent
        contentHeight: column.implicitHeight + 56
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        ColumnLayout {
            id: column
            x: 30
            y: 28
            width: flick.width - 60
            spacing: 16

            Label {
                text: "Sobre"
                color: DriveIcons.color.ink
                font.family: "Montserrat"
                font.weight: Font.DemiBold
                font.pixelSize: 24
            }

            // marca e versão
            Card {
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20
                    DriveMark { size: 64; showName: false; Layout.alignment: Qt.AlignTop }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        Label { text: "Drive"; color: DriveIcons.color.ink; font.family: "Montserrat"; font.weight: Font.ExtraBold; font.pixelSize: 26 }
                        Label {
                            text: "um produto <b>DBPixel</b>"
                            textFormat: Text.StyledText
                            color: DriveIcons.color.steelText
                            font.family: "Nunito Sans"
                            font.pixelSize: 13
                        }
                        Label {
                            Layout.topMargin: 6
                            text: "Versão " + page.support.version
                            color: DriveIcons.color.ink
                            font.family: "Nunito Sans"
                            font.weight: Font.Bold
                            font.pixelSize: 14
                        }
                        Body { text: "Aplicativo de sincronização do Armazenamento em Nuvem da DBPixel." }
                    }
                }
                Flow {
                    Layout.fillWidth: true
                    Layout.topMargin: 6
                    spacing: 10
                    DriveButton { small: true; glyph: "whatsapp"; text: "Suporte no WhatsApp"; onClicked: page.support.openWhatsApp("") }
                    DriveButton { small: true; glyph: "globe"; text: "dbpixel.com.br"; onClicked: page.support.openSite() }
                    DriveButton {
                        id: copyButton
                        small: true
                        glyph: copied ? "check" : "copy"
                        property bool copied: false
                        text: copied ? "Copiado" : "Copiar dados para o suporte"
                        onClicked: { page.support.copySupportInfo(); copied = true; copiedTimer.restart(); }
                        Timer { id: copiedTimer; interval: 2500; onTriggered: copyButton.copied = false }
                    }
                }
            }

            // atualização automática
            Card {
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 14
                    Rectangle {
                        readonly property bool good: page.support.updateState === DriveSupport.UpToDate || page.support.updateState === DriveSupport.Idle
                        readonly property bool bad: page.support.updateState === DriveSupport.Failed
                        width: 40; height: 40; radius: 12
                        color: bad ? "#FFF1E6" : (good ? DriveIcons.color.okSoft : DriveIcons.color.steelSoft)
                        DriveGlyph {
                            anchors.centerIn: parent
                            size: 20
                            name: parent.bad ? "alert" : (parent.good ? "check" : "sync")
                            color: parent.bad ? DriveIcons.color.warn : (parent.good ? DriveIcons.color.ok : DriveIcons.color.action)
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        CardTitle { text: "Atualizações" }
                        Body {
                            text: page.support.updateText !== "" ? page.support.updateText
                                : "O Drive se atualiza sozinho. Quando sai uma versão nova, ele baixa e instala sem você precisar fazer nada."
                            color: page.support.updateState === DriveSupport.Failed ? DriveIcons.color.warn : DriveIcons.color.muted
                        }
                    }
                    DriveButton {
                        small: true
                        glyph: "sync"
                        text: "Procurar agora"
                        enabled: page.support.updateState !== DriveSupport.Checking && page.support.updateState !== DriveSupport.Downloading
                                 && page.support.updateState !== DriveSupport.Installing
                        onClicked: page.support.checkForUpdates()
                    }
                }
                Rectangle {
                    visible: page.support.updateState === DriveSupport.Downloading
                    Layout.fillWidth: true
                    Layout.leftMargin: 54
                    height: 6; radius: 3
                    color: DriveIcons.color.line
                    Rectangle { width: parent.width * Math.max(0, page.support.updateProgress); height: parent.height; radius: 3; color: DriveIcons.color.action }
                }
            }

            // relatório de problema
            Card {
                CardTitle { text: "Relatar um problema" }
                Body { text: "Conte o que aconteceu. Junto vai o registro dos últimos dias deste computador, para o suporte da DBPixel entender o problema e corrigir." }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 104
                    radius: 10
                    color: "white"
                    border.color: description.activeFocus ? DriveIcons.color.action : "#D0D5DD"
                    border.width: description.activeFocus ? 2 : 1
                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 2
                        TextArea {
                            id: description
                            wrapMode: TextEdit.Wrap
                            placeholderText: "Ex.: a planilha Estoque.xlsx não sobe desde ontem à tarde."
                            placeholderTextColor: "#98A2B3"
                            color: DriveIcons.color.ink
                            font.family: "Nunito Sans"
                            font.pixelSize: 14
                            selectByMouse: true
                            background: null
                            Accessible.name: "Descreva o problema"
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    DriveButton {
                        primary: true
                        glyph: page.support.reportState === DriveSupport.ReportSending ? "sync" : "whatsapp"
                        text: page.support.reportState === DriveSupport.ReportSending ? "Enviando…" : "Enviar para o suporte"
                        enabled: page.support.reportState !== DriveSupport.ReportSending && description.text.trim().length > 0
                        onClicked: page.support.sendReport(description.text)
                    }
                    DriveButton { small: true; glyph: "folder"; text: "Abrir a pasta do registro"; onClicked: page.support.openLogFolder() }
                    Item { Layout.fillWidth: true }
                }
                Body {
                    visible: page.support.reportText !== ""
                    text: page.support.reportText
                    color: page.support.reportState === DriveSupport.ReportFailed ? DriveIcons.color.warn
                         : page.support.reportState === DriveSupport.ReportSent ? DriveIcons.color.ok : DriveIcons.color.muted
                    font.weight: Font.DemiBold
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 4
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                text: "Baseado no ownCloud Desktop Client, distribuído sob a GNU GPL versão 2. O código-fonte desta versão está em <a href=\"https://github.com/dbpixel/dbpixel-drive\">github.com/dbpixel/dbpixel-drive</a>."
                linkColor: DriveIcons.color.steelText
                color: "#7D8390"
                font.family: "Nunito Sans"
                font.pixelSize: 12
                onLinkActivated: page.support.openSource()
                HoverHandler { cursorShape: parent.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor }
            }
        }
    }
}
