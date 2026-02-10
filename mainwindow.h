#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QPoint>
#include <QEvent>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>

// 自定义标题栏类
class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    QLineEdit* searchEdit() const { return m_searchEdit; }
    
    void setWindowTitle(const QString &title);
    QString windowTitle() const { return m_title; }

    void updateMaximizeButton();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void setupUI();
    
    void applySystemStyle();

    QWidget *m_parentWindow;
    QLineEdit *m_searchEdit;
    QPushButton *m_minimizeButton;
    QPushButton *m_maximizeButton;
    QPushButton *m_closeButton;
    QPushButton *m_searchButton;
    QLabel *m_titleLabel;
    QPoint m_dragPosition;
    QString m_title;
    bool m_searchVisible;
};

// 主窗口类
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    #ifdef Q_OS_MACOS
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    #endif

private:
    void setupMenuBar();
    void setupCentralWidget();
    
    TitleBar *m_titleBar;
    QWidget *m_centralWidget;
    QMenuBar *m_menuBar;
    QAction *m_searchAction;
};
#endif // MAINWINDOW_H