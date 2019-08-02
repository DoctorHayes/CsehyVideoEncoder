#ifndef ENCODERTHREAD_H
#define ENCODERTHREAD_H

#include <QThread>
#include <QSettings>

class EncoderThread : public QThread
{
    Q_OBJECT
public:
    EncoderThread(const QSettings& settings, const QString& path);

    void run() override;

    void convertSvgToPng(const QString& filename);

private:
    QHash<QString, QVariant> m_Settings;
    const QString m_Path;
};

#endif // ENCODERTHREAD_H
