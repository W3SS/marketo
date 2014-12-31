/***************************************************************************
 *   Copyright (C) 2015 by Darcy Shen <sadhen1992@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kmarknote.h"
#include "markdown.h"

#include <iostream>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <KDE/KXMLGUIFactory>
#include <KDE/KStandardAction>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KFileDialog>
#include <KDE/KUrl>
#include <KDE/KWebView>
#include <KTextEditor/View>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KDE/KMenuBar>

#include <QTemporaryFile>
#include <QUrl>
#include <QTimer>

#include <string>
#include <ostream>
#include <sstream>

KMarknote::KMarknote(QWidget* parent)
    : KXmlGuiWindow(parent)
    , m_view(new MainView(parent))
    , column(1)
    , isPreview(false)
{
    setupAction();
    
    KTextEditor::Editor* new_editor = KTextEditor::EditorChooser::editor();
    if (!new_editor) {
        KMessageBox::error(this,  i18n("A KDE text-editor component could not be found;\n"
                                       "please check your KDE installation."));
        m_note = 0;
    } else {
        m_note = new_editor->createDocument(0);
        m_editor = qobject_cast<KTextEditor::View*>(m_note->createView(this));
        m_view->splitter->addWidget(m_editor);
        
        KConfigGroup cg(KGlobal::config(), "KMarknote");
        setAutoSaveSettings(cg, true);
       
        setCentralWidget(m_view);
        setupGUI(QSize(500,600), Default, "kmarknote.rc");
        guiFactory()->addClient(m_editor);
        restoreWindowSize(cg);
    }
    setupConnect();
    threeColView();
}

void KMarknote::setupAction()
{
    KStandardAction::openNew(this, SLOT(newNote()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());
    
    KAction* previewAction = actionCollection()->addAction("file_preview", this, SLOT(togglePreview()));
    previewAction->setIcon(KIcon("document-preview"));
    previewAction->setText(i18n("Preview"));
    
    KAction* oneColAction = actionCollection()->addAction("win_onecol", this, SLOT(oneColView()));
    KAction* twoColAction = actionCollection()->addAction("win_twocol", this, SLOT(twoColView()));
    KAction* threeColAction = actionCollection()->addAction("win_threecol", this, SLOT(threeColView()));
    
    oneColAction->setText(i18n("One Column View"));
    twoColAction->setText(i18n("Two Column View"));
    threeColAction->setText(i18n("Three Column View"));
    
    oneColAction->setIcon(KIcon("view-split-top-bottom"));
    twoColAction->setIcon(KIcon("view-split-left-right"));
    threeColAction->setIcon(KIcon("view-file-columns"));
}

void KMarknote::setupConnect()
{
    connect(m_note, SIGNAL(modifiedChanged(KTextEditor::Document*)),
            this, SLOT(updateCaption()));
    connect(m_note, SIGNAL(documentUrlChanged(KTextEditor::Document*)),
            this, SLOT(updateCaption()));
    connect(m_view->listView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(open(QModelIndex)));
}

void KMarknote::newNote()
{
    // TODO
}

void KMarknote::open()
{
    KUrl url = KFileDialog::getOpenFileName(KUrl(),QString() ,this);
    m_note->openUrl(url);
    oneColView();
}

void KMarknote::open(const QModelIndex &index)
{
    KUrl url = KUrl(m_view->lmodel->filePath(index));
    m_note->openUrl(url);
    // TODO: if in preview mode, keep preview
}

void KMarknote::updateCaption()
{
    setCaption("[modified]- KMarknote");
}

void KMarknote::togglePreview()
{
    if (isPreview) {
        unpreview();
        isPreview = false;
    } else {
        preview();
        isPreview = true;
    }
}

void KMarknote::preview()
{
    std::stringstream html_ss;
    std::string html;
    markdown::Document processer;
    
    // Markdown rendering
    processer.read(std::string(m_note->text().toUtf8().constData()));
    processer.write(html_ss);
    html = html_ss.str();
    
    // Preview it
    m_view->previewer->setHtml(QString::fromUtf8(html.c_str()), QUrl());   
    switch(column) {
        case 1:
        case 3:
            m_editor->setHidden(true);
            m_view->previewer->setHidden(false);
            break;
        case 2:
        default:
            break;
    }
    isPreview = true;
}

void KMarknote::unpreview()
{
    switch(column) {
        case 1:
        case 3:
            m_editor->setHidden(false);
            m_view->previewer->setHidden(true);
            break;
        case 2:
        default:
            break;
    }   
}

void KMarknote::updatePreviewer()
{
    if (column == 2) {
        QTimer::singleShot(1000, this, SLOT(preview()));
    }
}

void KMarknote::oneColView()
{
    m_view->treeView->setHidden(true);
    m_view->listView->setHidden(true);
    m_view->previewer->setHidden(true);
    column = 1;
}

void KMarknote::twoColView()
{
    QList<int> sizeList;
    
    sizeList << 0 << 0 << 400 << 400;
    m_view->splitter->setSizes(sizeList);
    
    m_view->treeView->setHidden(true);
    m_view->listView->setHidden(true);
    m_view->previewer->setHidden(false);
    column = 2;
}

void KMarknote::threeColView()
{
    QList<int> sizeList;
    m_view->treeView->setHidden(false);
    m_view->listView->setHidden(false);
    m_view->previewer->setHidden(true);
    
    sizeList << 50 << 50<< 300 << 300;
    m_view->splitter->setSizes(sizeList);
    column = 3;
}

KMarknote::~KMarknote()
{
}

#include "kmarknote.moc"