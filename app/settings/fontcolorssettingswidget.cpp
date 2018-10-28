#include <fontcolorssettingswidget.h>
#include <codeeditorsettings.h>

#include <texteditor/colorschemeedit.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QApplication>

namespace {

const char* g_colorSchemeFiles[] = {":/styles/creator-dark.xml",
                                    ":/styles/dark.xml",
                                    ":/styles/default.xml",
                                    ":/styles/default_classic.xml",
                                    ":/styles/grayscale.xml",
                                    ":/styles/inkpot.xml",
                                    ":/styles/intellij.xml",
                                    ":/styles/modnokai_night_shift_v2.xml",
                                    ":/styles/solarized-dark.xml",
                                    ":/styles/solarized-light.xml"};
const char* g_colorSchemeNames[] = {"Qt Dark",
                                    "Dark",
                                    "Default",
                                    "Default Classic",
                                    "Grayscale",
                                    "Inkpot",
                                    "IntelliJ IDEA",
                                    "Modnokai Night Shift v2",
                                    "Solarized Dark",
                                    "Solarized Light"};

void addColorSchemes(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_colorSchemeNames) / sizeof(g_colorSchemeNames[0]); ++i)
        comboBox->addItem(QObject::tr(g_colorSchemeNames[i]), g_colorSchemeNames[i]);
}

