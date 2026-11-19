<?php
/**
 * Tema "Drive" do ownCloud 10 (interface clássica, login e e-mails) para o drive.dbpixel.com.br.
 * Ativado com 'theme' => 'dbpixel' no config.php. Os textos e cores seguem os protótipos aprovados (L1, L2, P2, M1).
 */
class OC_Theme {
	public function getBaseUrl() {
		return 'https://dbpixel.com.br';
	}

	public function getSyncClientUrl() {
		return 'https://github.com/dbpixel/dbpixel-drive/releases/latest';
	}

	public function getTitle() {
		return 'Drive';
	}

	public function getName() {
		return 'Drive';
	}

	public function getHTMLName() {
		return 'Drive';
	}

	public function getEntity() {
		return 'DBPixel';
	}

	public function getSlogan() {
		return 'Drive · um produto DBPixel';
	}

	public function getLogoClaim() {
		return '';
	}

	public function getShortFooter() {
		return 'Drive · um produto <a href="https://dbpixel.com.br" target="_blank" rel="noreferrer">DBPixel</a>';
	}

	public function getLongFooter() {
		return 'Precisa de ajuda? <a href="https://wa.me/551340423797" target="_blank" rel="noreferrer">Fale com o suporte</a>';
	}

	public function getMailHeaderColor() {
		return '#041F46';
	}
}
