#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <global.h>
#include <QFrame>

class ControlScene;
class FormScene;
class QmlEditorView;
class ControlView;
class FormView;
class LoadingIndicator;
class QSplitter;
class QHBoxLayout;
class QVBoxLayout;
class QToolBar;
class QToolButton;
class OutputWidget;
class QComboBox;
class ControlScene;
class ControlScene;

class DesignerWidget : public QFrame
{
        Q_OBJECT

    public:
        explicit DesignerWidget(QWidget* parent = nullptr);
        const DesignMode& mode() const;
        void setMode(const DesignMode& mode);
        ControlScene* currentScene();
        ControlScene* controlScene();
        FormScene* formScene();
        QmlEditorView* qmlEditorView();
        ControlView* controlView();
        FormView* formView();
        LoadingIndicator* loadingIndicator();
        QSplitter* splitter();
        OutputWidget* outputWidget();

    public slots:
        void updateSkin();
        void handleControlClicked(Control*);
        void handleControlDoubleClick(Control*);
        void handleControlDrop(Control*, const QPointF&, const QString&);

    private slots:
        void checkErrors();
        void handleModeChange();
        void handleIndicatorChanges();
        void handleSnappingClick(bool value);
        void handleShowOutlineClick(bool value);
        void handleFitInSceneClick();
        void handleThemeChange(const QString& text);
        void handleZoomLevelChange(const QString& text);
        void handlePhonePortraitButtonClick();
        void handlePhoneLandscapeButtonClick();
        void handleDesktopSkinButtonClick();
        void handleNoSkinButtonClick();
        void handleRefreshPreviewClick();
        void handleClearControls();
        void handleEditorModeButtonClick();
        void handleCGuiModeButtonClick();
        void handleWGuiModeButtonClick();
        void handlePlayButtonClick();
        void handleBuildButtonClick();

    signals:
        void modeChanged();

    private:
        DesignMode _mode;
        ControlScene* _currentScene;
        QHBoxLayout* _hlayout;
        QToolBar* _toolbar2;
        QToolButton* _editorModeButton;
        QToolButton* _wGuiModeButton;
        QToolButton* _cGuiModeButton;
        QToolButton* _playButton;
        QToolButton* _buildButton;
        QVBoxLayout* _vlayout;
        QSplitter* _splitter;
        FormScene* _formScene;
        ControlScene* _controlScene;
        FormView* _formView;
        ControlView* _controlView;
        QmlEditorView* _qmlEditorView;
        OutputWidget* _outputWidget;
        qreal* _lastScaleOfWv;
        qreal* _lastScaleOfCv;
        QToolBar* _toolbar;
        QToolButton* _refreshPreviewButton;
        QToolButton* _clearFormButton;
        QToolButton* _undoButton;
        QToolButton* _redoButton;
        QToolButton* _phonePortraitButton;
        QToolButton* _phoneLandscapeButton;
        QToolButton* _desktopSkinButton;
        QToolButton* _noSkinButton;
        QToolButton* _snappingButton;
        QToolButton* _showOutlineButton;
        QToolButton* _fitInSceneButton;
        QComboBox* _zoomlLevelCombobox;
        QComboBox* _themeCombobox;
        LoadingIndicator* _loadingIndicator;
        QToolButton* _layItVertButton;
        QToolButton* _layItHorzButton;
        QToolButton* _layItGridButton;
        QToolButton* _breakLayoutButton;
        QTimer* _errorChecker;
};

#endif // DESIGNERWIDGET_H
