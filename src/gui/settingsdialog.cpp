/*
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "accountmanager.h"
#include "accountsettings.h"
#include "activitywidget.h"
#include "application.h"
#include "configfile.h"
#include "generalsettings.h"
#include "gui/brandfonts.h"
#include "gui/drivesupport.h"
#include "gui/qmlutils.h"
#include "owncloudgui.h"
#include "resources/qmlresources.h"
#include "resources/resources.h"
#include "theme.h"

#include <QAbstractButton>
#include <QDesktopServices>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QScrollArea>
#include <QWindow>

#ifdef Q_OS_MAC
#include "settingsdialog_mac.h"

void setActivationPolicy(ActivationPolicy policy);
#endif

Q_LOGGING_CATEGORY(lcSettingsDialog, "gui.settingsdialog", QtInfoMsg);

namespace {
auto minimumSizeHint(const QWidget *w)
{
    const QSize min { 980, 700 }; // Drive: menu lateral de 236 px + cartões das páginas // When changing this, please check macOS: widgets there have larger insets, so they take up more space.
    const auto screen = w->windowHandle() ? w->windowHandle()->screen() : QApplication::screenAt(QCursor::pos());
    if (screen) {
        const auto availableSize = screen->availableSize();
        if (availableSize.isValid()) {
            // Assume we can use at least 90% of the screen, if the screen is smaller than 800x700 pixels.
            //
            // Note: this means that the wizards have even less space: with the style we use, the
            // wizard tries to fit inside the window. So, if this is a common case that users have
            // such small screens, and the contents of the wizard screen are squashed together (or
            // not shown due to lack of space), we should consider putting that content in a
            // scroll-view.
            return min.boundedTo(availableSize * 0.9);
        }
    }
    return min;
}


class AvatarImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    AvatarImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Pixmap, QQuickImageProvider::ForceAsynchronousImageLoading)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        const auto qmlIcon = OCC::Resources::QMLResources::parseIcon(id);
        const auto accountState = OCC::AccountManager::instance()->accountState(QUuid::fromString(qmlIcon.iconName));
        return OCC::Resources::pixmap(requestedSize, accountState->account()->avatar(), qmlIcon.enabled ? QIcon::Normal : QIcon::Disabled, size);
    }
};
}


namespace {
// Drive (protótipo C1 a C5): cartões brancos, chaves liga/desliga, campos e botões do produto nas páginas
const auto drivePagesStyle = QStringLiteral(
    "QStackedWidget#stack { background: #F7F8FA; }"
    "QScrollArea { background: #F7F8FA; border: 0; }"
    "QScrollArea > QWidget > QWidget { background: #F7F8FA; }"
    "QLabel#drivePageTitle { font-family: Montserrat; font-size: 22px; font-weight: 600; color: #0A1222; }"
    "QLabel#drivePageSub { font-family: 'Nunito Sans'; font-size: 13px; color: #5E6573; margin-bottom: 6px; }"
    "QLabel#driveEmptyState { background: #E7F6EC; color: #146C36; border-radius: 12px; padding: 16px 18px; font-family: 'Nunito Sans'; font-size: 14px; font-weight: 700; }"
    "QGroupBox { background: #FFFFFF; border: 1px solid #E3E6EB; border-radius: 14px; margin-top: 14px; padding: 44px 18px 14px 18px;"
    "  font-family: Montserrat; font-size: 15px; font-weight: 600; color: #0A1222; }"
    "QGroupBox::title { subcontrol-origin: padding; subcontrol-position: top left; left: 18px; top: 16px; }"
    "QGroupBox QGroupBox { background: transparent; border: 0; border-top: 1px solid #F0F1F3; border-radius: 0; margin-top: 8px; padding: 34px 0 0 0;"
    "  font-size: 13px; }"
    "QGroupBox QGroupBox::title { left: 0px; top: 10px; }"
    "QCheckBox { font-family: 'Nunito Sans'; font-size: 14px; color: #0A1222; spacing: 12px; min-height: 32px; }"
    "QCheckBox::indicator { width: 40px; height: 22px; }"
    "QCheckBox::indicator:unchecked { image: url(:/qt/qml/org/ownCloud/gui/qml/drive/toggle-off.svg); }"
    "QCheckBox::indicator:checked { image: url(:/qt/qml/org/ownCloud/gui/qml/drive/toggle-on.svg); }"
    "QCheckBox::indicator:unchecked:disabled { image: url(:/qt/qml/org/ownCloud/gui/qml/drive/toggle-off-disabled.svg); }"
    "QCheckBox::indicator:checked:disabled { image: url(:/qt/qml/org/ownCloud/gui/qml/drive/toggle-on-disabled.svg); }"
    "QRadioButton { font-family: 'Nunito Sans'; font-size: 14px; color: #0A1222; spacing: 10px; min-height: 30px; }"
    "QRadioButton::indicator { width: 14px; height: 14px; border-radius: 9px; border: 2px solid #7D8390; background: #FFFFFF; }"
    "QRadioButton::indicator:checked { width: 8px; height: 8px; border-radius: 9px; border: 5px solid #47689A; background: #FFFFFF; }"
    "QLabel { font-family: 'Nunito Sans'; font-size: 13px; color: #0A1222; }"
    "QPushButton, QToolButton { background: #F4F5F7; border: 1px solid #D9DCE2; border-radius: 10px; padding: 8px 16px;"
    "  font-family: Montserrat; font-size: 13px; font-weight: 600; color: #0A1222; }"
    "QPushButton:hover, QToolButton:hover { background: #E9EBEF; }"
    "QPushButton:disabled { color: #9AA0AC; }"
    "QComboBox, QLineEdit, QSpinBox { min-height: 34px; border: 1px solid #7D8390; border-radius: 10px; padding: 0 10px; background: #FFFFFF;"
    "  font-family: 'Nunito Sans'; font-size: 14px; color: #0A1222; }"
    "QComboBox:disabled, QLineEdit:disabled, QSpinBox:disabled { color: #9AA0AC; border-color: #D5D8DE; background: #F7F8FA; }"
    "QComboBox::drop-down { border: 0; width: 28px; }"
    "QSpinBox::up-button, QSpinBox::down-button { width: 0px; border: 0; }"
    "QTabWidget::pane { border: 1px solid #E3E6EB; border-radius: 12px; background: #FFFFFF; top: -1px; }"
    "QTabBar::tab { background: transparent; padding: 10px 18px; margin-right: 4px; font-family: Montserrat; font-size: 13px; font-weight: 600;"
    "  color: #5E6573; border: 0; border-bottom: 3px solid transparent; }"
    "QTabBar::tab:selected { color: #0A1222; border-bottom: 3px solid #47689A; }"
    "QHeaderView::section { background: #FFFFFF; border: 0; border-bottom: 1px solid #E6E8EC; padding: 8px; font-family: Montserrat;"
    "  font-size: 12px; font-weight: 600; color: #5E6573; }"
    "QTreeView, QTableView, QListView { border: 0; background: #FFFFFF; font-family: 'Nunito Sans'; font-size: 13px;"
    "  selection-background-color: #EBF0F7; selection-color: #0A1222; }"
    "QScrollBar:vertical { width: 8px; background: transparent; margin: 2px; }"
    "QScrollBar::handle:vertical { background: #C9CED6; border-radius: 4px; min-height: 30px; }"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    "QScrollBar:horizontal { height: 8px; background: transparent; margin: 2px; }"
    "QScrollBar::handle:horizontal { background: #C9CED6; border-radius: 4px; min-width: 30px; }"
    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }");

// página com título e subtítulo no alto, como no protótipo; `content` vai logo abaixo
QWidget *drivePage(const QString &title, const QString &subtitle, QWidget *content, bool scroll)
{
    auto *inner = new QWidget;
    auto *layout = new QVBoxLayout(inner);
    layout->setContentsMargins(30, 26, 30, 26);
    layout->setSpacing(4);
    auto *t = new QLabel(title, inner);
    t->setObjectName(QStringLiteral("drivePageTitle"));
    auto *s = new QLabel(subtitle, inner);
    s->setObjectName(QStringLiteral("drivePageSub"));
    s->setWordWrap(true);
    layout->addWidget(t);
    layout->addWidget(s);
    layout->addWidget(content, scroll ? 0 : 1);
    if (!scroll) {
        return inner;
    }
    auto *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidget(inner);
    layout->addStretch();
    return area;
}
}

namespace OCC {

SettingsDialog::SettingsDialog(ownCloudGui *gui, QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::SettingsDialog)
    , _gui(gui)
{
    setObjectName(QStringLiteral("Settings")); // required as group for saveGeometry call
    setWindowTitle(Theme::instance()->appNameGUI());
    _ui->setupUi(this);

    setMinimumSize(::minimumSizeHint(this));

    // People perceive this as a Window, so also make Ctrl+W work
    addAction(tr("Hide"), Qt::CTRL | Qt::Key_W, this, &SettingsDialog::hide);

    // Drive (protótipo C1 a C5): o menu vira uma coluna navy à esquerda; as páginas ficam à direita
    BrandFonts::load();
    _ui->verticalLayout_2->removeWidget(_ui->quickWidget);
    _ui->verticalLayout_2->removeWidget(_ui->stack);
    auto *row = new QHBoxLayout;
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);
    row->addWidget(_ui->quickWidget);
    row->addWidget(_ui->stack, 1);
    _ui->verticalLayout_2->addLayout(row);
    _ui->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
    _ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    _ui->quickWidget->setFixedWidth(236);
    _ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    _ui->stack->setStyleSheet(drivePagesStyle);
    _ui->quickWidget->engine()->addImageProvider(QStringLiteral("avatar"), new AvatarImageProvider);
    _ui->quickWidget->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/AccountBar.qml")), this);
    connect(
        _ui->quickWidget->engine(), &QQmlEngine::quit, QApplication::instance(),
        [this] {
            auto box = new QMessageBox(QMessageBox::Question, QStringLiteral("Sair do Drive"),
                QStringLiteral("Ao sair, seus arquivos param de sincronizar até o Drive abrir de novo."), QMessageBox::Yes | QMessageBox::No, this);
            box->setAttribute(Qt::WA_DeleteOnClose);
            box->button(QMessageBox::Yes)->setText(QStringLiteral("Sair do Drive"));
            box->button(QMessageBox::No)->setText(QStringLiteral("Continuar"));
            connect(box, &QMessageBox::accepted, this, [] {
                // delay quit to prevent a Qt 6.6 crash in the destructor of the dialog
                QTimer::singleShot(0, qApp, &QCoreApplication::quit);
            });
            box->open();
        },
        Qt::QueuedConnection);

    _activitySettings = new ActivitySettings;
    _activityPage = drivePage(QStringLiteral("Problemas e atividade"),
        QStringLiteral("O que não sincronizou e o que mudou por último. Arquivos com problema voltam a tentar sozinhos."), _activitySettings, false);
    _ui->stack->addWidget(_activityPage);
    connect(_activitySettings, &ActivitySettings::guiLog, _gui,
        [this](const QString &title, const QString &msg) {
            _gui->slotShowOptionalTrayMessage(title, msg);
        });
    ConfigFile cfg;
    _activitySettings->setNotificationRefreshInterval(cfg.notificationRefreshInterval());

    // Geral (protótipo C2) em QML; GeneralSettings fica escondido guardando as opções
    _generalSettings = new GeneralSettings(this);
    _generalSettings->hide();
    auto *general = new QmlUtils::OCQuickWidget;
    general->setResizeMode(QQuickWidget::SizeRootObjectToView);
    general->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/drive/GeneralPage.qml")), this, _generalSettings, QJSEngine::CppOwnership);
    _generalPage = general;
    _ui->stack->addWidget(_generalPage);
    // Rede em página própria (protótipo C3)
    auto *networkCard = new QGroupBox;
    networkCard->setTitle(QStringLiteral("Conexão"));
    auto *networkLayout = new QVBoxLayout(networkCard);
    networkLayout->addWidget(_generalSettings->takeNetworkSettings());
    _networkPage = drivePage(QStringLiteral("Rede"), QStringLiteral("Para quando a internet é compartilhada ou passa por proxy"), networkCard, true);
    _ui->stack->addWidget(_networkPage);
    connect(_generalSettings, &GeneralSettings::showAbout, this, &SettingsDialog::openAbout);
    // Sobre (protótipo C5) é uma página das configurações, não uma janela à parte
    auto *about = new QmlUtils::OCQuickWidget;
    about->setResizeMode(QQuickWidget::SizeRootObjectToView);
    about->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/drive/AboutPage.qml")), this, DriveSupport::instance(), QJSEngine::CppOwnership);
    _aboutPage = about;
    _ui->stack->addWidget(_aboutPage);
    connect(_activitySettings, &ActivitySettings::issueCountChanged, this, [this](int count) {
        _issueCount = count;
        Q_EMIT issueCountChanged();
    });
    connect(_generalSettings, &GeneralSettings::syncOptionsChanged, FolderMan::instance(), &FolderMan::slotReloadSyncOptions);

    cfg.restoreGeometry(this);
#ifdef Q_OS_MAC
    setActivationPolicy(ActivationPolicy::Accessory);
#endif

    connect(_ui->dialogStack, &QStackedWidget::currentChanged, this, [this] {
        auto *w = _ui->dialogStack->currentWidget();
        if (!w->windowTitle().isEmpty()) {
            setWindowTitle(tr("%1 - %2").arg(Theme::instance()->appNameGUI(), w->windowTitle()));
        } else {
            setWindowTitle(Theme::instance()->appNameGUI());
        }
    });

    setCurrentPage(SettingsPage::Settings);
    auto addAccount = [this](AccountStatePtr accountStatePtr) {
        if (!accountStatePtr)
            return;
        auto accountSettings = new AccountSettings(accountStatePtr, this);
        _ui->stack->addWidget(accountSettings);
        _widgetForAccount.insert(accountStatePtr->account().data(), accountSettings);
        // select the first added account
        if (_widgetForAccount.size() == 1) {
            setCurrentAccount(accountStatePtr->account().data());
        }
    };
    for (const auto &accountState : AccountManager::instance()->accounts()) {
        addAccount(accountState);
    }
    // Refactor todo: make these real functions. Naive abuse of lambdas obfuscates the responsibilities of the class as important
    // functionality is hidden in the impl instead of being clearly visible in the interface. This is very bad practice and we
    // need to start correcting that to support maintainability.
    connect(AccountManager::instance(), &AccountManager::accountAdded, this, addAccount);
    connect(AccountManager::instance(), &AccountManager::accountRemoved, this, [this](AccountStatePtr accountStatePtr) {
        if (!accountStatePtr)
            return;
        Account *acc = accountStatePtr->account().data();
        if (AccountSettings *asw = _widgetForAccount.value(acc)) {
            _ui->stack->removeWidget(asw);
            _widgetForAccount.remove(acc);
            asw->deleteLater();
            // go to the settings page if the last account was removed
            if (_widgetForAccount.isEmpty()) {
                _ui->stack->setCurrentWidget(_generalPage);
            }
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    delete _ui;
}

void SettingsDialog::addModalWidget(QWidget *w)
{
    ownCloudGui::raise();
    if (_ui->dialogStack->indexOf(w) == -1) {
        _ui->dialogStack->addWidget(w);
        _ui->dialogStack->setCurrentWidget(w);
    }
}

void SettingsDialog::requestModality(Account *account)
{
    _ui->quickWidget->setEnabled(false);
    if (_modalStack.isEmpty()) {
        setCurrentAccount(account);
    }
    _modalStack.append(account);
    ownCloudGui::raise();
}

void SettingsDialog::ceaseModality(Account *account)
{
    if (_modalStack.contains(account)) {
        _modalStack.removeOne(account);
        if (!_modalStack.isEmpty()) {
            setCurrentAccount(_modalStack.first());
        }
    }
    _ui->quickWidget->setEnabled(_modalStack.isEmpty());
}

AccountSettings *SettingsDialog::accountSettings(Account *account) const
{
    return _widgetForAccount.value(account, nullptr);
}

void SettingsDialog::setVisible(bool visible)
{
    if (!visible) {
        ConfigFile cfg;
        cfg.saveGeometry(this);
    }

#ifdef Q_OS_MAC
    if (visible) {
        setActivationPolicy(ActivationPolicy::Regular);
    } else {
        setActivationPolicy(ActivationPolicy::Accessory);
    }
#endif
    QMainWindow::setVisible(visible);
}

void SettingsDialog::setCurrentPage(SettingsPage currentPage)
{
    _currentPage = currentPage;
    _currentAccount = nullptr;
    switch (_currentPage) {
    case SettingsPage::Activity:
        _ui->stack->setCurrentWidget(_activityPage);
        _activitySettings->slotShowIssuesTab();
        break;
    case SettingsPage::Network:
        _ui->stack->setCurrentWidget(_networkPage);
        break;
    case SettingsPage::Settings:
        _ui->stack->setCurrentWidget(_generalPage);
        break;
    case SettingsPage::About:
        _ui->stack->setCurrentWidget(_aboutPage);
        break;
    case SettingsPage::Account:
        // handled by set account
        [[fallthrough]];
    case SettingsPage::None:
        Q_UNREACHABLE();
    }
    Q_EMIT currentAccountChanged();
    Q_EMIT currentPageChanged();
}

void SettingsDialog::openAbout()
{
    setCurrentPage(SettingsPage::About);
}

void SettingsDialog::openHelp()
{
    // suporte da DBPixel no WhatsApp: (13) 4042-3797
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://wa.me/551340423797")));
}

SettingsDialog::SettingsPage SettingsDialog::currentPage() const
{
    return _currentPage;
}


void SettingsDialog::setCurrentAccount(Account *account)
{
    _currentAccount = account;
    _ui->stack->setCurrentWidget(accountSettings(account));
    _currentPage = SettingsPage::Account;

    Q_EMIT currentAccountChanged();
    Q_EMIT currentPageChanged();
}

Account *SettingsDialog::currentAccount() const
{
    return _currentAccount;
}

void SettingsDialog::addAccount()
{
    ocApp()->gui()->runNewAccountWizard();
}

} // namespace OCC

#include "settingsdialog.moc"
