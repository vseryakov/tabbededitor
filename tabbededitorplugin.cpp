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

namespace TabbedEditor {
namespace Internal {

using namespace Core::Internal;
using namespace TabbedEditor::Internal;

TabbedEditorPlugin::TabbedEditorPlugin()
{
    styleUpdatedToBaseColor = false;
}

TabbedEditorPlugin::~TabbedEditorPlugin()
{
    tabbedWidget->deleteLater();
}

bool TabbedEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{   
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)
    em = Core::EditorManager::instance();

    connect(em, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(updateStyleToBaseColor()));

    backgroundFrame = new QFrame();
    backgroundFrame->setMinimumHeight(25);
    backgroundFrame->setMaximumHeight(25);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    tabbedWidget = new TabbedEditorWidget(backgroundFrame);
    layout->addWidget(tabbedWidget->getTabWidget());
    backgroundFrame->setLayout(layout);

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
    backgroundFrame->setHidden(true);
    addAutoReleasedObject(tabbedWidget);

    return true;
}

void TabbedEditorPlugin::updateStyleToBaseColor()
{
    QColor baseColor = Utils::StyleHelper::baseColor();
    baseColor = baseColor.lighter(130);
    QString baseColorQSS = getQssStringFromColor(baseColor);

    QColor borderColor = Utils::StyleHelper::borderColor();
    QString borderColorQSS = getQssStringFromColor(borderColor);

    QColor highlightColor = Utils::StyleHelper::highlightColor();
    QString highlightColorQSS = getQssStringFromColor(highlightColor);

    QColor selectedTabBorderColor = highlightColor.lighter();
    QString selectedTabBorderColorQSS = getQssStringFromColor(selectedTabBorderColor);

    QColor shadowColor = Utils::StyleHelper::shadowColor();
    QString shadowColorQSS = getQssStringFromColor(shadowColor);

    if (styleUpdatedToBaseColor) {
        disconnect(em, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(updateStyleToBaseColor()));
        return;
    }
    QString stylesheetPattern = getStylesheetPatternFromFile(QString::fromUtf8(":/styles/default.qss"));

    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%FRAME_BACKGROUND_COLOR%"), highlightColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_SELECTED_BORDER_COLOR%"), selectedTabBorderColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_SELECTED_BACKGROUND_COLOR%"), baseColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_SELECTED_BOTTOM_BORDER_COLOR%"), baseColorQSS);

    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_BACKGROUND_COLOR_FROM%"), shadowColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_BACKGROUND_COLOR_TO%"), shadowColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_BORDER_COLOR%"), borderColorQSS);
    stylesheetPattern = stylesheetPattern.replace(QString::fromUtf8("%TAB_BOTTOM_BORDER_COLOR%"), borderColorQSS);

    backgroundFrame->setStyleSheet(stylesheetPattern);
    backgroundFrame->setHidden(false);
    styleUpdatedToBaseColor = true;
}
QString TabbedEditorPlugin::getQssStringFromColor(const QColor &color)
{
    QString QssString = QString::fromUtf8("rgba( " ) +
            QString::number(color.red()) +
            QString::fromUtf8(", ") +
            QString::number(color.green()) +
            QString::fromUtf8(", ") +
            QString::number(color.blue()) +
            QString::fromUtf8(", ") +
            QString::number(color.alpha()) +
            QString::fromUtf8(" )");
    return QssString;
}

void TabbedEditorPlugin::extensionsInitialized()
{

}

ExtensionSystem::IPlugin::ShutdownFlag TabbedEditorPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

QString TabbedEditorPlugin::getStylesheetPatternFromFile(const QString &filepath)
{
    QFile stylesheetFile(filepath);
    QString stylesheetContent;
    if (stylesheetFile.exists()) {
        if (stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            stylesheetContent = QString::fromUtf8(stylesheetFile.readAll());
            stylesheetFile.close();
        }
    }
    return stylesheetContent;
}

TabbedEditorWidget::TabbedEditorWidget(QWidget *parent):QWidget(parent)
{
    tabWidget = new QTabWidget(this);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
    tabWidget->setSizePolicy(sizePolicy);
    tabWidget->setUsesScrollButtons(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);

    Core::EditorManager *em = Core::EditorManager::instance();

    QList<Core::IEditor*> editors = em->visibleEditors();
    QListIterator<Core::IEditor*> editorsItr(editors);
    while (editorsItr.hasNext()) {
        QWidget *tab = new QWidget();
        Core::IEditor *editor = editorsItr.next();
        tabWidget->addTab(tab, editor->document()->displayName());
        tabsEditors.insert(tab, editor);
    }

    connect(em, SIGNAL(editorOpened(Core::IEditor*)), this, SLOT(handleEditorOpened(Core::IEditor*)));
    connect(em, SIGNAL(currentEditorChanged(Core::IEditor*)), this, SLOT(updateCurrentTab(Core::IEditor*)));
    connect(em, SIGNAL(editorsClosed(QList<Core::IEditor*>)), this, SLOT(handlerEditorClosed(QList<Core::IEditor*>)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int)));
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(handleTabCloseRequested(int)));
}

