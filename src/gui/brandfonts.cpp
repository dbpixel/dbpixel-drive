/*
 * Fontes da marca do Drive.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gui/brandfonts.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QQuickWindow>
#include <QStringList>

void OCC::BrandFonts::load()
{
    static bool loaded = false;
    if (loaded) {
        return;
    }
    loaded = true;
    // mesmos arquivos que o painel da bandeja carrega pelo QML (recursos do módulo org.ownCloud.gui)
    const QStringList files = {QStringLiteral("Montserrat-600"), QStringLiteral("Montserrat-700"), QStringLiteral("Montserrat-800"),
        QStringLiteral("NunitoSans-400"), QStringLiteral("NunitoSans-600"), QStringLiteral("NunitoSans-700")};
    for (const auto &f : files) {
        QFontDatabase::addApplicationFont(QStringLiteral(":/qt/qml/org/ownCloud/gui/fonts/%1.ttf").arg(f));
    }
}

void OCC::BrandFonts::applyToApplication()
{
    load();
    // texto das telas em QML desenhado pelo Windows (com hinting), igual ao das telas em QWidget; a renderização
    // própria do Qt Quick deixa as letras borradas em 100% e 125% de escala. Precisa vir antes da primeira janela QML.
    QQuickWindow::setTextRenderType(QQuickWindow::NativeTextRendering);
    QFont font(QStringLiteral("Nunito Sans"), 10);
    font.setStyleStrategy(QFont::PreferAntialias);
    QApplication::setFont(font);
    // as telas próprias (configurações, assistente) têm folhas de estilo mais específicas por cima desta
    qApp->setStyleSheet(QStringLiteral(
        "QToolTip { background: #FFFFFF; color: #0A1222; border: 1px solid #D0D5DD; padding: 6px 8px; font-family: 'Nunito Sans'; font-size: 13px; }"
        "QMenu { background: #FFFFFF; border: 1px solid #E3E6EB; padding: 6px; font-family: 'Nunito Sans'; font-size: 13px; color: #0A1222; }"
        "QMenu::item { padding: 8px 28px 8px 14px; border-radius: 6px; background: transparent; }"
        "QMenu::item:selected { background: #EBF0F7; color: #0A1222; }"
        "QMenu::item:disabled { color: #98A2B3; }"
        "QMenu::separator { height: 1px; background: #EEF0F3; margin: 4px 8px; }"
        "QMenu::icon { padding-left: 8px; }"
        "QMessageBox, QDialog { background: #FFFFFF; }"
        "QMessageBox QLabel { color: #0A1222; font-family: 'Nunito Sans'; font-size: 14px; }"
        "QDialog QLabel { color: #0A1222; }"
        "QDialog QPushButton, QMessageBox QPushButton { min-height: 34px; min-width: 88px; padding: 0 16px; border-radius: 9px;"
        "  border: 1px solid #D9DCE2; background: #F4F5F7; color: #0A1222; font-family: Montserrat; font-weight: 600; font-size: 13px; }"
        "QDialog QPushButton:hover, QMessageBox QPushButton:hover { background: #E9EBEF; }"
        "QDialog QPushButton:default, QMessageBox QPushButton:default { background: #47689A; border-color: #47689A; color: #FFFFFF; }"
        "QDialog QPushButton:default:hover, QMessageBox QPushButton:default:hover { background: #3F5F8F; }"
        "QDialog QPushButton:disabled { color: #98A2B3; background: #F4F5F7; }"
        "QDialog QLineEdit { min-height: 32px; padding: 0 10px; border: 1px solid #D0D5DD; border-radius: 8px; background: #FFFFFF; color: #0A1222; }"
        "QDialog QLineEdit:focus { border: 2px solid #47689A; }"
        "QDialog QTableView, QDialog QTreeView, QDialog QListView { border: 1px solid #E3E6EB; border-radius: 10px; background: #FFFFFF;"
        "  alternate-background-color: #F7F8FA; selection-background-color: #EBF0F7; selection-color: #0A1222; gridline-color: #F0F1F3; }"
        "QHeaderView::section { background: #F7F8FA; color: #5E6573; border: 0; border-bottom: 1px solid #E3E6EB; padding: 8px 10px;"
        "  font-family: 'Nunito Sans'; font-weight: 700; font-size: 12px; }"
        "QDialog QGroupBox { font-family: Montserrat; font-weight: 600; font-size: 14px; color: #0A1222; }"));
}
