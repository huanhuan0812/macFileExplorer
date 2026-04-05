#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QIcon>
#include <QStyle>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isCutOperation(false)
    , renameEditor(nullptr)
{
    setupUI();
    setWindowTitle("文件资源管理器");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    setupNavigationBar();
    setupAddressBar();
    setupContentArea();
    setupStatusBar();
    setupContextMenu();
    
    mainLayout->addWidget(navigationBar);
    mainLayout->addWidget(addressContainer);
    mainLayout->addWidget(contentStack, 1);
}

void MainWindow::setupNavigationBar()
{
    navigationBar = new QWidget(this);
    navigationBar->setFixedHeight(40);
    navigationBar->setStyleSheet("background-color: #ffffff; border-bottom: 1px solid #d0d0d0;");
    
    QHBoxLayout *layout = new QHBoxLayout(navigationBar);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(5);
    
    // 导航按钮
    backButton = new QPushButton("←", this);
    forwardButton = new QPushButton("→", this);
    upButton = new QPushButton("↑", this);
    refreshButton = new QPushButton("⟳", this);
    newFolderButton = new QPushButton("新建文件夹", this);
    viewModeButton = new QPushButton("视图", this);
    
    // 根据操作系统设置不同的工具提示
#ifdef Q_OS_MAC
    backButton->setToolTip("后退 (Cmd+← 或 Cmd+[)");
    forwardButton->setToolTip("前进 (Cmd+→ 或 Cmd+])");
    upButton->setToolTip("向上 (Cmd+↑)");
    refreshButton->setToolTip("刷新 (F5 或 Cmd+R)");
    newFolderButton->setToolTip("新建文件夹 (Cmd+N)");
#else
    backButton->setToolTip("后退 (Alt+←)");
    forwardButton->setToolTip("前进 (Alt+→)");
    upButton->setToolTip("向上 (Alt+↑)");
    refreshButton->setToolTip("刷新 (F5)");
    newFolderButton->setToolTip("新建文件夹 (Ctrl+N)");
#endif
    
    QList<QPushButton*> buttons = {backButton, forwardButton, upButton, refreshButton};
    for (auto btn : buttons) {
        btn->setFixedSize(40, 30);
        btn->setStyleSheet("QPushButton { background-color: transparent; border: none; font-size: 14pt; }"
                           "QPushButton:hover { background-color: #e5e5e5; border-radius: 4px; }");
    }
    
    newFolderButton->setStyleSheet("QPushButton { background-color: transparent; padding: 5px 10px; }"
                                   "QPushButton:hover { background-color: #e5e5e5; border-radius: 4px; }");
    
    viewModeButton->setStyleSheet("QPushButton { background-color: transparent; padding: 5px 10px; }"
                                  "QPushButton:hover { background-color: #e5e5e5; border-radius: 4px; }");
    
    layout->addWidget(backButton);
    layout->addWidget(forwardButton);
    layout->addWidget(upButton);
    layout->addWidget(refreshButton);
    layout->addWidget(newFolderButton);
    layout->addStretch();
    layout->addWidget(viewModeButton);
}

void MainWindow::setupAddressBar()
{
    addressContainer = new QWidget(this);
    addressContainer->setFixedHeight(45);
    addressContainer->setStyleSheet("background-color: #ffffff; border-bottom: 1px solid #d0d0d0;");
    
    QHBoxLayout *layout = new QHBoxLayout(addressContainer);
    layout->setContentsMargins(10, 5, 10, 5);
    
    QLabel *addressLabel = new QLabel("地址:", this);
    addressLabel->setStyleSheet("font-weight: bold;");
    
    addressBar = new QLineEdit(this);
    addressBar->setText(QDir::homePath());
    addressBar->setStyleSheet("QLineEdit { border: 1px solid #d0d0d0; border-radius: 4px; padding: 5px; background-color: #f9f9f9; }"
                              "QLineEdit:focus { border-color: #0078d4; background-color: white; }");
    
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("搜索");
    searchBox->setStyleSheet("QLineEdit { border: 1px solid #d0d0d0; border-radius: 4px; padding: 5px; background-color: #f9f9f9; }"
                             "QLineEdit:focus { border-color: #0078d4; background-color: white; }");
    searchBox->setFixedWidth(200);
    
    layout->addWidget(addressLabel);
    layout->addWidget(addressBar, 1);
    layout->addSpacing(20);
    layout->addWidget(searchBox);
    
    connect(addressBar, &QLineEdit::returnPressed, this, &MainWindow::navigateToAddress);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::filterFiles);
}

