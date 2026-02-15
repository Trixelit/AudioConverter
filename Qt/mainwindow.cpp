#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "converter.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_inBrowseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select Input File", "", "Common Audio Files (*.wav *.mp3 *.ogg *.flac *.aac *.m4a *.alac;;All Files (*.*)");

    if (!fileName.isEmpty()) {
        ui->inFilePath->setText(fileName);
    }
}

void MainWindow::on_outBrowseButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Select Output File", "", "WAV File (*.wav)");

    if (!fileName.isEmpty()) {
        ui->outFilePath->setText(fileName);
    }
}



void MainWindow::on_convertButton_clicked()
{
    if (ui->inFilePath->text().isEmpty() || ui->outFilePath->text().isEmpty()) {
        QMessageBox::warning(this, "Missing file", "Please select input and output files.");
        return;
    }

    ui->inBrowseButton->setEnabled(false);
    ui->outBrowseButton->setEnabled(false);
    ui->convertButton->setEnabled(false);
    QApplication::processEvents();

    Target target;

    if (ui->radio3cx->isChecked()) {
        target = Target::TARGET_3CX;
    } else {
        target = Target::TARGET_YEASTAR;
    }

    if (convertAudio(ui->inFilePath->text().toStdString(), ui->outFilePath->text().toStdString(), target) == 0) {
        QMessageBox::information(this, "Success", "Conversion completed successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Conversion failed, please check the input and output paths are valid.");
    }

    ui->inBrowseButton->setEnabled(true);
    ui->outBrowseButton->setEnabled(true);
    ui->convertButton->setEnabled(true);
}

