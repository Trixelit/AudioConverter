#include "fileitemwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QLocale>

FileItemWidget::FileItemWidget(const QString& filePath, double durationSeconds, QWidget* parent) : QWidget(parent) {
    QFileInfo info(filePath);

    auto* mainLayout = new QHBoxLayout(this);

    iconLabel = new QLabel;
    QIcon icon = QFileIconProvider().icon(info);
    iconLabel->setPixmap(icon.pixmap(32,32));

    nameLabel = new QLabel(info.fileName());

    QString sizeText = QLocale().formattedDataSize(info.size());

    int duration = static_cast<int>(durationSeconds);
    int mins = duration / 60;
    int secs = duration % 60;

    QString durationText = QString("%1:%2").arg(mins).arg(secs, 2, 10, QChar('0'));

    infoLabel = new QLabel(durationText + " | " + sizeText);

    progressBar = new QProgressBar;
    progressBar->setRange(0,100);
    progressBar->setValue(0);

    auto* textLayout = new QVBoxLayout;
    textLayout->addWidget(nameLabel);
    textLayout->addWidget(infoLabel);
    textLayout->addWidget(progressBar);

    mainLayout->addWidget(iconLabel);
    mainLayout->addLayout(textLayout);
}

void FileItemWidget::setProgress(double progress) {
    progressBar->setValue(static_cast<int>(progress * 100));
}