TabbedEditorWidget::~TabbedEditorWidget()
{
    tabWidget->deleteLater();
}

void TabbedEditorWidget::updateCurrentTab(Core::IEditor *editor)
{
    disconnect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int))); //prevent update
    if (!editor) return;
    if (!tabWidget) return;
    QWidget *currentTab = this->getTab(editor);
    if (!currentTab) return;
    tabWidget->setCurrentWidget(currentTab);
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleCurrentChanged(int))); //restore update
}

void TabbedEditorWidget::handleCurrentChanged(int index)
{
    if (index != -1) {
        QWidget *tab = tabWidget->widget(index);
        if (!tab) return;
        if (tabsEditors.contains(tab)) {
            Core::IEditor *editor = this->getEditor(tab);
            if(!editor) return;
            Core::EditorManager::instance()->activateEditor(editor);
        }
    }
}

void TabbedEditorWidget::handleEditorOpened(Core::IEditor *editor)
{
    QWidget *tab = new QWidget();

    tabWidget->addTab(tab, editor->document()->displayName());
    tabsEditors.insert(tab, editor);
    connect(editor->document(), SIGNAL(changed()), this, SLOT(updateTabText()));
}

void TabbedEditorWidget::handlerEditorClosed(QList<Core::IEditor *> editors)
{
    QListIterator<Core::IEditor*> editorsItr(editors);

    while (editorsItr.hasNext()) {
        Core::IEditor *editor = editorsItr.next();
        if (!editor) return;
        QWidget *tab = getTab(editor);
        if (!tab) return;
        if (!tabWidget) return;
        if (tabsEditors.contains(tab)) tabsEditors.remove(tab);
        if (-1 < tabWidget->indexOf(tab)) tabWidget->removeTab( tabWidget->indexOf(tab) );
    }
}

void TabbedEditorWidget::handleTabCloseRequested(int index)
{
    if (-1 < index) {
        QWidget *tab = tabWidget->widget(index);
        if (!tab) return;
        QList<Core::IEditor*> editorsToClose;
        editorsToClose.clear();
        if (tabsEditors.contains(tab)) {
            Core::IEditor *editor;
            editor = this->getEditor(tab);
            if (!editor) return;
            editorsToClose.append(this->getEditor(tab));
            Core::EditorManager::instance()->closeEditors(editorsToClose);
            if (tabsEditors.contains(tab)) tabsEditors.remove(tab);
            if (-1 < tabWidget->indexOf(tab)) tabWidget->removeTab(tabWidget->indexOf(tab));
        }
    }
}

void TabbedEditorWidget::selectTabAction()
{
    if (QShortcut *shortCut = qobject_cast<QShortcut*>(sender())) {
        int index = tabShortcuts.indexOf(shortCut);
        tabWidget->setCurrentIndex(index);
    }
}

void TabbedEditorWidget::updateTabText()
{
    Core::IDocument *document = qobject_cast<Core::IDocument *>(QObject::sender());
    QString tabTitle = document->displayName();
    if (document->isModified()) {
        tabTitle = tabTitle + QString::fromUtf8("*");
    }

    QList<Core::IEditor*> editors = Core::DocumentModel::editorsForDocument(document);
    for (int i = 0 ; i < editors.count() ; i++) {
        QWidget *tabToUpdate = this->getTab(editors.at(i));
        int tabToUpdateIndex = tabWidget->indexOf( tabToUpdate );
        tabWidget->setTabText(tabToUpdateIndex , tabTitle );
    }
}

QWidget *TabbedEditorWidget::getTabWidget()
{
    return this->tabWidget;
}

Core::IEditor *TabbedEditorWidget::getEditor(QWidget *tab)
{
    return tabsEditors.value(tab);
}

QWidget *TabbedEditorWidget::getTab(Core::IEditor *editor)
{
    return tabsEditors.key(editor);
}

bool TabbedEditorWidget::isEditorWdiget(QObject *obj)
{
    if (!obj) return false;
    QMapIterator<QWidget*, Core::IEditor*> i(tabsEditors);
    while (i.hasNext()) {
        i.next();
        if (obj == i.value()->widget()) return true;
    }
    return false;
}

}//Internal
}//TabbedEditor
