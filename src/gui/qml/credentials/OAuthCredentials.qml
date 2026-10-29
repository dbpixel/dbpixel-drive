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

// Drive (protótipo W2): um botão principal para abrir o navegador; depois de aberto, o cartão
// "Aguardando você entrar…" com as ações secundárias. Cores do produto: ação #47689A, texto branco.
Credentials {
    id: oauthView
    readonly property QmlOAuthCredentials credentials: ocContext
    readonly property OCQuickWidget widget: ocQuickWidget
    property bool browserWasOpened: false

    StackLayout {
        id: stackLayout
        Layout.fillWidth: true
        Layout.fillHeight: false
        currentIndex: credentials.isValid ? 0 : 1

        ColumnLayout {
            spacing: 12

            // antes de abrir: uma ação clara
            DriveButton {
                id: openBrowserButton
                visible: !oauthView.browserWasOpened
                primary: true
                enabled: credentials.ready
                glyph: "ext"
                text: "Abrir o navegador para entrar"
                onClicked: {
                    oauthView.browserWasOpened = true;
                    credentials.openAuthenticationUrlInBrowser();
                }
                Keys.onBacktabPressed: {
                    widget.parentFocusWidget.focusPrevious();
                }
            }

            // depois de abrir: o que está acontecendo
            Rectangle {
                visible: oauthView.browserWasOpened
                Layout.fillWidth: true
                Layout.preferredHeight: 84
                radius: 14
                color: "#F4F5F7"
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 18
                    // indicador de espera na cor do produto
                    Item {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        Rectangle { anchors.fill: parent; radius: 20; color: "transparent"; border.color: "#DDE1E7"; border.width: 4 }
                        Rectangle {
                            width: 12; height: 12; radius: 6; color: "#47689A"
                            x: 14; y: -2
                        }
                        RotationAnimation on rotation { running: oauthView.browserWasOpened; from: 0; to: 360; duration: 1000; loops: Animation.Infinite }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Label { text: "Aguardando você entrar…"; color: "#0A1222"; font.family: "Montserrat"; font.weight: Font.DemiBold; font.pixelSize: 16 }
                        Label { text: "A página abriu no seu navegador padrão"; color: "#5E6573"; font.family: "Nunito Sans"; font.pixelSize: 13 }
                    }
                }
            }

            RowLayout {
                spacing: 10
                DriveButton {
                    visible: oauthView.browserWasOpened
                    enabled: credentials.ready
                    glyph: "ext"
                    text: "Abrir de novo"
                    onClicked: credentials.openAuthenticationUrlInBrowser()
                }
                DriveButton {
                    id: copyToClipboardButton
                    enabled: credentials.ready
                    glyph: "copy"
                    text: "Copiar o link"
                    onClicked: credentials.copyAuthenticationUrlToClipboard()
                    Keys.onTabPressed: event => {
                        // there is no logout button
                        if (!credentials.isRefresh) {
                            widget.parentFocusWidget.focusNext();
                            event.accepted = true;
                        }
                    }
                }
                Loader {
                    Layout.maximumHeight: 42
                    sourceComponent: logOutButton
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 8
                wrapMode: Text.WordWrap
                text: "Não tem acesso? Peça ao responsável pela conta para criar o seu usuário no painel da DBPixel."
                color: "#5E6573"
                font.family: "Nunito Sans"
                font.pixelSize: 13
            }
        }

        ColumnLayout {
            spacing: 12
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "Não deu para entrar. Tente de novo."
                color: "#B84000"
                font.family: "Montserrat"
                font.weight: Font.DemiBold
                font.pixelSize: 15
            }
            DriveButton {
                id: restartButton
                primary: true
                text: "Tentar de novo"
                onClicked: {
                    oauthView.browserWasOpened = false;
                    credentials.requestRestart();
                }
                Keys.onBacktabPressed: {
                    widget.parentFocusWidget.focusPrevious();
                }
            }
        }
    }

    Connections {
        target: widget

        function onFocusFirst() {
            stackLayout.nextItemInFocusChain(true).forceActiveFocus(Qt.TabFocusReason);
        }
    }
}
