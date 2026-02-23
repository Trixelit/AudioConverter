#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "converter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QThread>
#include "worker.h"
#include "fileitemwidget.h"

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

    Target target;

    if (ui->radio3cx->isChecked()) {
        target = Target::TARGET_3CX;
    } else if (ui->radioYeastar->isChecked()) {
        target = Target::TARGET_YEASTAR;
    } else {
        QMessageBox::warning(this, "Missing Target", "Please select a target system.");
        return;
    }

    bool batchMode = ui->batchCheckBox->isChecked();
    QStringList inputFiles = ui->inFilePath->text().split(";", Qt::SkipEmptyParts);
    QString outputPath = ui->outFilePath->text();

    ui->fileListWidget->clear();

    m_fileWidgets.clear();

    for (const QString& file : std::as_const(inputFiles)) {
        QListWidgetItem* item = new QListWidgetItem(ui->fileListWidget);

        double duration = getAudioDurationSeconds(file.toStdString());

        auto* widget = new FileItemWidget(file, duration);

        item->setSizeHint(widget->sizeHint());
        ui->fileListWidget->addItem(item);
        ui->fileListWidget->setItemWidget(item, widget);

        m_fileWidgets.append(widget);
    }

    QThread* thread = new QThread;
    Worker* worker = new Worker(inputFiles, outputPath, target, batchMode);

    worker->moveToThread(thread);

    // When thread starts -> run worker
    connect(thread, &QThread::started, worker, &Worker::process);

    // connect progress bar updates
    connect(worker, &Worker::fileProgress, this, [this](int index, double progress) {
        if (index >= 0 && index < m_fileWidgets.size()) {
            m_fileWidgets[index]->setProgress(progress);

            QListWidgetItem* item = ui->fileListWidget->item(index);

            ui->fileListWidget->scrollToItem(item, QAbstractItemView::PositionAtBottom);
        }
    });


    // When finished
    connect(worker, &Worker::finished, this, [=](int successCount, int failCount){
        QMessageBox::information(this, "Done", QString("Success: %1\nFailed: %2").arg(successCount).arg(failCount));

        ui->convertButton->setEnabled(true);
        ui->inBrowseButton->setEnabled(true);
        ui->outBrowseButton->setEnabled(true);

        thread->quit();
    });

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();
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

