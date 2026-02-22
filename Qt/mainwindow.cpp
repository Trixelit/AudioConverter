#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "converter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

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
    QString filter = "Common Audio Files (*.wav *.mp3 *.ogg *.flac *.aac *.m4a *.alac);;All Files (*.*)";

    if (ui->batchCheckBox->isChecked()) {
        QStringList files = QFileDialog::getOpenFileNames(this, "Select Input Files", QDir::homePath(), filter);

        if (!files.isEmpty()) {
            ui->inFilePath->setText(files.join(";"));
        }
    } else {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Input File", QDir::homePath(), filter);

        if (!fileName.isEmpty()) {
            ui->inFilePath->setText(fileName);
        }
    }
}

void MainWindow::on_outBrowseButton_clicked()
{
    if (ui->batchCheckBox->isChecked()) {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", QDir::homePath(), QFileDialog::ShowDirsOnly);

        if (!dir.isEmpty()) {
            ui->outFilePath->setText(dir);
        }
    } else {
        QString fileName = QFileDialog::getSaveFileName(this, "Select Output File", "", "WAV File (*.wav)");

        if (!fileName.isEmpty()) {
            ui->outFilePath->setText(fileName);
        }
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

    bool batchMode = ui->batchCheckBox->isChecked();

    if (!batchMode) {
        if (convertAudio(ui->inFilePath->text().toStdString(), ui->outFilePath->text().toStdString(), target) == 0) {
            QMessageBox::information(this, "Success", "Conversion completed successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Conversion failed, please check the input and output paths are valid.");
        }
    } else {
        QStringList inputFiles = ui->inFilePath->text().split(";", Qt::SkipEmptyParts);

        QString outDir = ui->outFilePath->text();

        int successCount = 0;
        int failCount = 0;

        for (const QString& file : inputFiles) {
            QString trimmed = file.trimmed();
            QFileInfo info(trimmed);

            QString outPath = outDir + "/" + info.baseName() + ".wav";

            int result = convertAudio(trimmed.toStdString(), outPath.toStdString(), target);

            if (result == 0) {
                successCount++;
            } else {
                failCount++;
            }

            QApplication::processEvents();
        }

        QMessageBox::information(this, "Batch Complete", QString("Success: %1\nFailed: %2").arg(successCount).arg(failCount));
    }

    ui->inBrowseButton->setEnabled(true);
    ui->outBrowseButton->setEnabled(true);
    ui->convertButton->setEnabled(true);
}


void MainWindow::on_batchCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if (ui->batchCheckBox->isChecked()) {
        ui->inTextLabel->setText("Input files:");
        ui->outTextLabel->setText("Output Directory:");
        QApplication::processEvents();
    } else {
        ui->inTextLabel->setText("Input:");
        ui->outTextLabel->setText("Output:");
        QApplication::processEvents();
    }
}

