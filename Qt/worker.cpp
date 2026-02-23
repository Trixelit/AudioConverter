#include "worker.h"
#include <QFileInfo>

Worker::Worker(QStringList inputFiles, QString outputPath, Target target, bool batchMode)
    : m_inputFiles(inputFiles),
    m_outputPath(outputPath),
    m_target(target),
    m_batchMode(batchMode)
{}

void Worker::process() {
    int successCount = 0;
    int failCount = 0;

    if (!m_batchMode) {
        int result = convertAudio(
            m_inputFiles.first().toStdString(),
            m_outputPath.toStdString(),
            m_target,
            [this](double progress) {
                emit fileProgress(0, progress);
            }
            );

        if (result == 0) {
            successCount++;
        } else { failCount++; }

    } else {
        for (int i = 0; i < m_inputFiles.size(); ++i) {

            QString file = m_inputFiles[i].trimmed();
            QFileInfo info(file);

            QString outPath = m_outputPath + "/" + info.baseName() + ".wav";

            int result = convertAudio(
                file.toStdString(),
                outPath.toStdString(),
                m_target,
                [this, i](double progress) {
                    emit fileProgress(i, progress);
                }
                );

            if (result == 0) successCount++;
            else failCount++;
        }
    }
    emit finished(successCount, failCount);
}
