#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QListView>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileSystemModel>
#include <QDir>
#include <QMenu>
#include <QPoint>
#include <QAction>
#include <QClipboard>
#include <QAbstractItemView>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void setupNavigationBar();
    void setupAddressBar();
    void setupContentArea();
    void setupStatusBar();
    void setupContextMenu();
    
    // 辅助函数
    void navigateToAddress();
    void filterFiles(const QString &text);
    void createNewFolder();
    void goBack();
    void goForward();
    void goUp();
    void onItemDoubleClicked(const QModelIndex &index);
    void setCurrentPath(const QString &path);
    void updateNavigationHistory(const QString &oldPath, const QString &newPath);
    void refreshCurrentView();
    void updateItemCount();
    void updateNavigationButtons();
    
    // 文件操作函数
    void copyFiles();
    void cutFiles();
    void pasteFiles();
    void deleteFiles();
    void renameFileInline();  // 内联重命名
    void showFileProperties();
    void openFileLocation();
    
    // 右键菜单处理
    void showContextMenu(const QPoint &pos);
    void showBackgroundContextMenu(const QPoint &pos);
    void showFileContextMenu(const QPoint &pos, const QModelIndex &index);
    
    // 获取选中的文件路径列表
    QStringList getSelectedFilePaths();
    QStringList getSelectedFileNames();

private slots:
    void onRenameFinished();   // 重命名完成槽函数

private:
    QWidget *centralWidget;
    QWidget *addressContainer;
    
    // 标题栏控件
    QPushButton *backButton;
    QPushButton *forwardButton;
    QPushButton *upButton;
    QLineEdit *addressBar;
    QLineEdit *searchBox;
    QPushButton *refreshButton;
    QPushButton *newFolderButton;
    QPushButton *viewModeButton;
    
    // 导航栏
    QWidget *navigationBar;
    QTreeView *navigationTree;
    
    // 内容区域
    QStackedWidget *contentStack;
    QListView *fileListView;
    QTreeView *fileDetailsView;
    QFileSystemModel *fileSystemModel;
    
    QLabel *statusLabel;
    QLabel *itemCountLabel;
    
    // 导航历史
    QStringList backHistory;
    QStringList forwardHistory;
    QString currentPath;
    
    // 剪贴板操作
    QStringList clipboardPaths;
    bool isCutOperation;  // true: 剪切, false: 复制
    
    // 右键菜单
    QMenu *contextMenu;
    QMenu *backgroundMenu;
    QAction *copyAction;
    QAction *cutAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *renameAction;
    QAction *propertiesAction;
    QAction *openLocationAction;
    QAction *newFolderAction;
    QAction *refreshAction;
    
    // 重命名相关
    QWidget *renameEditor;      // 重命名编辑器
    QModelIndex renameIndex;     // 正在重命名的索引
    QString oldName;             // 原文件名
};

#endif // MAINWINDOW_H