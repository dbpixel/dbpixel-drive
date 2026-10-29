#include "serverurlsetupwizardpage.h"
#include "ui_serverurlsetupwizardpage.h"

#include "gui/qmlutils.h"
#include "theme.h"

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QVBoxLayout>


namespace OCC::Wizard {

ServerUrlSetupWizardPage::ServerUrlSetupWizardPage(const QUrl &serverUrl)
    : _ui(new ::Ui::ServerUrlSetupWizardPage)
{
    _ui->setupUi(this);

    _ui->welcomeTextLabel->setText(tr("Welcome to %1").arg(Theme::instance()->appNameGUI()));

    // not the best style, but we hacked such branding into the pages elsewhere, too
    if (!Theme::instance()->overrideServerUrlV2().isEmpty()) {
        // note that the text should be set before the page is displayed, this way validateInput() will enable the next button
        _ui->urlLineEdit->setText(Theme::instance()->overrideServerUrlV2());

        _ui->urlLineEdit->hide();
        _ui->serverUrlLabel->hide();

        // Drive (protótipo W1): o título fica na coluna da esquerda; aqui entram os três cartões e o servidor
        _ui->logoLabel->hide();
        _ui->welcomeTextLabel->hide();
        for (int i = _ui->verticalLayout->count() - 1; i >= 0; --i) {
            if (_ui->verticalLayout->itemAt(i)->spacerItem()) {
                delete _ui->verticalLayout->takeAt(i);
            }
        }
        auto *welcome = new QmlUtils::OCQuickWidget(this);
        welcome->setResizeMode(QQuickWidget::SizeRootObjectToView);
        welcome->setOCContext(QUrl(QStringLiteral("qrc:/qt/qml/org/ownCloud/gui/qml/drive/WizardWelcome.qml")), this);
        _ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
        _ui->verticalLayout->insertWidget(0, welcome, 1);
    } else {
        _ui->urlLineEdit->setText(serverUrl.toString());

        connect(this, &AbstractSetupWizardPage::pageDisplayed, this, [this]() {
            _ui->urlLineEdit->setFocus();
        });
    }

    _ui->logoLabel->setText(QString());
    _ui->logoLabel->setPixmap(Theme::instance()->wizardHeaderLogo().pixmap(200, 200));
    //: This is the accessibility text for the logo in the setup wizard page. The parameter is the name for the (branded) application.
    _ui->logoLabel->setAccessibleName(tr("%1 logo").arg(Theme::instance()->appNameGUI()));

    if (!Theme::instance()->wizardUrlPostfix().isEmpty()) {
        _ui->urlLineEdit->setPostfix(Theme::instance()->wizardUrlPostfix());
    }

    connect(_ui->urlLineEdit, &QLineEdit::textChanged, this, &AbstractSetupWizardPage::contentChanged);
}

QString ServerUrlSetupWizardPage::serverHost() const
{
    const QUrl url(userProvidedUrl());
    return url.host().isEmpty() ? userProvidedUrl() : url.host();
}

QString ServerUrlSetupWizardPage::userProvidedUrl() const
{
    QString url = _ui->urlLineEdit->text().simplified();

    if (!Theme::instance()->wizardUrlPostfix().isEmpty()) {
        url += Theme::instance()->wizardUrlPostfix();
    }

    return url;
}

ServerUrlSetupWizardPage::~ServerUrlSetupWizardPage()
{
    delete _ui;
}

bool ServerUrlSetupWizardPage::validateInput()
{
    return !_ui->urlLineEdit->text().isEmpty();
}
}