void MainWindow::setupContentArea()
{
    contentStack = new QStackedWidget(this);
    
    // 创建文件系统模型
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::homePath());
    fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    
    // 列表视图（默认）
    fileListView = new QListView(this);
    fileListView->setModel(fileSystemModel);
    fileListView->setRootIndex(fileSystemModel->index(QDir::homePath()));
    fileListView->setViewMode(QListView::IconMode);
    fileListView->setIconSize(QSize(48, 48));
    fileListView->setGridSize(QSize(100, 100));
    fileListView->setSpacing(10);
    fileListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileListView->setContextMenuPolicy(Qt::CustomContextMenu);
    fileListView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁用默认编辑
    fileListView->setStyleSheet("QListView { background-color: white; outline: none; }"
                                "QListView::item:hover { background-color: #e5f3ff; }"
                                "QListView::item:selected { background-color: #cce5ff; }");
    
    // 详细信息视图
    fileDetailsView = new QTreeView(this);
    fileDetailsView->setModel(fileSystemModel);
    fileDetailsView->setRootIndex(fileSystemModel->index(QDir::homePath()));
    fileDetailsView->setAlternatingRowColors(true);
    fileDetailsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileDetailsView->setContextMenuPolicy(Qt::CustomContextMenu);
    fileDetailsView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁用默认编辑
    fileDetailsView->setStyleSheet("QTreeView { background-color: white; alternate-background-color: #f9f9f9; }"
                                   "QTreeView::item:hover { background-color: #e5f3ff; }"
                                   "QTreeView::item:selected { background-color: #cce5ff; }");
    
    // 设置列宽
    fileDetailsView->header()->setStretchLastSection(true);
    fileDetailsView->setColumnWidth(0, 250);
    fileDetailsView->setColumnWidth(1, 120);
    fileDetailsView->setColumnWidth(2, 100);
    fileDetailsView->setColumnWidth(3, 120);
    
    // 默认显示列表视图
    contentStack->addWidget(fileListView);
    contentStack->addWidget(fileDetailsView);
    contentStack->setCurrentIndex(0);  // 默认为列表视图
    
    // 视图切换按钮功能
    connect(viewModeButton, &QPushButton::clicked, this, [this]() {
        int currentIndex = contentStack->currentIndex();
        contentStack->setCurrentIndex(currentIndex == 0 ? 1 : 0);
        viewModeButton->setText(currentIndex == 0 ? "详细信息" : "图标视图");
    });
    
    // 连接信号
    connect(newFolderButton, &QPushButton::clicked, this, &MainWindow::createNewFolder);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshCurrentView);
    connect(backButton, &QPushButton::clicked, this, &MainWindow::goBack);
    connect(forwardButton, &QPushButton::clicked, this, &MainWindow::goForward);
    connect(upButton, &QPushButton::clicked, this, &MainWindow::goUp);
    connect(fileListView, &QListView::doubleClicked, this, &MainWindow::onItemDoubleClicked);
    connect(fileDetailsView, &QTreeView::doubleClicked, this, &MainWindow::onItemDoubleClicked);
    
    // 右键菜单
    connect(fileListView, &QListView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    connect(fileDetailsView, &QTreeView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    
    // 保存导航历史
    currentPath = QDir::homePath();
    updateNavigationButtons();
}

void MainWindow::setupStatusBar()
{
    statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(statusLabel);
    statusBar()->setStyleSheet("QStatusBar { background-color: #f3f3f3; border-top: 1px solid #d0d0d0; }");
    
    itemCountLabel = new QLabel("0 个项目", this);
    statusBar()->addPermanentWidget(itemCountLabel);
    
    connect(fileSystemModel, &QFileSystemModel::rowsInserted, this, &MainWindow::updateItemCount);
    connect(fileSystemModel, &QFileSystemModel::rowsRemoved, this, &MainWindow::updateItemCount);
    
    updateItemCount();
}

void MainWindow::setupContextMenu()
{
    // 文件右键菜单
    contextMenu = new QMenu(this);
    copyAction = new QAction("复制", this);
    cutAction = new QAction("剪切", this);
    pasteAction = new QAction("粘贴", this);
    deleteAction = new QAction("删除", this);
    renameAction = new QAction("重命名", this);
    propertiesAction = new QAction("属性", this);
    openLocationAction = new QAction("打开文件位置", this);
    
#ifdef Q_OS_MAC
    // Mac 快捷键使用 Cmd 键
    copyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));  // 实际上 Qt 在 Mac 上会自动映射 Cmd+C
    cutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    pasteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    deleteAction->setShortcut(QKeySequence::Delete);
    renameAction->setShortcut(QKeySequence(Qt::Key_F2));
