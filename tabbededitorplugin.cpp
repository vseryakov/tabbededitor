#include "tabbededitorplugin.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <utils/stylehelper.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/editorview.h>
#include <coreplugin/editormanager/documentmodel.h>
#include <coreplugin/idocument.h>
#include <texteditor/texteditor.h>
#include <utils/stylehelper.h>

#include <QList>
#include <QMap>
#include <QListIterator>
#include <QAction>
#include <QLabel>
#include <QPointer>
#include <QObject>
#include <QMetaObject>
#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#include <QByteArray>
#include <QBoxLayout>
#include <QFrame>
#include <QMainWindow>
#include <QTabBar>

namespace TabbedEditor {
namespace Internal {

using namespace Core::Internal;
using namespace TabbedEditor::Internal;

TabbedEditorPlugin::TabbedEditorPlugin()
{
}

TabbedEditorPlugin::~TabbedEditorPlugin()
{
    tabbedWidget->deleteLater();
}

bool TabbedEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    backgroundFrame = new QFrame();
    tabbedWidget = new TabbedEditorWidget(backgroundFrame);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tabbedWidget->getTabWidget());
    backgroundFrame->setLayout(layout);
    backgroundFrame->setMinimumHeight(tabbedWidget->getTabWidget()->iconSize().height() * 1.5);
    backgroundFrame->setMaximumHeight(backgroundFrame->minimumHeight());

    QWidget *coreWidget = Core::ICore::mainWindow();
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(coreWidget);
    mainWindow->layout()->setSpacing(0);

    QWidget* oldCentralWidget = mainWindow->centralWidget();
    QWidget* newCentralWidget = new  QWidget(mainWindow);
    newCentralWidget->setMinimumHeight(0);

    QVBoxLayout *newCentralWidgetLayout = new QVBoxLayout();
    newCentralWidgetLayout->setSpacing(0);
    newCentralWidgetLayout->setContentsMargins(0, 0, 0, 0);
    newCentralWidgetLayout->addWidget(backgroundFrame);
    newCentralWidgetLayout->addWidget(oldCentralWidget);
    newCentralWidget->setLayout(newCentralWidgetLayout);
    mainWindow->setCentralWidget(newCentralWidget);
    backgroundFrame->setHidden(Core::ICore::settings()->value(QLatin1String("TabbedEditor/hidden")).toBool());
    addAutoReleasedObject(tabbedWidget);

    Core::ActionContainer *menu = Core::ActionManager::createMenu("TabbedEditor.Menu");
    menu->menu()->setTitle(QLatin1String("Tabs"));

