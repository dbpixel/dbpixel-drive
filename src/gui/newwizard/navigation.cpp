#include "navigation.h"

#include <QHBoxLayout>
#include <QRadioButton>

namespace OCC::Wizard {

Navigation::Navigation(QWidget *parent)
    : QWidget(parent)
{
    // this class manages its own layout
    setLayout(new QHBoxLayout(this));
}

void Navigation::setEntries(const QList<SetupWizardState> &newEntries)
{
    // TODO: more advanced implementation (reuse existing buttons within layout)
    // current active page is also lost that way
    removeAllItems();

    int number = 0;
    for (const auto state : newEntries) {
        // Drive: etapas numeradas com os nomes do protótipo (1 Boas-vindas, 2 Entrar, 3 Sincronização)
        QString label = Utility::enumToDisplayName(state);
        switch (state) {
        case SetupWizardState::ServerUrlState:
            label = QStringLiteral("Boas-vindas");
            break;
        case SetupWizardState::CredentialsState:
            label = QStringLiteral("Entrar");
            break;
        case SetupWizardState::AccountConfiguredState:
            label = QStringLiteral("Sincronização");
            break;
        default:
            break;
        }
        const QString text = QStringLiteral("%1  %2").arg(++number).arg(label);

        auto newButton = new QRadioButton(text, this);

        _entries.insert(state, newButton);
        layout()->addWidget(newButton);

        connect(newButton, &QRadioButton::clicked, this, [this, state]() {
            // clicks to the current state button should be ignored
            // this used to be handled by disabling the button
            if (state != _activeState) {
                Q_EMIT paginationEntryClicked(state);
            }
        });
    }

    enableOrDisableButtons();
}

// needed to clean up widgets we added to the layout
Navigation::~Navigation() noexcept
{
    removeAllItems();
}

void Navigation::removeAllItems()
{
    qDeleteAll(_entries);
}

void Navigation::enableOrDisableButtons()
{
    for (const auto state : _entries.keys()) {
        auto button = _entries.value(state);

        const auto enabled = [&state, this]() {
            if (_enabled) {
                return state <= _activeState;
            }

            return false;
        }();

        // we just ignore clicks to the current page
        button->setEnabled(enabled);
    }
}

void Navigation::setActiveState(SetupWizardState newState)
{
    _activeState = newState;

    for (const auto key : _entries.keys()) {
        auto button = _entries.value(key);
        button->setChecked(key == _activeState);
    }

    enableOrDisableButtons();
}

}