#else
    // Windows/Linux 快捷键
    copyAction->setShortcut(QKeySequence::Copy);      // Ctrl+C
    cutAction->setShortcut(QKeySequence::Cut);        // Ctrl+X
    pasteAction->setShortcut(QKeySequence::Paste);    // Ctrl+V
    deleteAction->setShortcut(QKeySequence::Delete);  // Delete
    renameAction->setShortcut(QKeySequence(Qt::Key_F2));  // F2
#endif
    
    contextMenu->addAction(copyAction);
    contextMenu->addAction(cutAction);
    contextMenu->addAction(pasteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(renameAction);
    contextMenu->addAction(deleteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(propertiesAction);
    contextMenu->addAction(openLocationAction);
    
    // 背景右键菜单
    backgroundMenu = new QMenu(this);
    newFolderAction = new QAction("新建文件夹", this);
    refreshAction = new QAction("刷新", this);
    backgroundMenu->addAction(newFolderAction);
    backgroundMenu->addAction(pasteAction);
    backgroundMenu->addSeparator();
    backgroundMenu->addAction(refreshAction);
    backgroundMenu->addAction(propertiesAction);
    
    // 连接信号
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyFiles);
    connect(cutAction, &QAction::triggered, this, &MainWindow::cutFiles);
    connect(pasteAction, &QAction::triggered, this, &MainWindow::pasteFiles);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteFiles);
    connect(renameAction, &QAction::triggered, this, &MainWindow::renameFileInline);
    connect(propertiesAction, &QAction::triggered, this, &MainWindow::showFileProperties);
    connect(openLocationAction, &QAction::triggered, this, &MainWindow::openFileLocation);
    connect(newFolderAction, &QAction::triggered, this, &MainWindow::createNewFolder);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshCurrentView);
    
    // 添加全局快捷键
#ifdef Q_OS_MAC
    // Mac 快捷键
    QAction *copyShortcut = new QAction(this);
    copyShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    connect(copyShortcut, &QAction::triggered, this, &MainWindow::copyFiles);
    addAction(copyShortcut);
    
    QAction *cutShortcut = new QAction(this);
    cutShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    connect(cutShortcut, &QAction::triggered, this, &MainWindow::cutFiles);
    addAction(cutShortcut);
    
    QAction *pasteShortcut = new QAction(this);
    pasteShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    connect(pasteShortcut, &QAction::triggered, this, &MainWindow::pasteFiles);
    addAction(pasteShortcut);
#else
    // Windows/Linux 快捷键
    QAction *copyShortcut = new QAction(this);
    copyShortcut->setShortcut(QKeySequence::Copy);
    connect(copyShortcut, &QAction::triggered, this, &MainWindow::copyFiles);
    addAction(copyShortcut);
    
    QAction *cutShortcut = new QAction(this);
    cutShortcut->setShortcut(QKeySequence::Cut);
    connect(cutShortcut, &QAction::triggered, this, &MainWindow::cutFiles);
    addAction(cutShortcut);
    
    QAction *pasteShortcut = new QAction(this);
    pasteShortcut->setShortcut(QKeySequence::Paste);
    connect(pasteShortcut, &QAction::triggered, this, &MainWindow::pasteFiles);
    addAction(pasteShortcut);
#endif
    
    // 删除快捷键（Mac 上使用 Delete 或 Cmd+Delete）
    QAction *deleteShortcut = new QAction(this);
#ifdef Q_OS_MAC
    deleteShortcut->setShortcut(QKeySequence(Qt::Key_Delete));
    // 添加 Cmd+Delete 作为备选
    QAction *deleteShortcut2 = new QAction(this);
    deleteShortcut2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
    connect(deleteShortcut2, &QAction::triggered, this, &MainWindow::deleteFiles);
    addAction(deleteShortcut2);
#else
    deleteShortcut->setShortcut(QKeySequence::Delete);
