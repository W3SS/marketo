#include "kmarkpad.h"
#include "markdown.h"
#include "htmlgenerator.h"

#include <string>
#include <iostream>
#include <sstream>

#include <KDE/KLocale>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/Cursor>
#include <KWebView>
#include <KMessageBox>

#include <QSplitter>
#include <QHBoxLayout>
#include <QTimer>
#include <QList>
#include <QDir>
#include <QWebFrame>
#include <Qt>

using std::string;

KMarkPad::KMarkPad(QWidget *parent)
    : QWidget(parent)
{
    hl = new QHBoxLayout(this);
    hs = new QSplitter(this);
    m_previewer = new KWebView(this);
    m_livePreview = false;
    
    m_previewer->setTextSizeMultiplier(0.8);
    hl->setMargin(0);
    
    // TODO: new_editor should be carefully handled, may memory leak
    KTextEditor::Editor* new_editor = KTextEditor::EditorChooser::editor();
    if (!new_editor) {
        KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                      "please check your KDE installation."));
        m_note = 0;
    } else {
        m_note = new_editor->createDocument(0);
        m_editor = qobject_cast<KTextEditor::View*>(m_note->createView(this));
    }
    hs->addWidget(m_previewer);
    hs->addWidget(m_editor);
    hl->addWidget(hs);
    
    QList<int> sizeList;
    sizeList << 400 << 400;
    hs->setSizes(sizeList);
    
    connect(m_note, SIGNAL(textChanged(KTextEditor::Document *)), 
            this, SLOT(updatePreviewer()));
    connect(m_editor, SIGNAL(cursorPositionChanged(KTextEditor::View *,const KTextEditor::Cursor&)),
            this, SLOT(updatePreviewerByCursor(KTextEditor::View *, const KTextEditor::Cursor&)));
}

void KMarkPad::preview(bool livePreview)
{
    std::stringstream html_ss;
    string html;
    markdown::Document processer;
    QString notePath = QDir::homePath().append("/notes");
    
    // Markdown rendering
    //bool ret = processer.read(std::string(m_note->text().toUtf8().constData()));
    //if (!ret)
    //    return ;
    //processer.write(html_ss);
    //html = html_ss.str();
    HTMLGenerator gererator(string(m_note->text().toUtf8().constData()));
    html = gererator.generated();
    // Preview it
    m_livePreview = livePreview;
    QString content = QString::fromUtf8(html.c_str());
    content = QString("<html>") + QString("<head>")
        + QString("<link href=\"file://") + notePath +QString("/css/style.css\" rel=\"stylesheet\">")
        + QString("</head>") + QString("<body>")
        + content + QString("</body>")
        + QString("</html>");
    m_previewer->setHtml(content, QUrl());
    
    // Scroll to the correct position
    updatePreviewerByCursor(0, m_editor->cursorPosition());
    
    if (livePreview) {
        m_editor->setHidden(false);
        m_previewer->setHidden(false);
    } else {
        m_editor->setHidden(true);
        m_previewer->setHidden(false);
    }
}

void KMarkPad::unpreview()
{
    m_editor->setHidden(false);
    m_previewer->setHidden(true);
    m_livePreview = false;
}

void KMarkPad::updatePreviewer()
{
    if (m_livePreview)
        QTimer::singleShot(1000, this, SLOT(preview()));
}

void KMarkPad::updatePreviewerByCursor(KTextEditor::View *editor, const KTextEditor::Cursor& cursor)
{
    int sourceTotal = m_note->lines();
    int sourceCur = cursor.line();
    int targetTotal = m_previewer->page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
    int targetCur = sourceCur * targetTotal / sourceTotal;
    int offset = (sourceCur - sourceTotal/2) * 400 / sourceTotal;
    
    m_previewer->page()->mainFrame()->setScrollPosition(QPoint(0, targetCur + offset));
}

KMarkPad::~KMarkPad()
{

}

#include "kmarkpad.moc"
