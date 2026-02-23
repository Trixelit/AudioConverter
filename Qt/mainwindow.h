#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fileitemwidget.h"

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

private slots:
    void on_inBrowseButton_clicked();

    void on_outBrowseButton_clicked();

    void on_convertButton_clicked();

    void on_batchCheckBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::MainWindow *ui;
    QVector<FileItemWidget*> m_fileWidgets;
};
#endif // MAINWINDOW_H