#endif
    connect(deleteShortcut, &QAction::triggered, this, &MainWindow::deleteFiles);
    addAction(deleteShortcut);
    
    // F2 重命名快捷键（Mac 上使用 Enter 键重命名更常见）
    QAction *renameShortcut = new QAction(this);
#ifdef Q_OS_MAC
    renameShortcut->setShortcut(QKeySequence(Qt::Key_Return));  // Enter 键重命名
    // 也支持 F2
    QAction *renameShortcut2 = new QAction(this);
    renameShortcut2->setShortcut(QKeySequence(Qt::Key_F2));
    connect(renameShortcut2, &QAction::triggered, this, &MainWindow::renameFileInline);
    addAction(renameShortcut2);
#else
    renameShortcut->setShortcut(QKeySequence(Qt::Key_F2));
#endif
    connect(renameShortcut, &QAction::triggered, this, &MainWindow::renameFileInline);
    addAction(renameShortcut);
    
    // F5 刷新快捷键
    QAction *refreshShortcut = new QAction(this);
    refreshShortcut->setShortcut(QKeySequence(Qt::Key_F5));
    connect(refreshShortcut, &QAction::triggered, this, &MainWindow::refreshCurrentView);
    addAction(refreshShortcut);
    
#ifdef Q_OS_MAC
    // Mac 上使用 Cmd+R 刷新
    QAction *refreshShortcut2 = new QAction(this);
    refreshShortcut2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(refreshShortcut2, &QAction::triggered, this, &MainWindow::refreshCurrentView);
    addAction(refreshShortcut2);
#endif
    
    // Ctrl+N / Cmd+N 新建文件夹快捷键
    QAction *newFolderShortcut = new QAction(this);
    newFolderShortcut->setShortcut(QKeySequence::New);
    connect(newFolderShortcut, &QAction::triggered, this, &MainWindow::createNewFolder);
    addAction(newFolderShortcut);
    
    // Alt+Up / Cmd+Up 向上快捷键
    QAction *upShortcut = new QAction(this);
#ifdef Q_OS_MAC
    upShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
#else
    upShortcut->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Up));
#endif
    connect(upShortcut, &QAction::triggered, this, &MainWindow::goUp);
    addAction(upShortcut);
    
    // Alt+Left / Cmd+[ 后退快捷键
    QAction *backShortcut = new QAction(this);
#ifdef Q_OS_MAC
    backShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left));
    // Mac 上常用 Cmd+[
    QAction *backShortcut2 = new QAction(this);
    backShortcut2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    connect(backShortcut2, &QAction::triggered, this, &MainWindow::goBack);
    addAction(backShortcut2);
#else
    backShortcut->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
#endif
    connect(backShortcut, &QAction::triggered, this, &MainWindow::goBack);
    addAction(backShortcut);
    
    // Alt+Right / Cmd+] 前进快捷键
    QAction *forwardShortcut = new QAction(this);
#ifdef Q_OS_MAC
    forwardShortcut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right));
    // Mac 上常用 Cmd+]
    QAction *forwardShortcut2 = new QAction(this);
    forwardShortcut2->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    connect(forwardShortcut2, &QAction::triggered, this, &MainWindow::goForward);
    addAction(forwardShortcut2);
#else
    forwardShortcut->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
#endif
    connect(forwardShortcut, &QAction::triggered, this, &MainWindow::goForward);
    addAction(forwardShortcut);
    
    // Ctrl+A / Cmd+A 全选快捷键
    QAction *selectAllShortcut = new QAction(this);
    selectAllShortcut->setShortcut(QKeySequence::SelectAll);
    connect(selectAllShortcut, &QAction::triggered, this, [this]() {
        if (contentStack->currentIndex() == 0) {
            fileListView->selectAll();
        } else {
            fileDetailsView->selectAll();
        }
        statusLabel->setText("已全选");
    });
    addAction(selectAllShortcut);
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QWidget *sender = qobject_cast<QWidget*>(QObject::sender());
    if (!sender) return;
    
    QModelIndex index;
    if (sender == fileListView) {
        index = fileListView->indexAt(pos);
    } else if (sender == fileDetailsView) {
        index = fileDetailsView->indexAt(pos);
    }
    
    if (index.isValid()) {
        showFileContextMenu(pos, index);
    } else {
        showBackgroundContextMenu(pos);
    }
}