FormatDescriptions colorFormatDescriptions()
{
    // Note: default background colors are coming from FormatDescription::background()

    // Add font preference page
    FormatDescriptions formatDescr;
    formatDescr.reserve(C_LAST_STYLE_SENTINEL);
    formatDescr.emplace_back(C_TEXT, QObject::tr("Text"), QObject::tr("Generic text.\nApplied to "
                                                                      "text, if no other "
                                                                      "rules matching."));

    // Special categories
    const QPalette p = QApplication::palette();
    formatDescr.emplace_back(C_LINK, QObject::tr("Link"),
                             QObject::tr("Links that follow symbol under cursor."), Qt::blue);
    formatDescr.emplace_back(C_SELECTION, QObject::tr("Selection"), QObject::tr("Selected text."),
                             p.color(QPalette::HighlightedText));
    formatDescr.emplace_back(C_LINE_NUMBER, QObject::tr("Line Number"),
                             QObject::tr("Line numbers located on the left side of the editor."),
                             FormatDescription::AllControlsExceptUnderline);
    formatDescr.emplace_back(C_SEARCH_RESULT, QObject::tr("Search Result"),
                             QObject::tr("Highlighted search results inside the editor."),
                             FormatDescription::ShowBackgroundControl);
    formatDescr.emplace_back(C_SEARCH_SCOPE, QObject::tr("Search Scope"),
                             QObject::tr("Section where the pattern is searched in."),
                             FormatDescription::ShowBackgroundControl);
    formatDescr.emplace_back(C_PARENTHESES, QObject::tr("Parentheses"),
                             QObject::tr("Displayed when matching parentheses, square brackets "
                                         "or curly brackets are found."));
    formatDescr.emplace_back(C_PARENTHESES_MISMATCH, QObject::tr("Mismatched Parentheses"),
                             QObject::tr("Displayed when mismatched parentheses, "
                                         "square brackets, or curly brackets are found."));
    formatDescr.emplace_back(C_AUTOCOMPLETE, QObject::tr("Auto Complete"),
                             QObject::tr("Displayed when a character is automatically inserted "
                                         "like brackets or quotes."));
    formatDescr.emplace_back(C_CURRENT_LINE, QObject::tr("Current Line"),
                             QObject::tr("Line where the cursor is placed in."),
                             FormatDescription::ShowBackgroundControl);

    FormatDescription currentLineNumber(C_CURRENT_LINE_NUMBER,
                                        QObject::tr("Current Line Number"),
                                        QObject::tr("Line number located on the left side of the "
                                                    "editor where the cursor is placed in."),
                                        Qt::darkGray,
                                        FormatDescription::AllControlsExceptUnderline);
    currentLineNumber.format().setBold(true);
    formatDescr.push_back(std::move(currentLineNumber));


    formatDescr.emplace_back(C_OCCURRENCES, QObject::tr("Occurrences"),
                             QObject::tr("Occurrences of the symbol under the cursor.\n"
                                         "(Only the background will be applied.)"),
                             FormatDescription::ShowBackgroundControl);
    formatDescr.emplace_back(C_OCCURRENCES_UNUSED,
                             QObject::tr("Unused Occurrence"),
                             QObject::tr("Occurrences of unused variables."),
                             Qt::darkYellow,
                             QTextCharFormat::SingleUnderline);
    formatDescr.emplace_back(C_OCCURRENCES_RENAME, QObject::tr("Renaming Occurrence"),
                             QObject::tr("Occurrences of a symbol that will be renamed."),
                             FormatDescription::ShowBackgroundControl);

    // Standard categories
    formatDescr.emplace_back(C_NUMBER, QObject::tr("Number"), QObject::tr("Number literal."),
                             Qt::darkBlue);
    formatDescr.emplace_back(C_STRING, QObject::tr("String"),
                             QObject::tr("Character and string literals."), Qt::darkGreen);
    formatDescr.emplace_back(C_PRIMITIVE_TYPE, QObject::tr("Primitive Type"),
                             QObject::tr("Name of a primitive data type."), Qt::darkYellow);
    formatDescr.emplace_back(C_TYPE, QObject::tr("Type"), QObject::tr("Name of a type."),
                             Qt::darkMagenta);
    formatDescr.emplace_back(C_LOCAL, QObject::tr("Local"),
                             QObject::tr("Local variables."), QColor(9, 46, 100));
    formatDescr.emplace_back(C_FIELD, QObject::tr("Field"),
                             QObject::tr("Class' data members."), Qt::darkRed);
    formatDescr.emplace_back(C_GLOBAL, QObject::tr("Global"),
                             QObject::tr("Global variables."), QColor(206, 92, 0));
    formatDescr.emplace_back(C_ENUMERATION, QObject::tr("Enumeration"),
                             QObject::tr("Applied to enumeration items."), Qt::darkMagenta);

    TextEditor::Format functionFormat;
    functionFormat.setForeground(QColor(0, 103, 124));
    formatDescr.emplace_back(C_FUNCTION, QObject::tr("Function"), QObject::tr("Name of a function."),
                             functionFormat);
    functionFormat.setItalic(true);
    formatDescr.emplace_back(C_VIRTUAL_METHOD, QObject::tr("Virtual Function"),
                             QObject::tr("Name of function declared as virtual."),
                             functionFormat);

    formatDescr.emplace_back(C_BINDING, QObject::tr("QML Binding"),
                             QObject::tr("QML item property, that allows a "
                                         "binding to another property."),
                             Qt::darkRed);

    TextEditor::Format qmlLocalNameFormat;
    qmlLocalNameFormat.setItalic(true);
    formatDescr.emplace_back(C_QML_LOCAL_ID, QObject::tr("QML Local Id"),
                             QObject::tr("QML item id within a QML file."), qmlLocalNameFormat);
    formatDescr.emplace_back(C_QML_ROOT_OBJECT_PROPERTY,
                             QObject::tr("QML Root Object Property"),
                             QObject::tr("QML property of a parent item."), qmlLocalNameFormat);
    formatDescr.emplace_back(C_QML_SCOPE_OBJECT_PROPERTY,
                             QObject::tr("QML Scope Object Property"),
                             QObject::tr("Property of the same QML item."), qmlLocalNameFormat);
    formatDescr.emplace_back(C_QML_STATE_NAME, QObject::tr("QML State Name"),
                             QObject::tr("Name of a QML state."), qmlLocalNameFormat);

    formatDescr.emplace_back(C_QML_TYPE_ID, QObject::tr("QML Type Name"),
                             QObject::tr("Name of a QML type."), Qt::darkMagenta);

    TextEditor::Format qmlExternalNameFormat = qmlLocalNameFormat;
    qmlExternalNameFormat.setForeground(Qt::darkBlue);
    formatDescr.emplace_back(C_QML_EXTERNAL_ID, QObject::tr("QML External Id"),
                             QObject::tr("QML id defined in another QML file."),
                             qmlExternalNameFormat);
    formatDescr.emplace_back(C_QML_EXTERNAL_OBJECT_PROPERTY,
                             QObject::tr("QML External Object Property"),
                             QObject::tr("QML property defined in another QML file."),
                             qmlExternalNameFormat);

    TextEditor::Format jsLocalFormat;
    jsLocalFormat.setForeground(QColor(41, 133, 199)); // very light blue
    jsLocalFormat.setItalic(true);
    formatDescr.emplace_back(C_JS_SCOPE_VAR, QObject::tr("JavaScript Scope Var"),
                             QObject::tr("Variables defined inside the JavaScript file."),
                             jsLocalFormat);

    TextEditor::Format jsGlobalFormat;
    jsGlobalFormat.setForeground(QColor(0, 85, 175)); // light blue
    jsGlobalFormat.setItalic(true);
    formatDescr.emplace_back(C_JS_IMPORT_VAR, QObject::tr("JavaScript Import"),
                             QObject::tr("Name of a JavaScript import inside a QML file."),
                             jsGlobalFormat);
    formatDescr.emplace_back(C_JS_GLOBAL_VAR, QObject::tr("JavaScript Global Variable"),
                             QObject::tr("Variables defined outside the script."),
                             jsGlobalFormat);

    formatDescr.emplace_back(C_KEYWORD, QObject::tr("Keyword"),
                             QObject::tr("Reserved keywords of the programming language except "
                                         "keywords denoting primitive types."), Qt::darkYellow);
    formatDescr.emplace_back(C_OPERATOR, QObject::tr("Operator"),
                             QObject::tr("Operators (for example operator++ or operator-=)."));
    formatDescr.emplace_back(C_PREPROCESSOR, QObject::tr("Preprocessor"),
                             QObject::tr("Preprocessor directives."), Qt::darkBlue);
    formatDescr.emplace_back(C_LABEL, QObject::tr("Label"), QObject::tr("Labels for goto statements."),
                             Qt::darkRed);
    formatDescr.emplace_back(C_COMMENT, QObject::tr("Comment"),
                             QObject::tr("All style of comments except Doxygen comments."),
                             Qt::darkGreen);
    formatDescr.emplace_back(C_DOXYGEN_COMMENT, QObject::tr("Doxygen Comment"),
                             QObject::tr("Doxygen comments."), Qt::darkBlue);
    formatDescr.emplace_back(C_DOXYGEN_TAG, QObject::tr("Doxygen Tag"), QObject::tr("Doxygen tags."),
                             Qt::blue);
    formatDescr.emplace_back(C_VISUAL_WHITESPACE, QObject::tr("Visual Whitespace"),
                             QObject::tr("Whitespace.\nWill not be applied to whitespace "
                                         "in comments and strings."), Qt::lightGray);
    formatDescr.emplace_back(C_DISABLED_CODE, QObject::tr("Disabled Code"),
                             QObject::tr("Code disabled by preprocessor directives."));

    // Diff categories
    formatDescr.emplace_back(C_ADDED_LINE, QObject::tr("Added Line"),
                             QObject::tr("Applied to added lines in differences (in diff editor)."),
                             QColor(0, 170, 0));
    formatDescr.emplace_back(C_REMOVED_LINE, QObject::tr("Removed Line"),
                             QObject::tr("Applied to removed lines in differences (in diff editor)."),
                             Qt::red);
    formatDescr.emplace_back(C_DIFF_FILE, QObject::tr("Diff File"),
                             QObject::tr("Compared files (in diff editor)."), Qt::darkBlue);
    formatDescr.emplace_back(C_DIFF_LOCATION, QObject::tr("Diff Location"),
                             QObject::tr("Location in the files where the difference is "
                                         "(in diff editor)."), Qt::blue);

    // New diff categories
    formatDescr.emplace_back(C_DIFF_FILE_LINE, QObject::tr("Diff File Line"),
                             QObject::tr("Applied to lines with file information "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(255, 255, 0)));
    formatDescr.emplace_back(C_DIFF_CONTEXT_LINE, QObject::tr("Diff Context Line"),
                             QObject::tr("Applied to lines describing hidden context "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(175, 215, 231)));
    formatDescr.emplace_back(C_DIFF_SOURCE_LINE, QObject::tr("Diff Source Line"),
                             QObject::tr("Applied to source lines with changes "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(255, 223, 223)));
    formatDescr.emplace_back(C_DIFF_SOURCE_CHAR, QObject::tr("Diff Source Character"),
                             QObject::tr("Applied to removed characters "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(255, 175, 175)));
    formatDescr.emplace_back(C_DIFF_DEST_LINE, QObject::tr("Diff Destination Line"),
                             QObject::tr("Applied to destination lines with changes "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(223, 255, 223)));
    formatDescr.emplace_back(C_DIFF_DEST_CHAR, QObject::tr("Diff Destination Character"),
                             QObject::tr("Applied to added characters "
                                         "in differences (in side-by-side diff editor)."),
                             TextEditor::Format(QColor(), QColor(175, 255, 175)));

    formatDescr.emplace_back(C_LOG_CHANGE_LINE, QObject::tr("Log Change Line"),
                             QObject::tr("Applied to lines describing changes in VCS log."),
                             TextEditor::Format(QColor(192, 0, 0), QColor()));


    // Mixin categories
    formatDescr.emplace_back(C_ERROR,
                             QObject::tr("Error"),
                             QObject::tr("Underline color of error diagnostics."),
                             QColor(255,0, 0),
                             QTextCharFormat::SingleUnderline,
                             FormatDescription::ShowUnderlineControl);
    formatDescr.emplace_back(C_ERROR_CONTEXT,
                             QObject::tr("Error Context"),
                             QObject::tr("Underline color of the contexts of error diagnostics."),
                             QColor(255,0, 0),
                             QTextCharFormat::DotLine,
                             FormatDescription::ShowUnderlineControl);
    formatDescr.emplace_back(C_WARNING,
                             QObject::tr("Warning"),
                             QObject::tr("Underline color of warning diagnostics."),
                             QColor(255, 190, 0),
                             QTextCharFormat::SingleUnderline,
                             FormatDescription::ShowUnderlineControl);
    formatDescr.emplace_back(C_WARNING_CONTEXT,
                             QObject::tr("Warning Context"),
                             QObject::tr("Underline color of the contexts of warning diagnostics."),
                             QColor(255, 190, 0),
                             QTextCharFormat::DotLine,
                             FormatDescription::ShowUnderlineControl);
    TextEditor::Format declarationFormat = TextEditor::Format::createMixinFormat();
    declarationFormat.setBold(true);
    formatDescr.emplace_back(C_DECLARATION,
                             QObject::tr("Function Declaration"),
                             QObject::tr("Style adjustments to (function) declarations."),
                             declarationFormat,
                             FormatDescription::ShowFontUnderlineAndRelativeControls);
    formatDescr.emplace_back(C_FUNCTION_DEFINITION,
                             QObject::tr("Function Definition"),
                             QObject::tr("Name of function at its definition."),
                             TextEditor::Format::createMixinFormat());
    TextEditor::Format outputArgumentFormat = TextEditor::Format::createMixinFormat();
    outputArgumentFormat.setItalic(true);
    formatDescr.emplace_back(C_OUTPUT_ARGUMENT,
                             QObject::tr("Output Argument"),
                             QObject::tr("Writable arguments of a function call."),
                             outputArgumentFormat,
                             FormatDescription::ShowFontUnderlineAndRelativeControls);
    return formatDescr;
}
}

