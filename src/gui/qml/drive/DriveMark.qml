/*
 * Marca do Drive (manual LOG-16 e 9.2): quadrado chanfrado (cortes de 22% em cima à direita e embaixo à
 * esquerda) com a nuvem branca, mais o nome em Montserrat 800.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import "DriveIcons.js" as DriveIcons

Row {
    id: m
    property int size: 30
    property bool showName: true
    property color nameColor: "white"
    property color tileColor: DriveIcons.color.steel
    spacing: Math.round(size * 0.36)

    Item {
        width: m.size
        height: m.size
        anchors.verticalCenter: parent.verticalCenter
        Shape {
            width: 24
            height: 24
            scale: m.size / 24
            transformOrigin: Item.TopLeft
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: "transparent"
                fillColor: m.tileColor
                PathSvg { path: "M0 0 L18.7 0 L24 5.3 L24 24 L5.3 24 L0 18.7 Z" }
            }
            ShapePath {
                strokeColor: "transparent"
                fillColor: "white"
                PathSvg { path: "M7.4 16.6h9.2a3.3 3.3 0 0 0 .2-6.6 4.7 4.7 0 0 0-9 1.1 2.8 2.8 0 0 0-.4 5.5z" }
            }
        }
    }

    Label {
        visible: m.showName
        anchors.verticalCenter: parent.verticalCenter
        text: "Drive"
        color: m.nameColor
        font.family: "Montserrat"
        font.weight: Font.ExtraBold
        font.pixelSize: Math.round(m.size * 0.66)
    }
}