void MainWindow::showFileContextMenu(const QPoint &pos, const QModelIndex &index)
{
    Q_UNUSED(pos);
    
    // 获取选中的文件
    QStringList selectedFiles = getSelectedFilePaths();
    bool hasSelection = !selectedFiles.isEmpty();
    bool singleSelection = (selectedFiles.size() == 1);
    
    copyAction->setEnabled(hasSelection);
    cutAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
    renameAction->setEnabled(singleSelection);
    openLocationAction->setEnabled(singleSelection);
    propertiesAction->setEnabled(hasSelection);
    
    contextMenu->exec(QCursor::pos());
}

void MainWindow::showBackgroundContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    
    bool canPaste = !clipboardPaths.isEmpty();
    pasteAction->setEnabled(canPaste);
    
    backgroundMenu->exec(QCursor::pos());
}

QStringList MainWindow::getSelectedFilePaths()
{
    QStringList paths;
    QModelIndexList selectedIndexes;
    
    if (contentStack->currentIndex() == 0) {
        selectedIndexes = fileListView->selectionModel()->selectedIndexes();
    } else {
        selectedIndexes = fileDetailsView->selectionModel()->selectedIndexes();
    }
    
    for (const QModelIndex &index : selectedIndexes) {
        if (index.column() == 0) {  // 只取第一列
            QString path = fileSystemModel->filePath(index);
            if (!path.isEmpty()) {
                paths.append(path);
            }
        }
    }
    
    return paths;
}

QStringList MainWindow::getSelectedFileNames()
{
    QStringList names;
    QModelIndexList selectedIndexes;
    
    if (contentStack->currentIndex() == 0) {
        selectedIndexes = fileListView->selectionModel()->selectedIndexes();
    } else {
        selectedIndexes = fileDetailsView->selectionModel()->selectedIndexes();
    }
    
    for (const QModelIndex &index : selectedIndexes) {
        if (index.column() == 0) {
            QString name = fileSystemModel->fileName(index);
            if (!name.isEmpty()) {
                names.append(name);
            }
        }
    }
    
    return names;
}

void MainWindow::copyFiles()
{
    clipboardPaths = getSelectedFilePaths();
    isCutOperation = false;
    
    if (!clipboardPaths.isEmpty()) {
        statusLabel->setText(QString("已复制 %1 个项目").arg(clipboardPaths.size()));
    }
}

void MainWindow::cutFiles()
{
    clipboardPaths = getSelectedFilePaths();
    isCutOperation = true;
    
    if (!clipboardPaths.isEmpty()) {
        statusLabel->setText(QString("已剪切 %1 个项目").arg(clipboardPaths.size()));
    }
}

void MainWindow::pasteFiles()
{
    if (clipboardPaths.isEmpty()) {
        statusLabel->setText("没有要粘贴的内容");
        return;
    }
    
    int successCount = 0;
    int failCount = 0;
    
    for (const QString &sourcePath : clipboardPaths) {
        QFileInfo sourceInfo(sourcePath);
        QString destPath = currentPath + "/" + sourceInfo.fileName();
        
        // 处理文件名冲突
        int counter = 1;
        QString baseName = sourceInfo.completeBaseName();
        QString extension = sourceInfo.suffix();
        while (QFile::exists(destPath)) {
            if (extension.isEmpty()) {
                destPath = QString("%1/%2 (%3)").arg(currentPath, baseName).arg(counter);
            } else {
                destPath = QString("%1/%2 (%3).%4").arg(currentPath, baseName).arg(counter).arg(extension);
            }
            counter++;
        }
        
        bool success = false;
        if (isCutOperation) {
            // 剪切：移动文件
            success = QFile::rename(sourcePath, destPath);
        } else {
            // 复制：复制文件
            if (sourceInfo.isDir()) {
                success = QDir(sourcePath).exists();  // 目录复制需要递归实现，这里简化
                // 简单复制目录
                QDir dir(sourcePath);
                success = dir.mkpath(destPath);
                if (success) {
                    // 递归复制目录内容（简化版）
                    for (const QString &entry : dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
                        QFile::copy(sourcePath + "/" + entry, destPath + "/" + entry);
                    }
                }
            } else {
                success = QFile::copy(sourcePath, destPath);
            }
        }
        
        if (success) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    if (isCutOperation && successCount > 0) {
        clipboardPaths.clear();  // 剪切操作后清空剪贴板
    }
    
    statusLabel->setText(QString("粘贴完成：成功 %1 个，失败 %2 个").arg(successCount).arg(failCount));
    refreshCurrentView();
}

void MainWindow::deleteFiles()
{
    QStringList selectedFiles = getSelectedFilePaths();
    if (selectedFiles.isEmpty()) return;
    
    int result = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除选中的 %1 个项目吗？").arg(selectedFiles.size()),
                                       QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        int successCount = 0;
        for (const QString &path : selectedFiles) {
            QFileInfo info(path);
            bool success;
            if (info.isDir()) {
                QDir dir(path);
                success = dir.removeRecursively();
            } else {
                success = QFile::remove(path);
            }
            
            if (success) {
                successCount++;
            }
        }
        
        statusLabel->setText(QString("已删除 %1 个项目").arg(successCount));
        refreshCurrentView();
    }
}

