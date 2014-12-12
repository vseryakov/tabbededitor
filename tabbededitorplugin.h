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

    QWidget *getTabWidget();
    Core::IEditor *getEditor(QWidget *tab);
    QWidget *getTab(Core::IEditor *editor);

    bool isEditorWdiget(QObject *obj);

private:
    QTabWidget *tabWidget;
    QMap<QWidget*, Core::IEditor*> tabsEditors;
    QList<QShortcut*> tabShortcuts;

private slots:
    void updateCurrentTab(Core::IEditor* editor);
    void handleCurrentChanged(int index);
    void handleEditorOpened(Core::IEditor* editor);
    void handlerEditorClosed(QList<Core::IEditor*> editors);
    void handleTabCloseRequested(int index);

    void selectTabAction();
    void updateTabText();
};

class TabbedEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "TabbedEditor.json")

private:
    TabbedEditorWidget *tabbedWidget;
    QFrame *backgroundFrame;
    bool styleUpdatedToBaseColor;
    Core::EditorManager *em;
    QString getQssStringFromColor(const QColor &color);

public:
    TabbedEditorPlugin();
    ~TabbedEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();
private slots:
    QString getStylesheetPatternFromFile(const QString &filepath);
    void updateStyleToBaseColor();
};

} // namespace Internal
} // namespace TabbedEditor

#endif // TABBEDEDITOR_H
