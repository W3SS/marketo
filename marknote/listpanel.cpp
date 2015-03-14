#include "listpanel.h"
#include "kmarknote_generalsettings.h"

#include <QVBoxLayout>
#include <QListView>
#include <QFileSystemModel>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

#include <KUrl>

ListPanel::ListPanel(QWidget* parent)
    : Panel(parent),
    m_parent(parent)
{
    lmodel = new QFileSystemModel; 
    lmodel->setRootPath(GeneralSettings::noteDir());
    lmodel->setFilter(QDir::Files);
    
    m_filters << "*.md" << "*.markdown";
    lmodel->setNameFilters(m_filters);
    lmodel->setNameFilterDisables(false);
    
    listView = new QListView(this);
    listView->setModel(lmodel);
    listView->setRootIndex(lmodel->index(GeneralSettings::noteDir()));
    listView->setGridSize(QSize(listView->sizeHint().width(), 34));
    listView->setIconSize(QSize(listView->sizeHint().width(), 34));
    listView->setAlternatingRowColors(true);
    connect(listView, SIGNAL(clicked(QModelIndex)),
        this, SLOT(setUrlFromIndex(QModelIndex)));
    
    vl = new QVBoxLayout(this);
    vl->addWidget(listView);
}

void ListPanel::setUrlFromIndex(const QModelIndex& index)
{
    KUrl url(lmodel->filePath(index));
    
    setUrl(url);
}

bool ListPanel::urlChanged()
{
    if (QFileInfo(url().toLocalFile()).isDir()) {
        lmodel->setRootPath(url().path());
        lmodel->setNameFilters(m_filters);
        listView->setRootIndex(lmodel->index(url().path()));
    }
    emit changeUrl(url());
    return true;
}

void ListPanel::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu *contextMenu = new QMenu();
    QAction *newNoteAction = contextMenu->addAction(QString("New Note"));
    connect(newNoteAction, SIGNAL(triggered()), m_parent, SLOT(newNote()));
    contextMenu->exec(QCursor::pos());
}

ListPanel::~ListPanel()
{

}

#include "listpanel.moc"