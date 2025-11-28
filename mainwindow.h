#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "trayiconmanager.h"
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    // QSystemTrayIcon *tray;
    // QMenu *trayMenu;
    TrayIconManager *trayManager;
    void updateToggleActionText();
    QAction *toggleAction;

private slots:
    void runUsbip();

protected:
    void closeEvent(QCloseEvent *event) override;

};
#endif // MAINWINDOW_H
