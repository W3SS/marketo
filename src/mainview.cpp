#include "mainview.h"

#include <QDir>
#include <QtDebug>

MainView::MainView(QWidget *parent)
    : QWidget(parent)
{
    resize(629, 470);
    verticalLayout = new QVBoxLayout(this);
    splitter = new QSplitter(this);
    treeView = new QTreeView(splitter);
    listView = new QListView(splitter);
    previewer = new KWebView(splitter);
    
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(treeView);
    splitter->addWidget(listView);
    splitter->addWidget(previewer);
    
    verticalLayout->addWidget(splitter);
    
    tmodel = new QFileSystemModel;
    tmodel->setRootPath(QDir::homePath());
    tmodel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    treeView->setModel(tmodel);
    treeView->setRootIndex(tmodel->index(QDir::homePath().append("/notes")));
    treeView->resizeColumnToContents(0);
    treeView->setColumnHidden(1, true);
    treeView->setColumnHidden(2, true);
    treeView->setColumnHidden(3, true);
    treeView->setUniformRowHeights(true);
    treeView->setIconSize(QSize(treeView->sizeHint().width(), 34));
    treeView->setHeaderHidden(true);

    lmodel = new QFileSystemModel; 
    lmodel->setRootPath(QDir::homePath());
    lmodel->setFilter(QDir::Files);
    
    QStringList filters;
    filters << "*.md" << "*.markdown";
    lmodel->setNameFilters(filters);
    lmodel->setNameFilterDisables(false);
    
    listView->setModel(lmodel);
    listView->setRootIndex(lmodel->index(QDir::homePath().append("/notes")));
    listView->setGridSize(QSize(listView->sizeHint().width(), 34));
    listView->setIconSize(QSize(listView->sizeHint().width(), 34));
    listView->setAlternatingRowColors(true);
    
    
    connect(treeView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(gotoDir(QModelIndex)));
}

void MainView::gotoDir(const QModelIndex& index)
{
    lmodel->setRootPath(tmodel->filePath(index));
    listView->setRootIndex(lmodel->index(tmodel->filePath(index)));
}

MainView::~MainView()
{
}

#include "mainview.moc"