struct ColorSchemeEntry
{
    ColorSchemeEntry(const QString &fileName,
                     bool readOnly):
        fileName(fileName),
        name(TextEditor::ColorScheme::readNameOfScheme(fileName)),
        readOnly(readOnly)
    { }

    QString fileName;
    QString name;
    QString id;
    bool readOnly;
};

class SchemeListModel : public QAbstractListModel
{
public:
    SchemeListModel(QObject *parent = 0):
        QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex &parent) const
    { return parent.isValid() ? 0 : m_colorSchemes.size(); }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DisplayRole)
            return m_colorSchemes.at(index.row()).name;

        return QVariant();
    }

    void removeColorScheme(int index)
    {
        beginRemoveRows(QModelIndex(), index, index);
        m_colorSchemes.removeAt(index);
        endRemoveRows();
    }

    void setColorSchemes(const QList<ColorSchemeEntry> &colorSchemes)
    {
        beginResetModel();
        m_colorSchemes = colorSchemes;
        endResetModel();
    }

    const ColorSchemeEntry &colorSchemeAt(int index) const
    { return m_colorSchemes.at(index); }

private:
    QList<ColorSchemeEntry> m_colorSchemes;
};

FontColorsSettingsWidget::FontColorsSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_fontGroup(new QGroupBox(contentWidget()))
  , m_fontLayout(new QVBoxLayout(m_fontGroup))
  , m_fontFamilyLabel(new QLabel(m_fontGroup))
  , m_fontSizeLabel(new QLabel(m_fontGroup))
  , m_fontFamilyBox(new QComboBox(m_fontGroup))
  , m_fontSizeBox(new QComboBox(m_fontGroup))
  , m_fontAntialiasingBox(new QCheckBox(m_fontGroup))
  , m_fontThickBox(new QCheckBox(m_fontGroup))
  , m_fontResetButton(new QPushButton(m_fontGroup))
  /****/
  , m_colorFormatDescriptions(colorFormatDescriptions())
  , m_schemeListModel(new SchemeListModel)
  , m_colorSchemeGroup(new QGroupBox(contentWidget()))
  , m_colorSchemeLayout(new QGridLayout(m_colorSchemeGroup))
  , m_colorSchemeBox(new QComboBox(m_colorSchemeGroup))
  , m_colorSchemeCopyButton(new QPushButton(m_colorSchemeGroup))
  , m_colorSchemeDeleteButton(new QPushButton(m_colorSchemeGroup))
  , m_colorSchemeEdit(new TextEditor::Internal::ColorSchemeEdit(m_colorSchemeGroup))
{
    contentLayout()->addWidget(m_fontGroup);
    contentLayout()->addWidget(m_colorSchemeGroup);
    contentLayout()->addStretch();

    /****/

    auto hb1 = new QHBoxLayout;
    hb1->setSpacing(8);
    hb1->setContentsMargins(0, 0, 0, 0);
    hb1->addWidget(m_fontFamilyLabel);
    hb1->addWidget(m_fontFamilyBox);
    hb1->addSpacing(20);
    hb1->addWidget(m_fontSizeLabel);
    hb1->addWidget(m_fontSizeBox);
    hb1->addSpacing(30);
    hb1->addWidget(m_fontResetButton);
    hb1->addStretch();
    auto hb2 = new QHBoxLayout;
    hb2->setSpacing(8);
    hb2->setContentsMargins(0, 0, 0, 0);
    hb2->addWidget(m_fontAntialiasingBox);
    hb2->addWidget(m_fontThickBox);
    hb2->addStretch();

    m_fontLayout->setSpacing(8);
    m_fontLayout->setContentsMargins(6, 6, 6, 6);
    m_fontLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_fontLayout->addLayout(hb1);
    m_fontLayout->addLayout(hb2);

    m_fontGroup->setTitle(tr("Font"));
    m_fontFamilyLabel->setText(tr("Family") + ":");
    m_fontFamilyBox->addItems(QFontDatabase().families());
    m_fontSizeLabel->setText(tr("Size") + ":");
    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
                             "18", "24", "36", "48", "64", "72", "96", "144"});
    m_fontAntialiasingBox->setText(tr("Prefer antialiasing"));
    m_fontThickBox->setText(tr("Prefer thicker"));
    m_fontResetButton->setText(tr("Reset"));

    m_fontFamilyBox->setToolTip(tr("Chage font family"));
    m_fontSizeBox->setToolTip(tr("Chage font pixel size"));
    m_fontAntialiasingBox->setToolTip(tr("Enable font antialiasing"));
    m_fontThickBox->setToolTip(tr("Enable text thickness increasing"));
    m_fontResetButton->setToolTip(tr("Reset font settings to default"));

    m_fontFamilyBox->setCursor(Qt::PointingHandCursor);
    m_fontSizeBox->setCursor(Qt::PointingHandCursor);
    m_fontAntialiasingBox->setCursor(Qt::PointingHandCursor);
    m_fontThickBox->setCursor(Qt::PointingHandCursor);
    m_fontResetButton->setCursor(Qt::PointingHandCursor);

    /****/

    m_colorSchemeLayout->setSpacing(8);
    m_colorSchemeLayout->setContentsMargins(6, 6, 6, 6);
    m_colorSchemeLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_colorSchemeLayout->addWidget(m_colorSchemeBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeCopyButton, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeDeleteButton, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeEdit, 1, 0, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);

    m_colorSchemeGroup->setTitle(tr("Color Scheme"));
    m_colorSchemeDeleteButton->setText(tr("Delete"));
    m_colorSchemeCopyButton->setText(tr("Copy") + "...");

    m_colorSchemeBox->setToolTip(tr("Chage code color scheme"));
    m_colorSchemeCopyButton->setToolTip(tr("Copy and create new color scheme from current scheme"));
    m_colorSchemeDeleteButton->setToolTip(tr("Delete current color scheme"));

    m_colorSchemeBox->setCursor(Qt::PointingHandCursor);
    m_colorSchemeCopyButton->setCursor(Qt::PointingHandCursor);
    m_colorSchemeDeleteButton->setCursor(Qt::PointingHandCursor);

    m_colorSchemeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_colorSchemeBox->setSizePolicy(QSizePolicy::MinimumExpanding, m_colorSchemeBox->sizePolicy().verticalPolicy());

    /****/

    addColorSchemes(m_colorSchemeBox);

    connect(m_fontResetButton, &QPushButton::clicked, this, [=] {
        const FontColorsSettings settings;
        m_fontFamilyBox->setCurrentText(settings.fontFamily);
        m_fontSizeBox->setCurrentText(QString::number(settings.fontPixelSize));
        m_fontThickBox->setChecked(settings.fontPreferThick);
        m_fontAntialiasingBox->setChecked(settings.fontPreferAntialiasing);
    });

    activate();
    reset();
}

void FontColorsSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    FontColorsSettings* settings = CodeEditorSettings::fontColorsSettings();
    /****/
    settings->fontFamily = m_fontFamilyBox->currentText();
    settings->fontPixelSize = m_fontSizeBox->currentText().toInt();
    settings->fontPreferThick = m_fontThickBox->isChecked();
    settings->fontPreferAntialiasing = m_fontAntialiasingBox->isChecked();
    /****/
    settings->write();
}

void FontColorsSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    const FontColorsSettings* settings = CodeEditorSettings::fontColorsSettings();
    /****/
    m_fontFamilyBox->setCurrentText(settings->fontFamily);
    m_fontSizeBox->setCurrentText(QString::number(settings->fontPixelSize));
    m_fontThickBox->setChecked(settings->fontPreferThick);
    m_fontAntialiasingBox->setChecked(settings->fontPreferAntialiasing);
}

QIcon FontColorsSettingsWidget::icon() const
{
    return QIcon(":/images/settings/fontcolors.png");
}

QString FontColorsSettingsWidget::title() const
{
    return tr("Font") + " && " + tr("Colors");
}

bool FontColorsSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_fontFamilyLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontAntialiasingBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontThickBox->text().contains(word, Qt::CaseInsensitive);
}