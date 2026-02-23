#ifndef FILEITEMWIDGET_H
#define FILEITEMWIDGET_H

#include <QWidget>

class QLabel;
class QProgressBar;

class FileItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileItemWidget(const QString& filePath, double durationSeconds, QWidget* parent = nullptr);

    void setProgress(double progress); // 0.0 - 1.0

private:
    QLabel* iconLabel;
    QLabel* nameLabel;
    QLabel* infoLabel;
    QProgressBar* progressBar;
};

#endif // FILEITEMWIDGET_H
