#include "mainwindow.h"
#include <QApplication>
#include <QShortcut>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QScreen>
#include <QGuiApplication>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTimer>

// ==================== TitleBar 实现 ====================

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_parentWindow(parent)
    , m_searchVisible(false)
    , m_searchEdit(nullptr)
    , m_minimizeButton(nullptr)
    , m_maximizeButton(nullptr)
    , m_closeButton(nullptr)
    , m_searchButton(nullptr)
    , m_titleLabel(nullptr)
{
    setFixedHeight(
#ifdef Q_OS_MACOS
        28  // macOS 标题栏较矮
#else
        35  // Windows/Linux 标题栏较高
#endif
    );
    
    setAttribute(Qt::WA_StyledBackground);
    
    setupUI();
    
    // 安装事件过滤器
    if (m_searchEdit) {
        m_searchEdit->installEventFilter(this);
    }
    
    // 设置样式
    applySystemStyle();
}

void TitleBar::setupUI()
{
    // 先创建所有控件
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Path");
    m_searchEdit->setFixedWidth(400);
    
    m_searchButton = new QPushButton("🔍",this);
#ifdef Q_OS_MACOS
    m_searchButton->setFixedSize(20, 20);
#else
    m_searchButton->setFixedSize(24, 24);
#endif
    m_searchButton->setToolTip("Go");
    
    // 创建窗口控制按钮
#ifdef Q_OS_MACOS
    // macOS: 使用标准符号
    m_minimizeButton = new QPushButton("−",this);
    m_maximizeButton = new QPushButton("+",this);
    m_closeButton = new QPushButton("×",this);
    
    m_minimizeButton->setFixedSize(12, 12);
    m_maximizeButton->setFixedSize(12, 12);
    m_closeButton->setFixedSize(12, 12);
#else
    // Windows/Linux: 使用标准符号
    m_minimizeButton = new QPushButton("−",this);
    m_maximizeButton = new QPushButton("□",this);
    m_closeButton = new QPushButton("×",this);
    
    m_minimizeButton->setFixedSize(45, 24);
    m_maximizeButton->setFixedSize(45, 24);
    m_closeButton->setFixedSize(45, 24);
#endif
    
    // 设置主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
#ifdef Q_OS_MACOS
    // macOS 布局：窗口按钮在左边，搜索在中间
    mainLayout->setContentsMargins(20, 0, 10, 0);
    mainLayout->setSpacing(8);
    
    // 窗口按钮容器（左对齐）
    QHBoxLayout *windowButtonsLayout = new QHBoxLayout();
    windowButtonsLayout->setSpacing(8);
    windowButtonsLayout->addWidget(m_closeButton);
    windowButtonsLayout->addWidget(m_minimizeButton);
    windowButtonsLayout->addWidget(m_maximizeButton);
    windowButtonsLayout->addStretch();
    mainLayout->addLayout(windowButtonsLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchButton);
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();

    // m_closeButton->setGeometry(10,8,12,12);
    // m_minimizeButton->setGeometry(30,8,12,12);
    // m_maximizeButton->setGeometry(50,8,12,12);

    // m_searchButton->setGeometry(width()/2 - 250, 8, 20, 20);
    // m_searchEdit->setGeometry(width()/2 - 200, 8, 400, 20);

#else
    // Windows/Linux 布局：标题在左边，搜索在中间，窗口按钮在右边
    mainLayout->setContentsMargins(10, 0, 5, 0);
    mainLayout->setSpacing(5);
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchButton);
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    
    // 窗口按钮（右对齐）
    mainLayout->addWidget(m_minimizeButton);
    mainLayout->addWidget(m_maximizeButton);
    mainLayout->addWidget(m_closeButton);
#endif
    
    
    
    connect(m_minimizeButton, &QPushButton::clicked, [this]() {
        if (m_parentWindow) {
            m_parentWindow->showMinimized();
        }
    });
    
    connect(m_maximizeButton, &QPushButton::clicked, [this]() {
        if (m_parentWindow) {
            m_parentWindow->isMaximized() ? m_parentWindow->showNormal() : m_parentWindow->showMaximized();
            updateMaximizeButton();
        }
    });
    
    connect(m_closeButton, &QPushButton::clicked, [this]() {
        if (m_parentWindow) {
            m_parentWindow->close();
        }
    });
}

