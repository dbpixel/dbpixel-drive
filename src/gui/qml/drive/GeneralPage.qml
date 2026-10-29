/*
 * Drive (protótipo C2): página "Geral" das configurações. Opções com título, explicação e chave à direita,
 * idioma, arquivos que não sincronizam e o registro para o suporte.
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
    readonly property var settings: ocContext
    color: DriveIcons.color.page

    component Card: Rectangle {
        default property alias content: inner.data
        Layout.fillWidth: true
        Layout.preferredHeight: inner.implicitHeight + 16
        radius: 14
        color: "white"
        border.color: "#E3E6EB"
        ColumnLayout {
            id: inner
            anchors.fill: parent
            anchors.margins: 8
            anchors.leftMargin: 22
            anchors.rightMargin: 22
            spacing: 0
        }
    }

    component Toggle: AbstractButton {
        id: sw
        checkable: true
        implicitWidth: 44
        implicitHeight: 24
        focusPolicy: Qt.StrongFocus
        Accessible.role: Accessible.CheckBox
        background: Rectangle {
            radius: 12
            color: !sw.enabled ? "#D0D5DD" : (sw.checked ? DriveIcons.color.action : "#C4C9D2")
            border.color: sw.visualFocus ? DriveIcons.color.ink : "transparent"
            border.width: 2
            Rectangle {
                width: 18; height: 18; radius: 9
                y: 3
                x: sw.checked ? parent.width - width - 3 : 3
                color: "white"
                Behavior on x { NumberAnimation { duration: 120 } }
            }
        }
    }

    component Option: Item {
        id: opt
        property string title
        property string detail
        property alias checked: toggle.checked
        property alias toggleEnabled: toggle.enabled
        property bool last: false
        signal toggled(bool on)
        Layout.fillWidth: true
        implicitHeight: Math.max(64, texts.implicitHeight + 28)
        Rectangle { visible: !opt.last; anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#F0F1F3" }
        RowLayout {
            anchors.fill: parent
            spacing: 16
            ColumnLayout {
                id: texts
                Layout.fillWidth: true
                spacing: 2
                Label { Layout.fillWidth: true; text: opt.title; wrapMode: Text.WordWrap; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 14 }
                Label { Layout.fillWidth: true; text: opt.detail; wrapMode: Text.WordWrap; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 13 }
            }
            Toggle {
                id: toggle
                Accessible.name: opt.title
                onClicked: opt.toggled(checked)
            }
        }
        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: 70
            cursorShape: toggle.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (toggle.enabled) { toggle.toggle(); opt.toggled(toggle.checked); }
        }
    }

    component Field: ColumnLayout {
        property string label
        property string hint
        default property alias content: box.data
        Layout.fillWidth: true
        Layout.topMargin: 14
        Layout.bottomMargin: 6
        spacing: 6
        Label { text: parent.label; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.weight: Font.Bold; font.pixelSize: 13 }
        RowLayout { id: box; Layout.fillWidth: true; spacing: 10 }
        Label { visible: text !== ""; Layout.fillWidth: true; text: parent.hint; wrapMode: Text.WordWrap; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 12 }
    }

    component FieldText: Rectangle {
        property string text
        property string glyph: ""
        Layout.fillWidth: true
        implicitHeight: 42
        radius: 10
        color: "#FAFBFC"
        border.color: "#D0D5DD"
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10
            DriveGlyph { visible: parent.parent.glyph !== ""; name: parent.parent.glyph; size: 18; color: DriveIcons.color.navy }
            Label { Layout.fillWidth: true; text: parent.parent.text; elide: Text.ElideRight; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.pixelSize: 14 }
        }
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

            ColumnLayout {
                spacing: 2
                Label { text: "Geral"; color: DriveIcons.color.ink; font.family: "Montserrat"; font.weight: Font.DemiBold; font.pixelSize: 24 }
                Label { text: "Como o Drive se comporta neste computador"; color: DriveIcons.color.muted; font.family: "Nunito Sans"; font.pixelSize: 14 }
            }

            Card {
                Option {
                    title: "Abrir junto com o Windows"
                    detail: page.settings.autostartLocked ? "Definido pelo administrador deste computador" : "Recomendado para manter tudo sempre em dia"
                    checked: page.settings.autostart
                    toggleEnabled: !page.settings.autostartLocked
                    onToggled: on => page.settings.autostart = on
                }
                Option {
                    title: "Avisos na área de notificação"
                    detail: "Quando terminar uma sincronização grande ou algo der errado"
                    checked: page.settings.notifications
                    onToggled: on => page.settings.notifications = on
                }
                Option {
                    visible: page.settings.moveToTrashAvailable
                    title: "Mandar para a Lixeira"
                    detail: "Arquivos apagados na nuvem vão para a Lixeira deste PC, e não somem de vez"
                    checked: page.settings.moveToTrash
                    onToggled: on => page.settings.moveToTrash = on
                }
                Option {
                    title: "Pedir confirmação antes de apagar muitos arquivos"
                    detail: "Proteção contra exclusão por engano"
                    checked: page.settings.confirmDeletes
                    onToggled: on => page.settings.confirmDeletes = on
                }
                Option {
                    last: true
                    title: "Sincronizar arquivos ocultos"
                    detail: "Arquivos e pastas que o Windows esconde também vão para a nuvem"
                    checked: page.settings.syncHidden
                    onToggled: on => page.settings.syncHidden = on
                }
            }

            Card {
                Field {
                    label: "Idioma"
                    hint: page.settings.languageNeedsRestart ? "O idioma novo vale da próxima vez que o Drive abrir." : ""
                    ComboBox {
                        id: language
                        Layout.fillWidth: true
                        implicitHeight: 42
                        model: page.settings.languages
                        currentIndex: page.settings.languageIndex
                        onActivated: index => page.settings.languageIndex = index
                        font.family: "Nunito Sans"
                        font.pixelSize: 14
                        Accessible.name: "Idioma"
                        background: Rectangle { radius: 10; color: "#FAFBFC"; border.color: language.visualFocus ? DriveIcons.color.action : "#D0D5DD"; border.width: language.visualFocus ? 2 : 1 }
                        contentItem: Label { leftPadding: 14; verticalAlignment: Text.AlignVCenter; text: language.displayText; color: DriveIcons.color.ink; font: language.font; elide: Text.ElideRight }
                        indicator: DriveGlyph { x: language.width - width - 14; y: (language.height - height) / 2; name: "down"; size: 16; color: DriveIcons.color.muted }
                        delegate: ItemDelegate {
                            required property int index
                            required property string modelData
                            width: language.width
                            highlighted: language.highlightedIndex === index
                            contentItem: Label { text: modelData; color: DriveIcons.color.ink; font.family: "Nunito Sans"; font.pixelSize: 14 }
                            background: Rectangle { color: highlighted ? DriveIcons.color.steelSoft : "white" }
                        }
                        popup.background: Rectangle { radius: 10; color: "white"; border.color: "#D0D5DD" }
                    }
                }
                Field {
                    label: "Arquivos que não sincronizam"
                    hint: "Arquivos temporários e de sistema ficam só neste computador."
                    FieldText { text: "Temporários do Office, ~$*, .tmp, Thumbs.db e outros" }
                    DriveButton { small: true; text: "Editar"; onClicked: page.settings.editIgnoredFiles() }
                }
                Field {
                    label: "Registro para o suporte"
                    hint: "O Drive guarda um registro dos últimos dias. Ele vai junto quando você relata um problema em Sobre."
                    FieldText { text: "Registro dos últimos dias neste computador"; glyph: "folder" }
                    DriveButton { small: true; text: "Abrir a pasta"; onClicked: page.settings.openLogFolder() }
                }
                Item { Layout.preferredHeight: 10 }
            }
        }
    }
}
