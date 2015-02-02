#include <highlighterbykate.h>

#include <KTextEditor/HighlightInterface>
#include <KTextEditor/EditorChooser>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Range>
#include <KTextEditor/Attribute>
#include <QTextDocument>

HighlighterByKate::HighlighterByKate()
{
    m_new_editor = KTextEditor::EditorChooser::editor();
    m_note = m_new_editor->createDocument(0);
    
    // NOTE: currently it must be created for acccess of KateDocument::lineAttributes
    // TODO: should be removed when upstream fix it
    m_editor = qobject_cast<KTextEditor::View*>(m_note->createView(0));
}

void HighlighterByKate::highlight(const string& plain, const string type, std::ostream& out)
{

    m_note->setText(QString::fromUtf8(plain.c_str()));
    m_note->setHighlightingMode(QString::fromStdString(type.empty() ? "None" : mimeMap[type]));
    out << exportDocument(m_note).toUtf8().constData();
}

void exportText(QString& ret,
                const QString& text,
                const KTextEditor::Attribute::Ptr& attrib)
{
    if ( !attrib || !attrib->hasAnyProperty()) {
        ret.append(text);
        return;
    }

    if ( attrib->fontBold() ) {
        ret.append("<b>");
    }
    if ( attrib->fontItalic() ) {
        ret.append("<i>");
    }

    bool writeForeground = attrib->hasProperty(QTextCharFormat::ForegroundBrush);
    bool writeBackground = attrib->hasProperty(QTextCharFormat::BackgroundBrush);

    if ( writeForeground || writeBackground ) {
        ret.append(QString("<span style='%1%2'>")
            .arg(writeForeground ? QString(QLatin1String("color:") 
                                            + attrib->foreground().color().name() 
                                            + QLatin1Char(';')) 
                                    : QString())
            .arg(writeBackground ? QString(QLatin1String("background:") 
                                            + attrib->background().color().name() 
                                            + QLatin1Char(';')) 
                                    : QString()));
    }
    ret.append(text);

    if ( writeBackground || writeForeground ) {
        ret.append("</span>");
    }
    if ( attrib->fontItalic() ) {
        ret.append("</i>");
    }
    if ( attrib->fontBold() ) {
        ret.append("</b>");
    }
}

QString HighlighterByKate::exportDocument(KTextEditor::Document* note)
{
    QString ret("");
    KTextEditor::Range range;
  
    range = note->documentRange();
  
    KTextEditor::HighlightInterface* hiface = qobject_cast<KTextEditor::HighlightInterface*>(note);

    const KTextEditor::Attribute::Ptr noAttrib(0);
    
    for (int i = 0; i < note->lines(); ++i)
    {
        QString content("");
        const QString &line = note->line(i);

        QList<KTextEditor::HighlightInterface::AttributeBlock> attribs;
        if ( hiface ) {
            attribs = hiface->lineAttributes(i);
        }

        int lineStart = 0;
        int remainingChars = line.length();
        if ( range.onSingleLine() ) {
            lineStart = range.start().column();
            remainingChars = range.columnWidth();
        } else if ( i == range.start().line() ) {
            lineStart = range.start().column();
        } else if ( i == range.end().line() ) {
            remainingChars = range.end().column();
        }

        int handledUntil = lineStart;

        foreach ( const KTextEditor::HighlightInterface::AttributeBlock& block, attribs ) {
            // honor (block-) selections
            if ( block.start + block.length <= lineStart ) {
                continue;
            } else if ( block.start >= lineStart + remainingChars ) {
                break;
            }
            int start = qMax(block.start, lineStart);
            if ( start > handledUntil ) {
                exportText(content, line.mid( handledUntil, start - handledUntil ), noAttrib );
            }
            int length = qMin(block.length, remainingChars);
            exportText(content, line.mid( start, length ), block.attribute);
            handledUntil = start + length;
        }

        if ( handledUntil < lineStart + remainingChars ) {
            exportText(content, line.mid( handledUntil, remainingChars ), noAttrib );
        }
        
        if (i != range.end().line()) {
            ret.append(content.isEmpty() ? "\n" : content);
            ret.append("\n");
        }
    }
    return ret;
}

HighlighterByKate::~HighlighterByKate()
{

}