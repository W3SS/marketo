#include "mainview.h"
#include "kmarknote_generalsettings.h"
#include "terminalpanel.h"
#include "navpanel.h"
#include "listpanel.h"

#include <QDir>
#include <QtDebug>
#include <QVBoxLayout>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QSplitter>
#include <QModelIndex>

#include <KDE/KWebView>
#include <KDE/KUrl>
#include <KDE/KAction>
#include <KDE/KLocale>
#include <KTextEditor/Editor>
#include <KTextEditor/EditorChooser>
#include <KDE/KLineEdit>
#include <KDE/KMessageBox>


MainView::MainView(QWidget *parent, KAction *pAction)
    : Panel(parent)
{
    setupUI();
    GeneralSettings* generalSettings = GeneralSettings::self();
    const bool firstRun = (generalSettings->version() == 0);
    if (firstRun) {
        // TODO: let user choose home directory
        qDebug() << "firstRun of KMarkNote";
    }
    previewAction = pAction;
    column = generalSettings->startViewNumber();
    switch (column) {
        case 1:
            oneColView();
            break;
        case 2:
            twoColView();
            break;
        case 3:
            threeColView();
            break;
        default:
            threeColView();
            break;
    }
}

void MainView::setupUI()
{
    resize(629, 470);
    verticalLayout = new QVBoxLayout(this);
    vsplitter = new QSplitter(Qt::Vertical, this);
    hsplitter = new QSplitter(Qt::Horizontal, this);
    
    terminal = new TerminalPanel(this);
    terminal->hide();
    connect(terminal, SIGNAL(changeUrl(KUrl)), this, SLOT(setUrl(KUrl)));
    
    navigator = new Navigator(this);
    connect(navigator, SIGNAL(changeUrl(KUrl)), this, SLOT(setUrl(KUrl)));
    
    listPanel = new ListPanel(this);
    connect(listPanel, SIGNAL(changeUrl(KUrl)), this, SLOT(setUrl(KUrl)));
    
    noteView = new NoteView(hsplitter);
    note = noteView->note;
    markPad = noteView->markPad;

    vsplitter->addWidget(hsplitter);
    vsplitter->addWidget(terminal);
    QList<int> sizeList;
    sizeList << 800 << 300;
    vsplitter->setSizes(sizeList);
    verticalLayout->addWidget(vsplitter);

    hsplitter->addWidget(navigator);
    hsplitter->addWidget(listPanel);
    hsplitter->addWidget(noteView);
}

bool MainView::urlChanged()
{
    terminal->setUrl(url());
    listPanel->setUrl(url());
    navigator->setUrl(url());
    if (!url().toLocalFile().endsWith("/"))
        noteView->openUrl(url());
    return true;
}

KTextEditor::View* MainView::getEditor()
{
    return markPad->m_editor;
}

bool MainView::preview()
{
    markPad->preview(column == 2);
    previewAction->setChecked(true);
    return true;
}

bool MainView::unpreview()
{
    if (column == 2)
        return true;
    else {
        markPad->unpreview();
        previewAction->setChecked(false);
        return false;
    }
}

void MainView::oneColView()
{
    navigator->setHidden(true);
    listPanel->setHidden(true);
    noteView->hideTitleLine();
    
    column = 1;
    unpreview();
}

void MainView::twoColView()
{
    QList<int> sizeList;
    sizeList << 0 << 0 << 400;
    hsplitter->setSizes(sizeList);
    
    navigator->setHidden(true);
    listPanel->setHidden(true);
    noteView->hideTitleLine();
    
    column = 2;
    preview();
}

void MainView::threeColView()
{
    navigator->setHidden(false);
    listPanel->setHidden(false);
    noteView->showTitleLine();
    
    QList<int> sizeList;
    sizeList << 50 << 50<< 300;
    hsplitter->setSizes(sizeList);
    
    column = 3;
    unpreview();
}

void MainView::openUrl(KUrl url)
{
    // TODO:if the url is not in the watching dir and is in three column view
    // switch to one column view
    noteView->openUrl(url);
}

void MainView::newNote()
{
    KUrl url;
    
    url = note->url();
    url = url.directory().append("/Untitle.md");
    noteView->setTitle(url.fileName());
    note->openUrl(url);
}

void MainView::toggleTerminal()
{
    if (terminal->isVisible())
        terminal->hide();
    else
        terminal->show();
}

MainView::~MainView()
{
}

#include "mainview.moc"