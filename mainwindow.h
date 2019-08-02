#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QDir>

class QSettings;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadDirSettings();
    double calculationDuration();

public slots:
    void openFolder();
    void updateSettings();
    void runEncoder();

private:
    void createSourceListFile();
    QString determineProjectName();

    Ui::MainWindow *ui;
    QStringList m_SourceList;
    QDir m_SourceDir;
    QSettings* m_pDirSettings;
    bool m_AllowChangeSettings;
};

#endif // MAINWINDOW_H