void MainWindow::renameFileInline()
{
    // 如果已经有重命名编辑器，先完成当前的重命名
    if (renameEditor) {
        onRenameFinished();
        return;
    }
    
    // 获取选中的文件
    QStringList selectedFiles = getSelectedFilePaths();
    if (selectedFiles.size() != 1) return;
    
    // 获取当前选中的索引
    QModelIndexList selectedIndexes;
    if (contentStack->currentIndex() == 0) {
        selectedIndexes = fileListView->selectionModel()->selectedIndexes();
    } else {
        selectedIndexes = fileDetailsView->selectionModel()->selectedIndexes();
    }
    
    if (selectedIndexes.isEmpty()) return;
    
    QModelIndex index = selectedIndexes.first();
    if (index.column() != 0) {
        // 如果选中的不是第一列，获取同一行的第一列
        index = index.sibling(index.row(), 0);
    }
    
    // 保存重命名信息
    renameIndex = index;
    oldName = fileSystemModel->fileName(renameIndex);
    
    // 获取编辑位置
    QRect itemRect;
    QAbstractItemView *currentView = nullptr;
    
    if (contentStack->currentIndex() == 0) {
        currentView = fileListView;
        itemRect = fileListView->visualRect(renameIndex);
    } else {
        currentView = fileDetailsView;
        itemRect = fileDetailsView->visualRect(renameIndex);
    }
    
    // 创建行编辑控件
    QLineEdit *lineEdit = new QLineEdit(currentView->viewport());
    lineEdit->setText(oldName);
    lineEdit->selectAll();
    lineEdit->setFocus();
    
    // 设置样式，匹配Windows风格
    lineEdit->setStyleSheet("QLineEdit {"
                            "    border: 2px solid #0078d4;"
                            "    background-color: white;"
                            "    selection-background-color: #0078d4;"
                            "    border-radius: 2px;"
                            "    padding: 2px 4px;"
                            "}");
    
    // 设置编辑器位置和大小
    lineEdit->setGeometry(itemRect);
    lineEdit->show();
    
    renameEditor = lineEdit;
    
    // 连接信号
    connect(lineEdit, &QLineEdit::returnPressed, this, &MainWindow::onRenameFinished);
    connect(lineEdit, &QLineEdit::editingFinished, this, &MainWindow::onRenameFinished);
    
    // 安装事件过滤器来处理ESC键
    lineEdit->installEventFilter(this);
}

void MainWindow::onRenameFinished()
{
    if (!renameEditor || !renameIndex.isValid()) {
        if (renameEditor) {
            renameEditor->deleteLater();
            renameEditor = nullptr;
        }
        return;
    }
    
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(renameEditor);
    QString newName = lineEdit->text();
    
    // 清理编辑器
    renameEditor->deleteLater();
    renameEditor = nullptr;
    
    // 如果名称没有变化，不做任何操作
    if (newName.isEmpty() || newName == oldName) {
        return;
    }
    
    // 执行重命名
    QString oldPath = fileSystemModel->filePath(renameIndex);
    QFileInfo info(oldPath);
    QString newPath = info.absolutePath() + "/" + newName;
    
    // 检查文件是否已存在
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "重命名失败", 
                            QString("文件或文件夹 \"%1\" 已存在，无法重命名。").arg(newName),
                            QMessageBox::Ok);
        statusLabel->setText("重命名失败：文件已存在");
        return;
    }
    
    // 检查文件名是否包含非法字符
    if (newName.contains('/') || newName.contains('\\') || newName.contains(':') ||
        newName.contains('*') || newName.contains('?') || newName.contains('"') ||
        newName.contains('<') || newName.contains('>') || newName.contains('|')) {
        QMessageBox::warning(this, "重命名失败", 
                            "文件名不能包含以下字符：\\ / : * ? \" < > |",
                            QMessageBox::Ok);
        statusLabel->setText("重命名失败：文件名包含非法字符");
        return;
    }
    
    // 执行重命名
    if (QFile::rename(oldPath, newPath)) {
        statusLabel->setText(QString("已重命名为: %1").arg(newName));
        refreshCurrentView();
        
        // 选中重命名后的文件
        QModelIndex newIndex = fileSystemModel->index(newPath);
        if (newIndex.isValid()) {
            if (contentStack->currentIndex() == 0) {
                fileListView->setCurrentIndex(newIndex);
            } else {
                fileDetailsView->setCurrentIndex(newIndex);
            }
        }
    } else {
        statusLabel->setText("重命名失败");
        QMessageBox::warning(this, "重命名失败", "无法重命名该文件或文件夹。", QMessageBox::Ok);
    }
    
    // 清除重命名索引
    renameIndex = QModelIndex();
}