    QAction *toggle = new QAction(QLatin1String("Show/hide tabs"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(toggle, "TabbedEditor.Action", Core::Context(Core::Constants::C_GLOBAL));
    menu->addAction(cmd);
    connect(toggle, SIGNAL(triggered()), this, SLOT(toggleTabbar()));
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void TabbedEditorPlugin::toggleTabbar()
{
    backgroundFrame->setHidden(backgroundFrame->isHidden() ? false : true);
    Core::ICore::settings()->setValue(QLatin1String("TabbedEditor/hidden"), backgroundFrame->isHidden());
}

void TabbedEditorPlugin::extensionsInitialized()
{
}

bool TabbedEditorPlugin::delayedInitialize()
{
    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag TabbedEditorPlugin::aboutToShutdown()
{
    tabbedWidget->shutdown();
    return SynchronousShutdown;
}

TabbedEditorWidget::TabbedEditorWidget(QWidget *parent):QWidget(parent)
{
    tabWidget = new QTabWidget(this);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(0);
    tabWidget->setSizePolicy(sizePolicy);
    tabWidget->setDocumentMode(true);
    tabWidget->setUsesScrollButtons(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->tabBar()->setContentsMargins(0, 0, 0, 0);
    tabWidget->tabBar()->setBaseSize(0, 0);
    tabWidget->tabBar()->setDrawBase(false);

    if (!Core::ICore::settings()->group().isEmpty()) Core::ICore::settings()->endGroup();
    QStringList tabs = Core::ICore::settings()->value(QLatin1String("TabbedEditor/tabs")).toString().split(QLatin1String("|"));
    foreach(QString file, tabs) {
        if (!file.size()) continue;
        QWidget *tab = new QWidget();
        tabWidget->addTab(tab, QFileInfo(file).fileName());
        tabWidget->setTabToolTip(tabWidget->count() - 1, file);
    }

    connect(Core::EditorManager::instance(), SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(handleEditorOpened(Core::IEditor*)));
    connect(Core::EditorManager::instance(), SIGNAL(currentDocumentStateChanged()), this, SLOT(handleDocumentChanged()));
    connect(Core::EditorManager::instance(), SIGNAL(currentEditorChanged(Core::IEditor*)), this, SLOT(updateCurrentTab(Core::IEditor*)));
    connect(Core::EditorManager::instance(), SIGNAL(editorsClosed(QList<Core::IEditor*>)), this, SLOT(handlerEditorClosed(QList<Core::IEditor*>)));
    connect(tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(handleTabBarClicked(int)));
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabCloseRequested(int)));
    connect(tabWidget->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(handleTabMoved()));
}

TabbedEditorWidget::~TabbedEditorWidget()
{
    tabWidget->deleteLater();
    tabWidget = NULL;
}

void TabbedEditorWidget::shutdown()
{
    disconnect(Core::EditorManager::instance(), 0, this, 0);
    tabWidget->disconnect();
    this->disconnect();
}

void TabbedEditorWidget::handleTabMoved()
{
    if (!tabWidget) return;

    QStringList tabs;
    for (int i = 0; i < tabWidget->count(); i++) tabs << tabWidget->tabToolTip(i);
    if (!Core::ICore::settings()->group().isEmpty()) Core::ICore::settings()->endGroup();
    Core::ICore::settings()->setValue(QLatin1String("TabbedEditor/tabs"), tabs.join(QLatin1String("|")));
    Core::ICore::settings()->sync();
}

void TabbedEditorWidget::updateCurrentTab(Core::IEditor *editor)
{
    if (!tabWidget) return;
    QWidget *tab = this->getTab(editor);
    if (tab) tabWidget->setCurrentWidget(tab);
}

void TabbedEditorWidget::handleDocumentChanged()
{
    if (!tabWidget) return;
    Core::IEditor *editor = this->getEditor(tabWidget->currentIndex());
    if (!editor) return;
    QString title = editor->document()->displayName();
    if (editor->document()->isModified()) title += QString::fromUtf8("*");
    tabWidget->setTabText(tabWidget->currentIndex(), title);
}

void TabbedEditorWidget::handleTabBarClicked(int index)
{
    if (!tabWidget) return;
    Core::IEditor *editor = this->getEditor(index);
    if (editor) {
        Core::EditorManager::instance()->activateEditor(editor);
    } else {
        foreach (Core::DocumentModel::Entry *entry, Core::DocumentModel::entries()) {
            if (entry->fileName().toString() == tabWidget->tabToolTip(index)) {
                disconnect(Core::EditorManager::instance(), SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(handleEditorOpened(Core::IEditor*)));
                Core::EditorManager::openEditor(entry->fileName().toString(), entry->id());
                connect(Core::EditorManager::instance(), SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(handleEditorOpened(Core::IEditor*)));
                return;
            }
        }
    }
    Core::EditorManager::openEditor(tabWidget->tabToolTip(index));
}

void TabbedEditorWidget::handleEditorOpened(Core::IEditor *editor)
{
    if (!tabWidget) return;

    QWidget *tab = getTab(editor);
    if (tab) {
        tabWidget->setCurrentWidget(tab);
        return;
    }
    // No tab by editor, check by file path
    for (int i = 0; i < tabWidget->count(); i++) {
        if (tabWidget->tabToolTip(i) == editor->document()->filePath().toString()) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }
    // Completely new editor, create a new tab
    tab = new QWidget();
    tabWidget->addTab(tab, editor->document()->displayName());
    tabWidget->setTabToolTip(tabWidget->count() - 1, editor->document()->filePath().toString());
    handleTabMoved();
}

void TabbedEditorWidget::handlerEditorClosed(QList<Core::IEditor *> editors)
{
    if (!tabWidget) return;
    foreach (Core::IEditor *editor, editors) {
        QWidget *tab = getTab(editor);
        if (!tab) continue;
        if (tabWidget->indexOf(tab) > -1) tabWidget->removeTab(tabWidget->indexOf(tab));
        tab->deleteLater();
    }
}

void TabbedEditorWidget::handleTabCloseRequested(int index)
{
    if (!tabWidget) return;
    QList<Core::IEditor *> editors;
    Core::IEditor *editor = getEditor(index);
    if (editor) {
        editors << editor;
    } else {
        foreach (Core::DocumentModel::Entry *entry, Core::DocumentModel::entries()) {
            if (entry->fileName().toString() == tabWidget->tabToolTip(index)) {
                editor = Core::EditorManager::openEditor(entry->fileName().toString(), entry->id(), Core::EditorManager::DoNotChangeCurrentEditor|Core::EditorManager::DoNotMakeVisible);
                if (editor) editors << editor;
            }
        }
    }
    if (editors.count()) {
        Core::EditorManager::instance()->closeEditors(editors);
    } else {
        QWidget *tab = tabWidget->widget(index);
        tabWidget->removeTab(index);
        tab->deleteLater();
    }
}

QTabWidget *TabbedEditorWidget::getTabWidget()
{
    return this->tabWidget;
}

Core::IEditor *TabbedEditorWidget::getEditor(int index)
{
    if (index < 0 || index >= tabWidget->count()) return 0;
    QList<Core::IEditor*> editors = Core::DocumentModel::editorsForFilePath(tabWidget->tabToolTip(index));
    return editors.isEmpty() ? 0 : editors.first();
}

QWidget *TabbedEditorWidget::getTab(Core::IEditor *editor)
{
    if (!editor || !editor->document()) return 0;
    for (int i = 0; i < tabWidget->count(); i++) {
        if (editor->document()->filePath().toString() == tabWidget->tabToolTip(i)) return tabWidget->widget(i);
    }
    return 0;
}

} // Internal
} // TabbedEditor
