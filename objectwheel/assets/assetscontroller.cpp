#include <assetscontroller.h>
#include <assetspane.h>
#include <assetstree.h>
#include <QComboBox>

AssetsController::AssetsController(AssetsPane* assetsPane, QObject* parent) : QObject(parent)
  , m_assetsPane(assetsPane)
{
    connect(m_assetsPane->modeComboBox(), qOverload<int>(&QComboBox::activated),
            this, &AssetsController::onModeComboBoxActivation);
}

void AssetsController::discharge()
{

}

void AssetsController::onModeComboBoxActivation()
{
    m_assetsPane->assetsTree()->setMode(m_assetsPane->modeComboBox()->currentData().value<AssetsTree::Mode>());
}


