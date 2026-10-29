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

import QtQuick

// specifically use the basic style, as we modify the palette
import QtQuick.Controls.Basic
import QtQuick.Layouts

import org.ownCloud.resources 1.0
import org.ownCloud.gui 1.0
import org.ownCloud.libsync 1.0

Pane {
    id: credentialsView

    //TODO
    //required property QmlCredentials credentials
    readonly property OCQuickWidget widget: ocQuickWidget

    default property alias content: contentLayout.data
    property alias logOutButton: logutButtonComponent

    palette.window: Theme.brandedBackgoundColor
    palette.windowText: Theme.brandedForegroundColor

    Component {
        id: logutButtonComponent

        Button {
            id: logOutButton
            visible: credentials.isRefresh
            text: qsTr("Stay logged out")
            onClicked: credentials.logOutRequested()

            Keys.onTabPressed: event => {
                if (credentials.isRefresh) {
                    event.accepted = false;
                } else {
                    widget.parentFocusWidget.focusNext();
                }
            }

            Component.onCompleted: {
                if (Theme.secondaryButtonColor.valid) {
                    palette.button = Theme.secondaryButtonColor.color;
                    palette.buttonText = Theme.secondaryButtonColor.textColor;
                    palette.disabled.buttonText = Theme.secondaryButtonColor.textColorDisabled;
                }
            }
        }
    }

    Component.onCompleted: {
        if (Theme.primaryButtonColor.valid) {
            palette.button = Theme.primaryButtonColor.color;
            palette.buttonText = Theme.primaryButtonColor.textColor;
            palette.disabled.buttonText = Theme.primaryButtonColor.textColorDisabled;
        }
    }

    // Drive (protótipo W2): conteúdo alinhado em cima, em fundo branco; o título da etapa fica
    // na coluna da esquerda do assistente, então aqui não repete logo nem título
    padding: 24

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 14

        Label {
            Layout.fillWidth: true
            text: credentials.isRefresh ? "Entrar de novo como %1 em %2".arg(credentials.displayName).arg(credentials.host) : "Servidor: %1".arg(credentials.host)
            color: "#5E6573"
            font.family: "Nunito Sans"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            spacing: 12
        }
    }

    Connections {
        target: widget

        function onFocusLast() {
            if (logOutButton.visible) {
                logOutButton.forceActiveFocus(Qt.TabFocusReason);
            } else {
                logOutButton.nextItemInFocusChain(false).forceActiveFocus(Qt.TabFocusReason);
            }
        }
    }
}
