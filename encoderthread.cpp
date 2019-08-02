#include "encoderthread.h"

#include <QDebug>
#include <QDir>
#include <QProcess>

EncoderThread::EncoderThread(const QSettings& settings, const QString& path)
    : m_Path(path)
{
    // Copy settings into hash
    for(const QString& key: settings.allKeys()) {
      m_Settings[key] = settings.value(key);
    }
}

void EncoderThread::run() {

    QStringList params;

    // If combining the video with the lower-third, convert SVG to PNG
    if (m_Settings.value("EncodeSource", false).toBool())
    {
        convertSvgToPng("LowerThird.svg");
    }

    qInfo() << "Running Encoder";

    // Calculate the start time as total number of seconds.
    params << "START_TIME:" + QString::number(m_Settings.value("StartMin").toInt() * 60 + m_Settings.value("StartSec").toDouble());
    params << "END_TIME:" + QString::number(m_Settings.value("EndMin").toInt() * 60 + m_Settings.value("EndSec").toDouble());

    // Iterate over settings
    for (const QString& key : m_Settings.keys())
    {
        // Don't use these commands, because we are calculating the combined seconds for the script.
        if (key != "StartMin" && key != "StartSec" && key != "EndMin" && key != "EndSec")
        {
            params << key + ":" + m_Settings.value(key).toString();
        }
    }

    //qInfo() << params;

//    QProcess* process = new QProcess(this);
//    process->setProcessChannelMode(QProcess::MergedChannels);
//    process->setWorkingDirectory(m_SourceDir.path()); // Run Script from folder of source videos
//    process->startDetached("CMD /c \"" + QDir::currentPath() + "/encode.bat\"", params, m_SourceDir.path());
    QString fullCommand("CMD /c \"CD /d \"" + m_Path + "\" & \"" + QDir::currentPath() + "/encode.bat\" " + params.join(' ') + "\" &");

//    QDebug debug = qDebug();
//    debug.noquote();
//    debug << fullCommand;
    system(fullCommand.toStdString().c_str());

//    if (!process.waitForFinished(100000))
//        qDebug() << "Make failed:" << process.errorString();
//    else
//        qDebug() << "Make output:" << process.readAll();

    //qInfo() << process.readAllStandardOutput();

    qInfo() << "Done Encoding";
}

void EncoderThread::convertSvgToPng(const QString& filename)
{
    const QFileInfo svgInfo(m_Path + "/" + filename);

    if (svgInfo.exists())
    {

    //    // Using Qt's svg render (add to pro: QT += svg)
    //    // Note that this method requires the SVG follow the "tiny" specification.
    //    // See: https://www.w3.org/TR/SVGTiny12/
    //    QImage img = QIcon(svgInfo.absoluteFilePath()).pixmap(1920, 1080).toImage();
    //    img.save(m_SourceDir.path() + "/" + svgInfo.completeBaseName() + ".png");

        // Call Inkscape to convert svg to png
        QProcess process;
        process.start("\"C:/Program Files/Inkscape/inkscape\" -z -f \""
                      + svgInfo.filePath() + "\" -d 96 -e "
                      + "\"" + svgInfo.dir().path() + "/"
                      + svgInfo.completeBaseName() + ".png\"");

        if (!process.waitForFinished())
            return;
    }
}
