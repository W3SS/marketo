#include "mainview.h"
// #include "terminalpanel.h"
#include "navpanel.h"
#include "listpanel.h"

#include <KWebView>
#include <KLocale>
#include <KTextEditor/Editor>
#include <KLineEdit>
#include <KMessageBox>
#include <KConfigGroup>

#include <QUrl>
#include <QAction>
#include <QDir>
#include <QtDebug>
#include <QVBoxLayout>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QSplitter>
#include <QModelIndex>

MainView::MainView(QWidget *parent, QAction *pAction)
    : Panel(parent),
    previewAction(pAction)
    
{
    setupUI();
    
    column = 3;
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
    
    //terminal = new TerminalPanel(this);
    //terminal->hide();
    //connect(terminal, SIGNAL(changeUrl(QUrl)), this, SLOT(setUrl(QUrl)));
    
    navigator = new Navigator(this);
    connect(navigator, SIGNAL(changeUrl(QUrl)), this, SLOT(setUrl(QUrl)));
    
    listPanel = new ListPanel(this);
    connect(listPanel, SIGNAL(changeUrl(QUrl)), this, SLOT(setUrl(QUrl)));
    
    noteView = new NoteView(hsplitter, previewAction);
    note = noteView->note;
    markPad = noteView->markPad;

    vsplitter->addWidget(hsplitter);
    //vsplitter->addWidget(terminal);
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
    //terminal->setUrl(url());
    listPanel->setUrl(url());
    navigator->setUrl(url());
    if (QFileInfo(url().toLocalFile()).isFile())
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
    markPad->setFocus();
    previewAction->setChecked(true);
    return true;
}

bool MainView::unpreview()
{
    if (column == 2)
        return true;
    else {
        markPad->unpreview();
        markPad->setFocus();
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

void MainView::openUrl(QUrl url)
{
    // TODO:if the url is not in the watching dir and is in three column view
    // switch to one column view
    noteView->openUrl(url);
}

void MainView::slotOpen(const QUrl& url)
{
    noteView->openUrl(url);
}

void MainView::newNote()
{
    QUrl tmpUrl;
    
    tmpUrl = url();
    if (QFileInfo(tmpUrl.path()).isDir())
        tmpUrl = QUrl::fromLocalFile(tmpUrl.toLocalFile() + QString("/Untitled.md"));
    else
        tmpUrl.setUrl(tmpUrl.url(QUrl::RemoveFilename).append("/Untitled.md"));
    noteView->setTitle(tmpUrl.fileName());
    noteView->focusTitle();
    note->openUrl(tmpUrl);
}

void MainView::toggleTerminal()
{
    /*
    if (terminal->isVisible())
        terminal->hide();
    else
        terminal->show();
    */
}

MainView::~MainView()
{
}

#include "mainview.moc"