void TitleBar::applySystemStyle()
{
#ifdef Q_OS_MACOS
    // macOS 风格
    setStyleSheet(R"(
        TitleBar {
            background-color: #f0f0f0;
            border-bottom: 1px solid #d0d0d0;
        }
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 6px;
            padding: 0px;
            font-size: 10px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(0, 0, 0, 0.1);
        }
        QPushButton#closeButton {
            background-color: #ff5f56;
            color: #ff5f56;
        }
        QPushButton#closeButton:hover {
            background-color: #ff3b30;
            color: black;
        }
        QPushButton#minimizeButton {
            background-color: #ffbd2e;
            color: #ffbd2e;
        }
        QPushButton#minimizeButton:hover {
            background-color: #ff9f0a;
            color: black;
        }
        QPushButton#maximizeButton {
            background-color: #27ca3f;
            color: #27ca3f;
        }
        QPushButton#maximizeButton:hover {
            background-color: #00b412;
            color: black;
        }
        QLineEdit {
            background: white;
            border: 1px solid #c0c0c0;
            border-radius: 4px;
            padding: 4px 10px;
            color: #333333;
            font-size: 12px;
            selection-background-color: #007aff;
        }
        QLineEdit:focus {
            border: 1px solid #007aff;
        }
        QLabel {
            color: #333333;
            font-size: 13px;
            font-weight: 500;
        }
    )");
    
    // 设置按钮ID用于样式
    if (m_minimizeButton) {
        m_minimizeButton->setObjectName("minimizeButton");
    }
    if (m_maximizeButton) {
        m_maximizeButton->setObjectName("maximizeButton");
    }
    if (m_closeButton) {
        m_closeButton->setObjectName("closeButton");
    }
    if (m_searchButton) {
        m_searchButton->setObjectName("searchButton");
    }
          

#else
    // Windows/Linux 风格
    setStyleSheet(R"(
        TitleBar {
            background-color: #2d2d30;
            border-bottom: 1px solid #1e1e1e;
        }
        QPushButton {
            background: transparent;
            border: none;
            color: #cccccc;
            padding: 5px 12px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #3e3e42;
        }
        QPushButton#closeButton:hover {
            background-color: #c42b1c;
            color: white;
        }
        QLineEdit {
            background: #3e3e42;
            border: 1px solid #007acc;
            border-radius: 3px;
            padding: 4px 10px;
            color: #cccccc;
            font-size: 12px;
            selection-background-color: #007acc;
        }
        QLineEdit:focus {
            border: 2px solid #007acc;
            padding: 3px 9px;
        }
        QLabel {
            color: #cccccc;
            font-size: 12px;
            padding-left: 10px;
        }
    )");
    
    // 设置按钮ID用于样式
    if (m_minimizeButton) {
        m_minimizeButton->setObjectName("minimizeButton");
    }
    if (m_maximizeButton) {
        m_maximizeButton->setObjectName("maximizeButton");
    }
    if (m_closeButton) {
        m_closeButton->setObjectName("closeButton");
    }
    if (m_searchButton) {
        m_searchButton->setObjectName("searchButton");
    }
#endif
}

void TitleBar::updateMaximizeButton()
{
    if (m_parentWindow && m_maximizeButton) {
        if (m_parentWindow->isMaximized()) {
#ifdef Q_OS_MACOS
            m_maximizeButton->setText("-");
#else
            m_maximizeButton->setText("❐");
#endif
        } else {
#ifdef Q_OS_MACOS
            m_maximizeButton->setText("+");
#else
            m_maximizeButton->setText("□");
#endif
        }
    }
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_MACOS
    // macOS: 双击标题栏最大化（标准行为）
    if (event->button() == Qt::LeftButton && m_parentWindow) {
        // 使用 Qt 6 兼容的方法
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragPosition = event->globalPosition().toPoint() - m_parentWindow->frameGeometry().topLeft();
#else
        m_dragPosition = event->globalPos() - m_parentWindow->frameGeometry().topLeft();
#endif
        event->accept();
    }
#else
    // Windows/Linux: 单击拖动
    if (event->button() == Qt::LeftButton && m_parentWindow) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_dragPosition = event->globalPosition().toPoint() - m_parentWindow->frameGeometry().topLeft();
#else
        m_dragPosition = event->globalPos() - m_parentWindow->frameGeometry().topLeft();
#endif
        event->accept();
    }
#endif
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
#ifdef Q_OS_MACOS
    // macOS: 仅在非最大化时允许拖动
    if (event->buttons() & Qt::LeftButton && m_parentWindow && !m_parentWindow->isMaximized()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_parentWindow->move(event->globalPosition().toPoint() - m_dragPosition);
#else
        m_parentWindow->move(event->globalPos() - m_dragPosition);
#endif
        event->accept();
    }
#else
    // Windows/Linux: 拖动窗口
    if (event->buttons() & Qt::LeftButton && m_parentWindow && !m_parentWindow->isMaximized()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_parentWindow->move(event->globalPosition().toPoint() - m_dragPosition);
#else
        m_parentWindow->move(event->globalPos() - m_dragPosition);
#endif
        event->accept();
    }
#endif
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击标题栏切换最大化（所有系统通用）
    if (event->button() == Qt::LeftButton && m_parentWindow) {
        m_parentWindow->isMaximized() ? m_parentWindow->showNormal() : m_parentWindow->showMaximized();
        updateMaximizeButton();
        event->accept();
    }
}

void TitleBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeButton();
    }
    QWidget::changeEvent(event);
}

// ==================== MainWindow 实现 ====================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_titleBar(nullptr)
    , m_centralWidget(nullptr)
    , m_menuBar(nullptr)
    , m_searchAction(nullptr)
{
    // 设置无边框窗口
#ifdef Q_OS_MACOS
    // macOS: 使用系统原生的无边框窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
#else
    // Windows/Linux: 使用自定义边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
#endif
    
    // 创建自定义标题栏
    m_titleBar = new TitleBar(this);
    
    // 创建菜单栏
    m_menuBar = new QMenuBar();
    
    // 创建中央部件
    m_centralWidget = new QWidget();
    
    // 设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 添加标题栏和菜单栏
    QWidget *headerWidget = new QWidget();
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(m_titleBar);
    headerLayout->addWidget(m_menuBar);
    
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(m_centralWidget, 1);
    
    QWidget *container = new QWidget();
    container->setLayout(mainLayout);
    setCentralWidget(container);
    
    // 设置窗口属性
    setMinimumSize(800, 600);
    resize(800, 600);
    
    // 设置菜单
    setupMenuBar();
    
    // 设置中央部件
    setupCentralWidget();
    
}

MainWindow::~MainWindow()
{
    // 不需要手动删除，Qt 会自动管理子窗口
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_titleBar) {
        m_titleBar->updateMaximizeButton();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (m_titleBar) {
            m_titleBar->updateMaximizeButton();
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::setupMenuBar()
{
    if (!m_menuBar) return;
    
#ifdef Q_OS_MACOS
    // macOS: 菜单栏集成到系统菜单栏
    m_menuBar->setNativeMenuBar(true);
#endif
    
    // 文件菜单
    QMenu *fileMenu = m_menuBar->addMenu("File");
    fileMenu->addAction("New");
    
    
    
    // 帮助菜单
    QMenu *helpMenu = m_menuBar->addMenu("Help");
    //QAction *aboutAction = helpMenu->addAction("About");
    
}

void MainWindow::setupCentralWidget()
{
    if (!m_centralWidget) return;
    
}

#ifdef Q_OS_MACOS
void MainWindow::focusInEvent(QFocusEvent *event)
{
    QMainWindow::focusInEvent(event);
    if (m_titleBar) {
        m_titleBar->setStyleSheet(R"(
        TitleBar {
            background-color: #f0f0f0;
            border-bottom: 1px solid #d0d0d0;
        }
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 6px;
            padding: 0px;
            font-size: 10px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(0, 0, 0, 0.1);
        }
        QPushButton#closeButton {
            background-color: #ff5f56;
            color: #ff5f56;
        }
        QPushButton#closeButton:hover {
            background-color: #ff3b30;
            color: black;
        }
        QPushButton#minimizeButton {
            background-color: #ffbd2e;
            color: #ffbd2e;
        }
        QPushButton#minimizeButton:hover {
            background-color: #ff9f0a;
            color: black;
        }
        QPushButton#maximizeButton {
            background-color: #27ca3f;
            color: #27ca3f;
        }
        QPushButton#maximizeButton:hover {
            background-color: #00b412;
            color: black;
        }
        QLineEdit {
            background: white;
            border: 1px solid #c0c0c0;
            border-radius: 4px;
            padding: 4px 10px;
            color: #333333;
            font-size: 12px;
            selection-background-color: #007aff;
        }
        QLineEdit:focus {
            border: 1px solid #007aff;
        }
        QLabel {
            color: #333333;
            font-size: 13px;
            font-weight: 500;
        }
    )");

    }
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    QMainWindow::focusOutEvent(event);
    qDebug() << "MainWindow lost focus";
    if (m_titleBar) {
        setStyleSheet(R"(
        TitleBar {
            background-color: #f0f0f0;
            border-bottom: 1px solid #d0d0d0;
        }
        QPushButton {
            background: transparent;
            border: none;
            border-radius: 6px;
            padding: 0px;
            font-size: 10px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgba(0, 0, 0, 0.1);
        }
        QPushButton#closeButton {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QPushButton#closeButton:hover {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QPushButton#minimizeButton {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QPushButton#minimizeButton:hover {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QPushButton#maximizeButton {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QPushButton#maximizeButton:hover {
            background-color: #b2b2b2;
            color: #b2b2b2;
        }
        QLineEdit {
            background: white;
            border: 1px solid #c0c0c0;
            border-radius: 4px;
            padding: 4px 10px;
            color: #333333;
            font-size: 12px;
            selection-background-color: #007aff;
        }
        QLineEdit:focus {
            border: 1px solid #007aff;
        }
        QLabel {
            color: #333333;
            font-size: 13px;
            font-weight: 500;
        }
    )");
    }
}

#endif