/*
 * Ícone de traço do Drive (grade 24x24), desenhado com QtQuick.Shapes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Shapes
import "DriveIcons.js" as DriveIcons

Item {
    id: g
    property string name
    property color color: DriveIcons.color.ink
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
            PathSvg { path: DriveIcons.paths[g.name] || "" }
        }
    }
}