void MainWindow::showFileProperties()
{
    QStringList selectedFiles = getSelectedFilePaths();
    if (selectedFiles.isEmpty()) return;
    
    QString message;
    if (selectedFiles.size() == 1) {
        QFileInfo info(selectedFiles.first());
        message = QString("文件属性:\n\n"
                         "名称: %1\n"
                         "路径: %2\n"
                         "大小: %3 bytes\n"
                         "创建时间: %4\n"
                         "修改时间: %5\n"
                         "是否为目录: %6")
                  .arg(info.fileName())
                  .arg(info.absolutePath())
                  .arg(info.size())
                  .arg(info.birthTime().toString())
                  .arg(info.lastModified().toString())
                  .arg(info.isDir() ? "是" : "否");
    } else {
        message = QString("已选择 %1 个项目").arg(selectedFiles.size());
    }
    
    QMessageBox::information(this, "属性", message);
}

void MainWindow::openFileLocation()
{
    QStringList selectedFiles = getSelectedFilePaths();
    if (selectedFiles.size() != 1) return;
    
    QFileInfo info(selectedFiles.first());
    
    // 记录历史
    backHistory.append(currentPath);
    forwardHistory.clear();
    
    setCurrentPath(info.absolutePath());
    
    // 选中该文件
    QModelIndex index = fileSystemModel->index(selectedFiles.first());
    if (index.isValid()) {
        fileListView->setCurrentIndex(index);
        fileDetailsView->setCurrentIndex(index);
    }
}

void MainWindow::navigateToAddress()
{
    QString newPath = addressBar->text();
    if (newPath.isEmpty()) return;
    
    QDir dir(newPath);
    if (dir.exists()) {
        // 记录历史
        backHistory.append(currentPath);
        forwardHistory.clear();
        setCurrentPath(newPath);
    } else {
        statusLabel->setText("路径不存在: " + newPath);
        addressBar->setText(currentPath);
    }
}

void MainWindow::filterFiles(const QString &text)
{
    if (fileSystemModel) {
        statusLabel->setText("搜索: " + (text.isEmpty() ? "显示所有文件" : text));
        // 注意：完整的过滤功能需要自定义代理模型
    }
}

void MainWindow::createNewFolder()
{
    QString newFolderName = "新建文件夹";
    QString newPath = currentPath + "/" + newFolderName;
    int counter = 1;
    
    while (QDir(newPath).exists()) {
        newPath = currentPath + "/" + newFolderName + " (" + QString::number(counter) + ")";
        counter++;
    }
    
    QDir dir(currentPath);
    if (dir.mkdir(QFileInfo(newPath).fileName())) {
        statusLabel->setText("已创建文件夹: " + newPath);
        refreshCurrentView();
        
        // 自动重命名新建的文件夹
        QModelIndex newIndex = fileSystemModel->index(newPath);
        if (newIndex.isValid()) {
            if (contentStack->currentIndex() == 0) {
                fileListView->setCurrentIndex(newIndex);
            } else {
                fileDetailsView->setCurrentIndex(newIndex);
            }
            // 自动进入重命名模式
            renameIndex = newIndex;
            oldName = newFolderName;
            if (counter > 1) {
                oldName = newFolderName + " (" + QString::number(counter-1) + ")";
            }
            renameFileInline();
        }
    } else {
        statusLabel->setText("创建文件夹失败");
    }
}

