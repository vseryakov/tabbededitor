#ifndef TABBEDEDITOR_H
#define TABBEDEDITOR_H

#include <extensionsystem/iplugin.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <QtGui>
#include <QFrame>
#include <QTabWidget>
#include <QShortcut>

namespace TabbedEditor {
namespace Internal {

class TabbedEditorWidget : public QWidget
{
    Q_OBJECT

public:
    TabbedEditorWidget(QWidget * parent = 0);
    ~TabbedEditorWidget();
    void shutdown();
    QTabWidget *getTabWidget();
    Core::IEditor *getEditor(int index);
    QWidget *getTab(Core::IEditor *editor);

private:
    bool restored;
    QTabWidget *tabWidget;

private slots:
    void updateCurrentTab(Core::IEditor* editor);
    void handleEditorOpened(Core::IEditor* editor);
    void handlerEditorClosed(QList<Core::IEditor*> editors);
    void handleTabBarClicked(int index);
    void handleTabCloseRequested(int index);
    void handleDocumentChanged();
    void handleTabMoved();
};

class TabbedEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "TabbedEditor.json")

private:
    TabbedEditorWidget *tabbedWidget;
    QFrame *backgroundFrame;

public:
    TabbedEditorPlugin();
    ~TabbedEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    bool delayedInitialize();
    ShutdownFlag aboutToShutdown();

private slots:
    void toggleTabbar();
};

} // namespace Internal
} // namespace TabbedEditor

#endif // TABBEDEDITOR_H
