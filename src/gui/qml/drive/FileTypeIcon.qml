/*
 * Ícone do tipo de arquivo: quadrado na cor do tipo com o desenho em branco (PDF vermelho, documento azul,
 * planilha verde, apresentação laranja, imagem, vídeo, áudio, compactado, código, programa).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

import QtQuick
import QtQuick.Shapes

Rectangle {
    id: icon
    // extensão sem o ponto, em qualquer caixa ("PDF", "docx")
    property string ext
    property int size: 36
    readonly property var kind: kindOf(String(ext || "").toLowerCase())

    width: size
    height: size
    radius: Math.round(size / 4)
    color: kind.color
    Accessible.ignored: true

    function kindOf(e) {
        const groups = [
            { k: "pdf", color: "#D92D20", ext: ["pdf"] },
            { k: "doc", color: "#1570EF", ext: ["doc", "docx", "docm", "dot", "dotx", "odt", "rtf", "pages"] },
            { k: "sheet", color: "#1D7A46", ext: ["xls", "xlsx", "xlsm", "xlsb", "csv", "ods", "numbers"] },
            { k: "slides", color: "#D9480F", ext: ["ppt", "pptx", "pptm", "pps", "ppsx", "odp", "key"] },
            { k: "image", color: "#7A5AF8", ext: ["png", "jpg", "jpeg", "gif", "webp", "bmp", "tif", "tiff", "heic", "svg", "ico", "psd", "ai", "cdr", "raw"] },
            { k: "video", color: "#C11574", ext: ["mp4", "mov", "avi", "mkv", "wmv", "webm", "m4v", "mpg", "mpeg", "3gp"] },
            { k: "audio", color: "#0E9384", ext: ["mp3", "wav", "flac", "m4a", "aac", "ogg", "wma", "opus"] },
            { k: "archive", color: "#B54708", ext: ["zip", "rar", "7z", "tar", "gz", "tgz", "bz2", "xz", "cab", "iso"] },
            { k: "code", color: "#344054", ext: ["html", "htm", "css", "js", "ts", "json", "xml", "php", "py", "sql", "java", "c", "cpp", "h", "cs", "sh", "ps1", "yml", "yaml", "ini", "cfg"] },
            { k: "app", color: "#1D2939", ext: ["exe", "msi", "bat", "cmd", "dll", "apk", "appx", "msix"] },
            { k: "text", color: "#667085", ext: ["txt", "log", "md", "nfo"] },
            { k: "mail", color: "#0B4A8F", ext: ["eml", "msg", "pst", "ost"] }
        ];
        for (let i = 0; i < groups.length; ++i) {
            if (groups[i].ext.indexOf(e) >= 0)
                return groups[i];
        }
        return { k: "file", color: "#5E6573" };
    }

    // desenhos na grade de 24 px, traço branco
    readonly property var drawings: ({
        page: "M7 3h7l4 4v14H7zM14 3v4h4",
        pdf: "M7 3h7l4 4v14H7zM14 3v4h4M9.5 17.5c1.6-2.2 3-5.6 3-7.6 0-.8-.3-1.2-.7-1.2s-.7.4-.7 1.1c0 2.3 2.6 5.3 4.9 5.9.7.2 1.1 0 1.1-.4s-.4-.7-1.1-.7c-2.3 0-5.3 1.4-6.5 2.9z",
        doc: "M7 3h7l4 4v14H7zM14 3v4h4M10 12h5M10 15h5M10 18h3",
        sheet: "M5 5h14v14H5zM5 10h14M5 14.5h14M10 5v14",
        slides: "M4 5h16v11H4zM12 16v4M9 20h6M8 13v-2M12 13V9M16 13v-4",
        image: "M4 5h16v14H4zM4 16l5-5 4 4 2-2 5 5M15.5 9.5h.01",
        video: "M4 6h16v12H4zM10 9.5v5l4.5-2.5z",
        audio: "M9 18V6l10-2v12M9 18a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0zM19 16a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0z",
        archive: "M5 4h14v16H5zM12 4v2M12 8v2M12 12v2M10.5 14h3v3h-3z",
        code: "M9 8l-4 4 4 4M15 8l4 4-4 4M13 6l-2 12",
        app: "M4 5h16v14H4zM4 9h16M7 7h.01M9.5 7h.01",
        text: "M7 3h7l4 4v14H7zM14 3v4h4M10 12h5M10 15h5M10 18h5",
        mail: "M4 6h16v12H4zM4 7l8 6 8-6",
        file: "M7 3h7l4 4v14H7zM14 3v4h4"
    })

    Shape {
        anchors.centerIn: parent
        width: 24
        height: 24
        scale: icon.size * 0.58 / 24
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: "white"
            strokeWidth: 1.9
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathSvg { path: icon.drawings[icon.kind.k] || icon.drawings.file }
        }
    }
}
