#include "CodeEditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>
#include <QAbstractItemView>

CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Keywords
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(QColor(255, 140, 0));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\breturn\\b" << "\\binclude\\b" << "\\bdefine\\b"
                    << "\\bauto\\b" << "\\bconstexpr\\b" << "\\bdecltype\\b"
                    << "\\bnoexcept\\b" << "\\bnullptr\\b" << "\\boverride\\b"
                    << "\\bfinal\\b" << "\\busing\\b" << "\\btry\\b" << "\\bcatch\\b"
                    << "\\bthrow\\b" << "\\bdelete\\b" << "\\bnew\\b" << "\\bthis\\b";

    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Numbers
    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(255, 200, 100));
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*[fF]?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Preprocessor
    QTextCharFormat preprocessorFormat;
    preprocessorFormat.setForeground(QColor(150, 150, 255));
    rule.pattern = QRegularExpression("^\\s*#[a-zA-Z_]+");
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // Qt classes
    QTextCharFormat classFormat;
    classFormat.setForeground(QColor(100, 200, 255));
    classFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Strings
    QTextCharFormat stringFormat;
    stringFormat.setForeground(QColor(150, 255, 150));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Single line comments
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(QColor(128, 128, 128));
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Multi-line comments
    xmlCommentFormat.setForeground(QColor(128, 128, 128));
}

void CppHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, xmlCommentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    highlighter = new CppHighlighter(document());

    // Setup auto-completion
    setupCompleter();

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // Set font and tab settings
    QFont font("Consolas", 11);
    if (!font.exactMatch()) {
        font = QFont("Courier New", 11);
    }
    setFont(font);
    setTabStopDistance(40); // 4 spaces tab width

    // Apply styling
    setStyleSheet(R"(
        CodeEditor {
            background: rgba(30, 30, 30, 200);
            border: 1px solid rgba(255, 140, 0, 100);
            border-radius: 8px;
            color: white;
            selection-background-color: rgba(255, 140, 0, 100);
        }
    )");
}

void CodeEditor::setupCompleter()
{
    QStringList keywords;
    keywords << "class" << "struct" << "namespace" << "public" << "private" 
             << "protected" << "virtual" << "override" << "final" << "static"
             << "const" << "constexpr" << "inline" << "template" << "typename"
             << "if" << "else" << "for" << "while" << "do" << "switch" << "case"
             << "return" << "break" << "continue" << "try" << "catch" << "throw"
             << "int" << "char" << "bool" << "float" << "double" << "void" << "auto"
             << "QString" << "QWidget" << "QObject" << "QApplication" << "QMainWindow"
             << "QVBoxLayout" << "QHBoxLayout" << "QPushButton" << "QLabel"
             << "QTextEdit" << "QLineEdit" << "QTreeView" << "QSplitter";

    completer = new QCompleter(keywords, this);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    completer->setWidget(this);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (completer && completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        default:
            break;
        }
    }

    // Auto-indentation
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        QString line = cursor.block().text();
        QString indent;
        
        // Calculate indentation
        for (int i = 0; i < line.length(); ++i) {
            if (line[i] == ' ' || line[i] == '\t') {
                indent += line[i];
            } else {
                break;
            }
        }
        
        // Add extra indentation for blocks
        if (line.trimmed().endsWith('{')) {
            indent += "    ";
        }
        
        QPlainTextEdit::keyPressEvent(e);
        insertPlainText(indent);
        return;
    }

    // Auto-close brackets and quotes
    if (e->text() == "{") {
        insertPlainText("{}");
        moveCursor(QTextCursor::Left);
        return;
    } else if (e->text() == "(") {
        insertPlainText("()");
        moveCursor(QTextCursor::Left);
        return;
    } else if (e->text() == "[") {
        insertPlainText("[]");
        moveCursor(QTextCursor::Left);
        return;
    } else if (e->text() == "\"") {
        insertPlainText("\"\"");
        moveCursor(QTextCursor::Left);
        return;
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E);
    if (!completer || !isShortcut)
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
                      || eow.contains(e->text().right(1)))) {
        completer->popup()->hide();
        return;
    }

    if (completionPrefix != completer->completionPrefix()) {
        completer->setCompletionPrefix(completionPrefix);
        completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
    }
    
    QRect cr = cursorRect();
    cr.setWidth(completer->popup()->sizeHintForColumn(0)
                + completer->popup()->verticalScrollBar()->sizeHint().width());
    completer->complete(cr);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditor::insertCompletion(const QString &completion)
{
    if (completer->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(255, 140, 0, 30);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(40, 40, 40, 180));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(255, 140, 0));
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
