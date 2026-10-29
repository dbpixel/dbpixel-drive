#include "setupwizardwidget.h"
#include "ui_setupwizardwidget.h"

#include "gui/application.h"
#include "gui/brandfonts.h"
#include "gui/qmlutils.h"
#include "gui/guiutility.h"
#include "gui/owncloudgui.h"
#include "gui/settingsdialog.h"
#include "resources/template.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QMessageBox>
#include <QStyleFactory>

using namespace std::chrono_literals;

namespace {

using namespace OCC;

QString replaceCssColors()
{
    return Resources::Template::renderTemplateFromFile(QStringLiteral(":/client/resources/wizard/style.qss"),
        {
            {QStringLiteral("WIZARD_BACKGROUND_COLOR"), Theme::instance()->wizardHeaderBackgroundColor().name()}, //
            {QStringLiteral("WIZARD_FONT_COLOR"), Theme::instance()->wizardHeaderTitleColor().name()} //
        });
}

}

namespace OCC::Wizard {

Q_LOGGING_CATEGORY(lcSetupWizardWidget, "gui.setupwizard.window")

SetupWizardWidget::SetupWizardWidget(SettingsDialog *parent)
    : QWidget(parent)
    , _ui(new ::Ui::SetupWizardWidget)
{
    setWindowFlag(Qt::WindowCloseButtonHint, false);

    _ui->setupUi(this);
    _ui->backButton->setText(Utility::isMac() ? tr("Back") : tr("< &Back"));
    _ui->backButton->setAccessibleName(tr("Back"));

    slotHideErrorMessageWidget();

    connect(_ui->cancelButton, &QPushButton::clicked, this, [this] {
        auto messageBox = new QMessageBox(QMessageBox::Warning, tr("Cancel Setup"), tr("Do you really want to cancel the account setup?"),
            QMessageBox::Yes | QMessageBox::No, ocApp()->gui()->settingsDialog());
        messageBox->setAttribute(Qt::WA_DeleteOnClose);
        // Drive: botões em português mesmo se a tradução do Qt não carregar
        messageBox->button(QMessageBox::Yes)->setText(QStringLiteral("Cancelar a configuração"));
        messageBox->button(QMessageBox::No)->setText(QStringLiteral("Continuar configurando"));
        connect(messageBox, &QMessageBox::accepted, this, [this] {
            // call the base implementation
            Q_EMIT rejected();
        });
        ownCloudGui::raise();
        messageBox->open();
    });

    connect(_ui->nextButton, &QPushButton::clicked, this, &SetupWizardWidget::slotMoveToNextPage);

    connect(_ui->backButton, &QPushButton::clicked, this, [this]() {
        slotStartTransition();
        Q_EMIT backButtonClicked();
    });

    connect(_ui->navigation, &Navigation::paginationEntryClicked, this, [this](SetupWizardState clickedState) {
        slotStartTransition();
        Q_EMIT navigationEntryClicked(clickedState);
    });

    // different styles (e.g., 'Windows', 'Fusion') may require different approaches in the stylesheet
    // therefore we want to force a standard style on all platforms
    // this further makes sure the wizard (well, its contents) looks exactly the same on all platforms
    // Fusion should be available everywhere
    auto fusionStyle = QStyleFactory::create(QStringLiteral("Fusion"));
    if (OC_ENSURE(fusionStyle != nullptr)) {
        _ui->contentWidget->setStyle(fusionStyle);
    } else {
        qCDebug(lcSetupWizardWidget) << "Could not set up default style, wizard contents will be shown using default style";
    }

    loadStylesheet();
    setupDriveLayout();

    _ui->transitionProgressIndicator->setFixedSize(32, 32);
    _ui->transitionProgressIndicator->setColor(Theme::instance()->wizardHeaderTitleColor());
}

// Drive (um produto DBPixel): faixa navy com a marca e as etapas, coluna de título à esquerda,
// conteúdo branco e rodapé com o endosso. Cores do produto (manual 3.9): ação #47689A com texto branco.
void SetupWizardWidget::setupDriveLayout()
{
    BrandFonts::load();
    auto *mainLayout = qobject_cast<QVBoxLayout *>(layout());
    if (!OC_ENSURE(mainLayout)) {
        return;
    }
    mainLayout->setSpacing(0);

    // faixa de cima em QML: marca, etapas numeradas (check nas concluídas) e canto chanfrado
    auto *header = new QmlUtils::OCQuickWidget(this);
    header->setFixedHeight(72);
    header->setResizeMode(QQuickWidget::SizeRootObjectToView);
    header->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/drive/WizardHeader.qml")), this);
    // a navegação original continua existindo para o controlador, só não aparece
    _ui->navigation->hide();

    // corpo: coluna de título + páginas
    const int contentIndex = mainLayout->indexOf(_ui->contentWidget);
    mainLayout->removeWidget(_ui->contentWidget);
    auto *body = new QWidget(this);
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    auto *hero = new QWidget(body);
    hero->setObjectName(QStringLiteral("driveHero"));
    hero->setAttribute(Qt::WA_StyledBackground);
    hero->setFixedWidth(260);
    auto *heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(24, 32, 24, 24);
    heroLayout->setSpacing(12);
    _heroEyebrow = new QLabel(hero);
    _heroEyebrow->setObjectName(QStringLiteral("driveHeroEyebrow"));
    QFont eyebrowFont(QStringLiteral("Montserrat"), 8, QFont::Bold);
    eyebrowFont.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    _heroEyebrow->setFont(eyebrowFont);
    _heroEyebrow->setWordWrap(true);
    _heroTitle = new QLabel(hero);
    _heroTitle->setObjectName(QStringLiteral("driveHeroTitle"));
    _heroTitle->setWordWrap(true);
    _heroTitle->setTextFormat(Qt::RichText);
    _heroText = new QLabel(hero);
    _heroText->setObjectName(QStringLiteral("driveHeroText"));
    _heroText->setWordWrap(true);
    heroLayout->addWidget(_heroEyebrow);
    heroLayout->addWidget(_heroTitle);
    heroLayout->addWidget(_heroText);
    heroLayout->addStretch();
    bodyLayout->addWidget(hero);
    bodyLayout->addWidget(_ui->contentWidget, 1);
    mainLayout->insertWidget(contentIndex, body, 1);
    mainLayout->insertWidget(0, header);

    // rodapé: endosso à esquerda, botões à direita (Cancelar junto de Voltar/Próximo)
    auto *endorsement = new QLabel(QStringLiteral("UM PRODUTO <b>DBPIXEL</b>"), this);
    endorsement->setObjectName(QStringLiteral("driveEndorsement"));
    QFont endoFont(QStringLiteral("Montserrat"), 7, QFont::DemiBold);
    endoFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    endorsement->setFont(endoFont);
    _ui->bottomButtonsLayout->removeWidget(_ui->cancelButton);
    _ui->bottomButtonsLayout->insertWidget(0, endorsement);
    _ui->bottomButtonsLayout->insertWidget(_ui->bottomButtonsLayout->indexOf(_ui->backButton), _ui->cancelButton);
    _ui->bottomButtonsLayout->setSpacing(10);

    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    setStyleSheet(QStringLiteral(
        "#driveHero { background: #F7F8FA; border-right: 1px solid #ECEDF0; }"
        "#driveHeroEyebrow { color: #43638F; }"
        "#driveHeroTitle { color: #0A1222; font-family: Montserrat; font-weight: 600; font-size: 22px; }"
        "#driveHeroText { color: #5E6573; font-family: 'Nunito Sans'; font-size: 14px; }"
        "#driveEndorsement { color: #5E6573; }"
        "#nextButton { background: #47689A; color: #FFFFFF; border: 0; border-radius: 10px; min-height: 40px; padding: 0 20px; font-family: Montserrat; font-weight: 600; font-size: 14px; }"
        "#nextButton:hover { background: #3F5F8F; }"
        "#nextButton:disabled { background: #C9D4E5; color: #5E6B80; }"
        "#backButton, #cancelButton { background: #F4F5F7; color: #0A1222; border: 1px solid #D9DCE2; border-radius: 10px; min-height: 40px; padding: 0 18px; font-family: Montserrat; font-weight: 600; font-size: 14px; }"
        "#backButton:hover, #cancelButton:hover { background: #E9EBEF; }"
        "#backButton:disabled { color: #9AA0AC; }"));
}

QStringList SetupWizardWidget::stepNames() const
{
    return {QStringLiteral("Boas-vindas"), QStringLiteral("Entrar"), QStringLiteral("Sincronização")};
}

void SetupWizardWidget::updateDriveHero(SetupWizardState state)
{
    if (!_heroTitle) {
        return;
    }
    const int step = state == SetupWizardState::AccountConfiguredState ? 2 : (state == SetupWizardState::ServerUrlState ? 0 : 1);
    if (step != _stepIndex) {
        _stepIndex = step;
        Q_EMIT driveStepChanged();
    }
    // botões sem as setas "< >" do assistente original, como no protótipo
    _ui->backButton->setText(QStringLiteral("Voltar"));
    _ui->nextButton->setText(state == SetupWizardState::FinalState ? QStringLiteral("Concluir") : QStringLiteral("Continuar"));
    const auto em =[](const QString &t) { return QStringLiteral("<span style=\"color:#43638F\">%1</span>").arg(t); };
    switch (state) {
    case SetupWizardState::CredentialsState:
        _heroEyebrow->setText(QStringLiteral("ENTRAR"));
        _heroTitle->setText(QStringLiteral("Continue no %1.").arg(em(QStringLiteral("navegador"))));
        _heroText->setText(QStringLiteral("Abrimos a página de entrada da DBPixel. Entre com o seu usuário e senha e volte para cá. "
                                          "Sua senha nunca fica salva neste programa."));
        break;
    case SetupWizardState::AccountConfiguredState:
        _heroEyebrow->setText(QStringLiteral("SINCRONIZAÇÃO"));
        _heroTitle->setText(QStringLiteral("Onde ficam os %1?").arg(em(QStringLiteral("arquivos"))));
        _heroText->setText(QStringLiteral("Escolha a pasta neste computador e o que sincronizar. Dá para mudar depois nas configurações."));
        break;
    default:
        _heroEyebrow->setText(QStringLiteral("ARMAZENAMENTO EM NUVEM"));
        _heroTitle->setText(QStringLiteral("Seus arquivos da DBPixel, %1.").arg(em(QStringLiteral("no seu computador"))));
        _heroText->setText(QStringLiteral("O Drive mantém uma pasta no seu PC igual à sua nuvem. O que você salvar nela vai para a nuvem sozinho."));
        break;
    }
}

void SetupWizardWidget::loadStylesheet()
{
    _ui->contentWidget->setStyleSheet(replaceCssColors());
}

void SetupWizardWidget::displayPage(AbstractSetupWizardPage *page, SetupWizardState state)
{
    _transitioning = false;
    _ui->backButton->setEnabled(true);
    _ui->nextButton->setEnabled(true);

    if (state == SetupWizardState::FirstState) {
        _ui->backButton->setEnabled(false);
    } else if (state == SetupWizardState::FinalState) {
        _ui->nextButton->setEnabled(false);
    }

    if (state == SetupWizardState::FinalState) {
        _ui->nextButton->setText(Utility::isMac() ? tr("Done") : tr("&Finish"));
        _ui->nextButton->setAccessibleName(Utility::isMac() ? tr("Done") : tr("Finish"));
    } else {
        _ui->nextButton->setText(Utility::isMac() ? tr("Continue") : tr("&Next >"));
        _ui->nextButton->setAccessibleName(Utility::isMac() ? tr("Continue") : tr("Next"));
    }

    _currentPage = page;
    slotReplaceContent(_currentPage);

    // initial check whether to enable the next button right away
    slotUpdateNextButton();

    _ui->navigation->setActiveState(state);
    updateDriveHero(state);
    _ui->navigation->setEnabled(true);

    connect(_ui->errorMessageDismissButton, &QPushButton::clicked, this, &SetupWizardWidget::slotHideErrorMessageWidget);

    // by default, set focus on the next button
    _ui->nextButton->setFocus();

    // bring to front if necessary
    ownCloudGui::raise();

    connect(_currentPage, &AbstractSetupWizardPage::contentChanged, this, &SetupWizardWidget::slotUpdateNextButton);

    QTimer::singleShot(0, this, [this]() {
        // this can optionally be overwritten by the page
        Q_EMIT _currentPage->pageDisplayed();
    });
}

void SetupWizardWidget::slotStartTransition()
{
    _transitioning = true;

    _ui->transitionProgressIndicator->startAnimation();
    _ui->contentWidget->setCurrentWidget(_ui->transitionPage);

    // until a new page is displayed by the controller, we want to prevent the user from initiating another page change
    _ui->backButton->setEnabled(false);
    _ui->nextButton->setEnabled(false);
    _ui->navigation->setEnabled(false);
    // also, we should assume the user has seen the error message in case one is shown
    slotHideErrorMessageWidget();
}

void SetupWizardWidget::slotReplaceContent(QWidget *newWidget)
{
    _ui->contentWidget->removeWidget(_currentContentWidget);

    _ui->contentWidget->addWidget(newWidget);
    _ui->contentWidget->setCurrentWidget(newWidget);

    _currentContentWidget = newWidget;

    // inheriting the style sheet from content widget doesn't work in all cases
    _currentContentWidget->setStyleSheet(_ui->contentWidget->styleSheet());
}

void SetupWizardWidget::slotHideErrorMessageWidget()
{
    _ui->errorMessageWidget->hide();
}

void SetupWizardWidget::showErrorMessage(const QString &errorMessage)
{
    _ui->errorMessageLabel->setText(QStringLiteral("<b>%1</b>").arg(errorMessage));
    _ui->errorMessageLabel->setWordWrap(true);
    _ui->errorMessageWidget->show();
}

void SetupWizardWidget::setNavigationEntries(const QList<SetupWizardState> &entries)
{
    _ui->navigation->setEntries(entries);
}

void SetupWizardWidget::slotUpdateNextButton()
{
    _ui->nextButton->setEnabled(_currentPage->validateInput());
}

void SetupWizardWidget::disableNextButton()
{
    _ui->nextButton->setEnabled(false);
}

SetupWizardWidget::~SetupWizardWidget() noexcept
{
    delete _ui;
}

void SetupWizardWidget::slotMoveToNextPage()
{
    if (OC_ENSURE(_currentPage->validateInput())) {
        slotStartTransition();
        Q_EMIT nextButtonClicked();
    }
}
}
