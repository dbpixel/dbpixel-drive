/*
 * Painel da bandeja do Drive (um produto DBPixel), protótipo P1 a P5.
 * Cores do produto (manual 3.9): botão #47689A com texto branco, base #6F8BB8, texto #43638F, sobre escuro #9DB6DC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import org.ownCloud.gui 1.0

Item {
    id: root
    readonly property TrayPanel panel: ocContext

    readonly property color navy: "#041F46"
    readonly property color ink: "#0A1222"
    readonly property color muted: "#5E6573"
    readonly property color line: "#E6E8EC"
    readonly property color soft: "#F4F5F7"
    readonly property color action: "#47689A"
    readonly property color steel: "#6F8BB8"
    readonly property color steelText: "#43638F"
    readonly property color steelOnDark: "#9DB6DC"
    readonly property color okGreen: "#146C36"
    readonly property color warnBand: "#B84000"
    readonly property string display: "Montserrat"
    readonly property string body: "Nunito Sans"

    readonly property color bandColor: panel.state === "problem" ? warnBand : (panel.state === "paused" || panel.state === "offline" || panel.state === "signedout") ? "#3A4252" : navy

    FontLoader { source: "../fonts/Montserrat-600.ttf" }
    FontLoader { source: "../fonts/Montserrat-700.ttf" }
    FontLoader { source: "../fonts/Montserrat-800.ttf" }
    FontLoader { source: "../fonts/NunitoSans-400.ttf" }
    FontLoader { source: "../fonts/NunitoSans-600.ttf" }
    FontLoader { source: "../fonts/NunitoSans-700.ttf" }

    // ícone de traço (grade 24x24), mesmo desenho do protótipo
    component Glyph: Item {
        id: g
        property string d
        property color color: root.ink
        property real stroke: 2
        property int size: 18
        width: size
        height: size
        Shape {
            width: 24
            height: 24
            scale: g.size / 24
            transformOrigin: Item.TopLeft
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: g.color
                strokeWidth: g.stroke
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                PathSvg { path: g.d }
            }
        }
    }

    // marca do produto: quadrado chanfrado azul-aço (cortes de 22%) com nuvem branca
    component Mark: Item {
        id: m
        property int size: 24
        width: size
        height: size
        Shape {
            width: 24
            height: 24
            scale: m.size / 24
            transformOrigin: Item.TopLeft
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: "transparent"
                fillColor: root.steel
                PathSvg { path: "M0 0 L18.7 0 L24 5.3 L24 24 L5.3 24 L0 18.7 Z" }
            }
            ShapePath {
                strokeColor: "transparent"
                fillColor: "white"
                PathSvg { path: "M7.4 16.6h9.2a3.3 3.3 0 0 0 .2-6.6 4.7 4.7 0 0 0-9 1.1 2.8 2.8 0 0 0-.4 5.5z" }
            }
        }
    }

    readonly property var icons: ({
        check: "M20 6 9 17l-5-5",
        sync: "M21 12a9 9 0 0 1-15.4 6.4M3 12A9 9 0 0 1 18.4 5.6M18 2v4h-4M6 22v-4h4",
        alert: "M12 3 2 20h20zM12 10v4M12 17.5v.5",
        pause: "M9 5v14M15 5v14",
        folder: "M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z",
        globe: "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18",
        gear: "M9 12a3 3 0 1 0 6 0a3 3 0 1 0-6 0M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z",
        whatsapp: "M4 20l1.3-4A8 8 0 1 1 8 18.7zM9 9.5c0 3 2.5 5.5 5.5 5.5l1-1.5-2-1-1 1a4 4 0 0 1-2-2l1-1-1-2z",
        power: "M12 3v9M6.3 6.3a8 8 0 1 0 11.4 0",
        wifi: "M2 9a15 15 0 0 1 20 0M5 12.5a10 10 0 0 1 14 0M8.5 16a5 5 0 0 1 7 0M12 19.5v.1",
        lock: "M5 11h14v10H5zM8 11V7a4 4 0 0 1 8 0v4",
        play: "M7 4l13 8-13 8z"
    })

    readonly property var stateIcon: ({ ok: "check", sync: "sync", problem: "alert", paused: "pause", offline: "wifi", signedout: "lock", empty: "folder" })

    // sombra simples em camadas (o painel flutua sobre a área de trabalho)
    Repeater {
        model: 4
        Rectangle {
            anchors.fill: card
            anchors.margins: -(index + 1) * 2
            radius: 14 + (index + 1) * 2
            color: "transparent"
            border.color: Qt.rgba(4 / 255, 31 / 255, 70 / 255, 0.07 - index * 0.015)
            border.width: 2
        }
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 12
        radius: 14
        color: "white"
        clip: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // faixa de cima: marca, conta e situação
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: header.implicitHeight + 34
                color: root.bandColor
                radius: 14
                Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 14; color: parent.color }
                clip: true

                // canto chanfrado na cor do produto
                Rectangle {
                    width: 68; height: 68; rotation: 45
                    x: parent.width - 34; y: -34
                    color: panel.state === "problem" ? root.navy : root.steel
                }

                ColumnLayout {
                    id: header
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 18
                    anchors.topMargin: 16
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.rightMargin: 30
                        spacing: 10
                        Mark { size: 24 }
                        Label { text: "Drive"; color: "white"; font.family: root.display; font.weight: Font.ExtraBold; font.pixelSize: 16 }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            visible: panel.accountInitials !== ""
                            width: 26; height: 26; radius: 13
                            color: Qt.rgba(1, 1, 1, 0.14)
                            Label { anchors.centerIn: parent; text: panel.accountInitials; color: "white"; font.family: root.display; font.weight: Font.Bold; font.pixelSize: 11 }
                        }
                        Label { text: panel.accountName; color: "#C9D2E0"; font.family: root.body; font.pixelSize: 13; elide: Text.ElideRight; Layout.maximumWidth: 140 }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 14
                        Rectangle {
                            width: 44; height: 44; radius: 12
                            color: Qt.rgba(1, 1, 1, 0.1)
                            Glyph {
                                anchors.centerIn: parent
                                size: 24; stroke: 2.2
                                d: root.icons[root.stateIcon[panel.state] || "check"]
                                color: panel.state === "ok" ? "#3DDC84" : panel.state === "sync" ? root.steelOnDark : "white"
                                RotationAnimation on rotation { running: panel.state === "sync"; from: 0; to: 360; duration: 1600; loops: Animation.Infinite }
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label { Layout.fillWidth: true; text: panel.title; color: "white"; font.family: root.display; font.weight: Font.DemiBold; font.pixelSize: 16; wrapMode: Text.WordWrap }
                            Label {
                                Layout.fillWidth: true; text: panel.detail; wrapMode: Text.WordWrap
                                color: panel.state === "problem" ? "#FFE3D1" : "#AEB9CC"; font.family: root.body; font.pixelSize: 13
                            }
                        }
                    }

                    Rectangle {
                        visible: panel.progress >= 0
                        Layout.fillWidth: true
                        height: 4; radius: 2
                        color: Qt.rgba(1, 1, 1, 0.16)
                        Rectangle { width: parent.width * Math.max(0, panel.progress); height: parent.height; radius: 2; color: root.steelOnDark }
                    }
                }
            }

            // meio: retomar/entrar, problemas, em andamento e recentes
            Flickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentHeight: middle.implicitHeight + 12
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                // barra fina, por cima da margem direita, sem empurrar nem cortar a lista
                ScrollBar.vertical: ScrollBar {
                    id: panelScroll
                    policy: ScrollBar.AsNeeded
                    width: 6
                    padding: 0
                    background: Item {}
                    // só aparece quando há o que rolar (o desenho próprio substitui a regra do estilo padrão)
                    contentItem: Rectangle {
                        implicitWidth: 4; radius: 2
                        visible: panelScroll.size < 1.0
                        color: Qt.rgba(10 / 255, 18 / 255, 34 / 255, 0.25)
                    }
                }

                ColumnLayout {
                    id: middle
                    width: parent.width
                    spacing: 0

                    // pausado, sem conexão ou sem login: uma ação clara
                    Rectangle {
                        visible: panel.state === "paused" || panel.state === "signedout"
                        Layout.fillWidth: true
                        Layout.margins: 8
                        Layout.preferredHeight: resumeCol.implicitHeight + 28
                        radius: 12
                        color: root.soft
                        ColumnLayout {
                            id: resumeCol
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            DriveButton {
                                primary: true
                                text: panel.state === "signedout" ? "Entrar pelo navegador" : "Retomar agora"
                                glyph: panel.state === "signedout" ? "lock" : "sync"
                                onClicked: panel.state === "signedout" ? panel.signIn() : panel.togglePause()
                            }
                            Label {
                                Layout.fillWidth: true; wrapMode: Text.WordWrap
                                text: panel.state === "signedout" ? "Por segurança, o acesso expirou. Seus arquivos ficam esperando até você entrar." : "Sem internet, a sincronização volta sozinha quando a rede voltar."
                                color: root.muted; font.family: root.body; font.pixelSize: 13
                            }
                        }
                    }

                    // problemas
                    Rectangle {
                        visible: panel.problems.length > 0
                        Layout.fillWidth: true
                        Layout.margins: 8
                        Layout.preferredHeight: probCol.implicitHeight + 24
                        radius: 12
                        color: "#FFF1E6"
                        ColumnLayout {
                            id: probCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 10
                            Repeater {
                                model: panel.problems.slice(0, 3)
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12
                                    FileBadge { ext: modelData.ext }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 1
                                        Label { Layout.fillWidth: true; text: modelData.name; elide: Text.ElideMiddle; color: root.ink; font.family: root.body; font.weight: Font.DemiBold; font.pixelSize: 14 }
                                        Label { Layout.fillWidth: true; text: modelData.message; wrapMode: Text.WordWrap; maximumLineCount: 2; elide: Text.ElideRight; color: "#7A3300"; font.family: root.body; font.pixelSize: 13 }
                                    }
                                }
                            }
                            DriveButton { text: "Ver o que houve"; small: true; onClicked: panel.openIssues() }
                        }
                    }

                    SectionHead { visible: panel.current.length > 0; title: "Agora"; link: panel.paused ? "" : "Pausar"; onLinkClicked: panel.togglePause() }
                    Repeater { model: panel.current; FileRow { entry: modelData } }

                    SectionHead { visible: panel.recent.length > 0; title: panel.current.length > 0 ? "Recentes" : "Arquivos recentes"; link: "Ver todos"; onLinkClicked: panel.openIssues() }
                    Repeater { model: panel.recent; FileRow { entry: modelData } }

                    Label {
                        visible: panel.recent.length === 0 && panel.current.length === 0 && panel.problems.length === 0 && panel.state !== "paused" && panel.state !== "signedout"
                        Layout.fillWidth: true
                        Layout.margins: 18
                        Layout.topMargin: 28
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: "Os arquivos que você salvar na pasta do Drive aparecem aqui assim que forem enviados."
                        color: root.muted; font.family: root.body; font.pixelSize: 13
                    }
                }
            }

            // rodapé: espaço e atalhos
            Rectangle { Layout.fillWidth: true; height: 1; color: root.line }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 18
                Layout.topMargin: 14
                Layout.bottomMargin: 16
                spacing: 12

                ColumnLayout {
                    visible: panel.quotaText !== ""
                    Layout.fillWidth: true
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: panel.quotaText; color: root.ink; font.family: root.body; font.weight: Font.Bold; font.pixelSize: 13 }
                        Item { Layout.fillWidth: true }
                        Label { text: panel.quotaPercentText; color: root.muted; font.family: root.body; font.pixelSize: 13 }
                    }
                    Rectangle {
                        visible: panel.quotaFraction >= 0
                        Layout.fillWidth: true
                        height: 6; radius: 3; color: root.soft
                        Rectangle { width: parent.width * Math.max(0, panel.quotaFraction); height: parent.height; radius: 3; color: panel.quotaFraction > 0.9 ? root.warnBand : root.navy }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    DriveButton { Layout.fillWidth: true; primary: true; glyph: "folder"; text: "Abrir pasta"; onClicked: panel.openFolder() }
                    DriveButton { Layout.fillWidth: true; glyph: "globe"; text: "Ver no site"; onClicked: panel.openWeb() }
                    DriveButton {
                        id: gearButton
                        glyph: "gear"; square: true
                        checked: menu.opened
                        Accessible.name: "Configurações"
                        onClicked: menu.opened ? menu.close() : menu.open()
                    }
                }
            }
        }

        // menu da engrenagem (P4)
        Popup {
            id: menu
            x: card.width - width - 18
            y: card.height - height - 66
            width: 250
            padding: 6
            background: Rectangle { radius: 12; color: "white"; border.color: Qt.rgba(10 / 255, 18 / 255, 34 / 255, 0.12) }
            contentItem: ColumnLayout {
                spacing: 0
                MenuRow { glyph: panel.paused ? "play" : "pause"; text: panel.paused ? "Retomar sincronização" : "Pausar sincronização"; onClicked: { menu.close(); panel.togglePause() } }
                MenuRow { glyph: "gear"; text: "Configurações"; onClicked: { menu.close(); panel.openSettings() } }
                MenuRow { glyph: "whatsapp"; text: "Ajuda pelo WhatsApp"; onClicked: { menu.close(); panel.openHelp() } }
                MenuRow { glyph: "globe"; text: "Abrir drive.dbpixel.com.br"; onClicked: { menu.close(); panel.openWeb() } }
                Rectangle { Layout.fillWidth: true; Layout.topMargin: 4; Layout.bottomMargin: 4; height: 1; color: root.line }
                MenuRow { glyph: "power"; text: "Sair do Drive"; onClicked: { menu.close(); panel.quit() } }
            }
        }
    }

    // ---------------------------------------------------------------- peças
    component DriveButton: AbstractButton {
        id: b
        property bool primary: false
        property bool small: false
        property bool square: false
        property string glyph: ""
        implicitHeight: small ? 34 : 40
        implicitWidth: square ? implicitHeight : row.implicitWidth + 24
        hoverEnabled: true
        background: Rectangle {
            radius: 10
            color: b.primary ? (b.down ? "#3A5784" : b.hovered ? "#3F5F8F" : root.action) : (b.checked ? root.navy : b.hovered ? "#E9EBEF" : root.soft)
            border.color: b.primary || b.checked ? "transparent" : "#D9DCE2"
        }
        contentItem: Item {
            RowLayout {
                id: row
                anchors.centerIn: parent
                spacing: 8
                Glyph { visible: b.glyph !== ""; d: root.icons[b.glyph] || ""; size: b.square ? 18 : 16; color: b.primary || b.checked ? "white" : root.ink }
                Label { visible: b.text !== ""; text: b.text; color: b.primary ? "white" : root.ink; font.family: root.display; font.weight: Font.DemiBold; font.pixelSize: b.small ? 13 : 13 }
            }
        }
    }

    // ícone do tipo do arquivo (PDF vermelho, documento azul, planilha verde…)
    component FileBadge: FileTypeIcon {
        size: 36
    }

    component FileRow: ItemDelegate {
        id: fr
        property var entry
        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.rightMargin: 8
        implicitHeight: 54
        hoverEnabled: true
        onClicked: panel.openFile(entry.path)
        background: Rectangle { radius: 10; color: fr.hovered ? root.soft : "transparent" }
        contentItem: RowLayout {
            spacing: 12
            FileBadge { ext: fr.entry.ext }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1
                Label { Layout.fillWidth: true; text: fr.entry.name; elide: Text.ElideMiddle; color: root.ink; font.family: root.body; font.weight: Font.DemiBold; font.pixelSize: 14 }
                Label { Layout.fillWidth: true; text: fr.entry.folder; elide: Text.ElideLeft; color: root.muted; font.family: root.body; font.pixelSize: 13 }
                Rectangle {
                    visible: fr.entry.progress >= 0
                    Layout.fillWidth: true; Layout.topMargin: 4
                    height: 3; radius: 2; color: root.line
                    Rectangle { width: parent.width * Math.max(0, fr.entry.progress || 0); height: parent.height; radius: 2; color: root.navy }
                }
            }
            ColumnLayout {
                spacing: 1
                Label { Layout.alignment: Qt.AlignRight; text: fr.entry.status || ""; color: fr.entry.up && fr.entry.progress < 0 ? root.okGreen : root.muted; font.family: root.body; font.weight: fr.entry.up && fr.entry.progress < 0 ? Font.Bold : Font.Normal; font.pixelSize: 12 }
                Label { Layout.alignment: Qt.AlignRight; visible: !!fr.entry.time; text: fr.entry.time || ""; color: root.muted; font.family: root.body; font.pixelSize: 12 }
            }
        }
    }

    component SectionHead: RowLayout {
        id: sh
        property string title
        property string link
        signal linkClicked
        Layout.fillWidth: true
        Layout.leftMargin: 18
        Layout.rightMargin: 18
        Layout.topMargin: 14
        Layout.bottomMargin: 4
        Label { text: title; color: root.ink; font.family: root.display; font.weight: Font.DemiBold; font.pixelSize: 13 }
        Item { Layout.fillWidth: true }
        Label {
            visible: link !== ""
            text: link; color: root.steelText; font.family: root.body; font.weight: Font.Bold; font.pixelSize: 13
            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: sh.linkClicked() }
        }
    }

    component MenuRow: ItemDelegate {
        id: mr
        property string glyph
        Layout.fillWidth: true
        implicitHeight: 40
        hoverEnabled: true
        background: Rectangle { radius: 8; color: mr.hovered ? root.soft : "transparent" }
        contentItem: RowLayout {
            spacing: 12
            Glyph { d: root.icons[mr.glyph] || ""; size: 17; color: root.ink }
            Label { Layout.fillWidth: true; text: mr.text; color: root.ink; font.family: root.body; font.pixelSize: 14 }
        }
    }
}