void MainWindow::goBack()
{
    if (backHistory.isEmpty()) return;
    
    // 获取要返回的路径
    QString previousPath = backHistory.takeLast();
    
    // 将当前路径保存到前进历史
    forwardHistory.append(currentPath);
    
    // 限制历史记录数量
    while (forwardHistory.size() > 50) forwardHistory.removeFirst();
    
    // 导航到上一个路径
    setCurrentPath(previousPath);
}

void MainWindow::goForward()
{
    if (forwardHistory.isEmpty()) return;
    
    // 获取要前进的路径
    QString nextPath = forwardHistory.takeLast();
    
    // 将当前路径保存到后退历史
    backHistory.append(currentPath);
    
    // 限制历史记录数量
    while (backHistory.size() > 50) backHistory.removeFirst();
    
    // 导航到下一个路径
    setCurrentPath(nextPath);
}

void MainWindow::goUp()
{
    QDir dir(currentPath);
    if (dir.cdUp()) {
        QString parentPath = dir.absolutePath();
        // 直接将当前路径加入后退历史，然后导航
        backHistory.append(currentPath);
        forwardHistory.clear();
        setCurrentPath(parentPath);
    }
}

void MainWindow::onItemDoubleClicked(const QModelIndex &index)
{
    if (!fileSystemModel) return;
    
    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);
    
    if (fileInfo.isDir()) {
        // 进入文件夹时记录历史
        backHistory.append(currentPath);
        forwardHistory.clear();
        setCurrentPath(filePath);
    } else if (fileInfo.isFile()) {
        // 用系统默认程序打开文件
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        statusLabel->setText("打开文件: " + fileInfo.fileName());
    }
}

void MainWindow::setCurrentPath(const QString &path)
{
    if (!fileSystemModel) return;
    
    currentPath = path;
    addressBar->setText(path);
    
    // 更新视图
    QModelIndex newIndex = fileSystemModel->index(path);
    fileListView->setRootIndex(newIndex);
    fileDetailsView->setRootIndex(newIndex);
    
    // 更新窗口标题
    setWindowTitle(QFileInfo(path).fileName() + " - 文件资源管理器");
    
    // 更新状态栏
    statusLabel->setText("当前路径: " + path);
    updateItemCount();
    
    // 更新按钮状态
    updateNavigationButtons();
}

void MainWindow::updateNavigationHistory(const QString &oldPath, const QString &newPath)
{
    // 这个函数现在不再使用，保留是为了接口兼容
    Q_UNUSED(oldPath);
    Q_UNUSED(newPath);
}

void MainWindow::refreshCurrentView()
{
    if (!fileSystemModel) return;
    
    // 刷新模型
    QString currentRoot = fileSystemModel->rootPath();
    fileSystemModel->setRootPath(currentRoot);
    
    // 重新设置当前路径
    QModelIndex currentIndex = fileSystemModel->index(currentPath);
    fileListView->setRootIndex(currentIndex);
    fileDetailsView->setRootIndex(currentIndex);
    
    statusLabel->setText("已刷新");
    updateItemCount();
}

void MainWindow::updateItemCount()
{
    if (!fileSystemModel) return;
    
    QModelIndex rootIndex = fileSystemModel->index(currentPath);
    int count = fileSystemModel->rowCount(rootIndex);
    itemCountLabel->setText(QString::number(count) + " 个项目");
}

void MainWindow::updateNavigationButtons()
{
    // 根据历史记录是否为空来启用/禁用按钮
    backButton->setEnabled(!backHistory.isEmpty());
    forwardButton->setEnabled(!forwardHistory.isEmpty());
    
    // 设置按钮样式（禁用时显示灰色）
    QString enabledStyle = "QPushButton { background-color: transparent; border: none; font-size: 14pt; }"
                           "QPushButton:hover { background-color: #e5e5e5; border-radius: 4px; }";
    QString disabledStyle = "QPushButton { background-color: transparent; border: none; font-size: 14pt; color: #c0c0c0; }";
    
    backButton->setStyleSheet(backButton->isEnabled() ? enabledStyle : disabledStyle);
    forwardButton->setStyleSheet(forwardButton->isEnabled() ? enabledStyle : disabledStyle);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == renameEditor && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            // 按ESC取消重命名
            if (renameEditor) {
                renameEditor->deleteLater();
                renameEditor = nullptr;
                renameIndex = QModelIndex();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}