// Ícones de traço do Drive (grade 24x24), os mesmos do protótipo das telas.
// Círculos foram convertidos em arcos para caberem num único caminho SVG.
.pragma library

var paths = {
    check: "M20 6 9 17l-5-5",
    user: "M8 8a4 4 0 1 0 8 0a4 4 0 1 0-8 0M4 21a8 8 0 0 1 16 0",
    wifi: "M2 9a15 15 0 0 1 20 0M5 12.5a10 10 0 0 1 14 0M8.5 16a5 5 0 0 1 7 0M12 19.5v.1",
    info: "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M12 11v5M12 8v.5",
    power: "M12 3v9M6.3 6.3a8 8 0 1 0 11.4 0",
    plus: "M12 5v14M5 12h14",
    down: "M6 9l6 6 6-6",
    more: "M5 12h.01M12 12h.01M19 12h.01",
    sync: "M21 12a9 9 0 0 1-15.4 6.4M3 12A9 9 0 0 1 18.4 5.6M18 2v4h-4M6 22v-4h4",
    alert: "M12 3 2 20h20zM12 10v4M12 17.5v.5",
    pause: "M9 5v14M15 5v14",
    play: "M7 4l13 8-13 8z",
    folder: "M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z",
    globe: "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18",
    lock: "M5 11h14v10H5zM8 11V7a4 4 0 0 1 8 0v4",
    whatsapp: "M4 20l1.3-4A8 8 0 1 1 8 18.7zM9 9.5c0 3 2.5 5.5 5.5 5.5l1-1.5-2-1-1 1a4 4 0 0 1-2-2l1-1-1-2z",
    cloud: "M7 18a5 5 0 0 1-.6-10A6 6 0 0 1 18 8.5 4.5 4.5 0 0 1 17.5 18z",
    ext: "M14 4h6v6M20 4l-9 9M18 14v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h5",
    copy: "M8 8h12v12H8zM16 8V6a2 2 0 0 0-2-2H6a2 2 0 0 0-2 2v8a2 2 0 0 0 2 2h2",
    drive: "M3 8a3 3 0 0 1 3-3h12a3 3 0 0 1 3 3v8a3 3 0 0 1-3 3H6a3 3 0 0 1-3-3zM7 15h.5M11 15h6",
    gear: "M9 12a3 3 0 1 0 6 0a3 3 0 1 0-6 0M19.4 15a1.7 1.7 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.8-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.8 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.8.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.8V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1z"
};

// cores do produto (manual 3.9) e da casca DBPixel, para as telas do Drive
var color = {
    navy: "#041F46",
    ink: "#0A1222",
    muted: "#5E6573",
    line: "#E6E8EC",
    soft: "#F4F5F7",
    page: "#F7F8FA",
    action: "#47689A",
    actionHover: "#3F5F8F",
    actionDown: "#3A5784",
    actionDisabled: "#C9D4E5",
    steel: "#6F8BB8",
    steelText: "#43638F",
    steelOnDark: "#9DB6DC",
    steelSoft: "#EBF0F7",
    ok: "#146C36",
    okSoft: "#E7F6EC",
    okBright: "#3DDC84",
    warn: "#B84000"
};
