/*
 * Fontes da marca do Drive (Montserrat nos títulos e botões, Nunito Sans no texto), embutidas no app
 * porque o Windows dos clientes não as tem. Licença SIL OFL 1.1 (arquivos em gui/fonts).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "gui/owncloudguilib.h"

namespace OCC::BrandFonts {

/// Registra as fontes uma vez por processo; pode ser chamado de qualquer tela.
OWNCLOUDGUI_EXPORT void load();

/// Fonte padrão (Nunito Sans) e estilo de menus, dicas e caixas de diálogo para o aplicativo inteiro.
OWNCLOUDGUI_EXPORT void applyToApplication();

}
