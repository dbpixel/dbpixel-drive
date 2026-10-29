/*
 * Tema DBPixel Drive. Incluído direto no theme.cpp (THEME_INCLUDE), por isso fica só no cabeçalho.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "theme.h"
#include "common/version.h"
#include "resources/resources.h"

#include <QColor>
#include <QIcon>
#include <QString>

namespace OCC {

class DBPixelTheme : public Theme
{
public:
    // o servidor já vem preenchido: o cliente só digita usuário e senha
    QString overrideServerUrl() const override { return QStringLiteral("https://drive.dbpixel.com.br"); }

    QString helpUrl() const override { return QStringLiteral("https://dbpixel.com.br/tutoriais/armazenamento-em-nuvem"); }

    // conteúdo do assistente em fundo branco com texto escuro (protótipo W1 a W5); a faixa navy com a
    // marca e as etapas é desenhada à parte em SetupWizardWidget::setupDriveLayout()
    QColor wizardHeaderBackgroundColor() const override { return Qt::white; }
    QColor wizardHeaderTitleColor() const override { return QColor(0x0A, 0x12, 0x22); }
    QIcon wizardHeaderLogo() const override { return Resources::themeUniversalIcon(QStringLiteral("wizard_logo")); }

    // Drive é um produto: a cor do produto (azul-aço) substitui o laranja (manual 3.9 e 9.2).
    // Botão principal no tom "botão" #47689A com texto branco (COR-14).
    QmlButtonColor primaryButtonColor() const override
    {
        const QColor steel(0x47, 0x68, 0x9A);
        return {steel, Qt::white, QColor(0xC9, 0xD4, 0xE5)};
    }
    QmlButtonColor secondaryButtonColor() const override { return {QColor(0xF4, 0xF5, 0xF7), QColor(0x0A, 0x12, 0x22), QColor(0x5E, 0x65, 0x73)}; }

    QColor avatarColor() const override { return QColor(0x04, 0x1F, 0x46); }
    QColor avatarColorChecked() const override { return QColor(0x6F, 0x8B, 0xB8); }

    bool moveToTrashDefaultValue() const override { return true; }

    // uma conta por instalação: o Drive fala só com o drive.dbpixel.com.br
    bool multiAccount() const override { return false; }

    // sem "Copyright ownCloud GmbH" nas configurações: a marca é deles (a menção à licença fica no "Sobre")
    bool aboutShowCopyright() const override { return false; }

    QString about() const override
    {
        return QStringLiteral("<p>Versão %1 · um produto DBPixel</p><p>Drive é o aplicativo de sincronização do Armazenamento em Nuvem da DBPixel.</p>"
                              "<p>Suporte: <a href=\"https://dbpixel.com.br\">dbpixel.com.br</a> · WhatsApp (13) 4042-3797</p>"
                              "<p><small>Baseado no ownCloud Desktop Client, distribuído sob a GNU GPL versão 2.</small></p>")
            .arg(Utility::escape(Version::displayString()));
    }
};

}
