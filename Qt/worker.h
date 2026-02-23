#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QStringList>
#include "converter.h"

class Worker : public QObject {
    Q_OBJECT

public:
    Worker(QStringList inputFiles, QString outputPath, Target target, bool batchMode);

public slots:
    void process();

signals:
    void fileProgress(int fileIndex, double progress);
    void progress(int done, int total);
    void finished(int successCount, int failCount);

private:
    QStringList m_inputFiles;
    QString m_outputPath;
    Target m_target;
    bool m_batchMode;
};

#endif // WORKER_H

