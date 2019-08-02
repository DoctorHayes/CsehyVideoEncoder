#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "encoderthread.h"

#include <cstdlib>

#include <QFileDialog>
#include <QTextStream>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QByteArray>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pDirSettings(nullptr),
    m_AllowChangeSettings(false)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/footage.ico"));

    connect(ui->folderButton, &QPushButton::clicked,
            this, &MainWindow::openFolder);

    connect(ui->dirEdit, &QLineEdit::returnPressed,
            this, &MainWindow::openFolder);

    connect(ui->startMinEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
    connect(ui->startSecEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
    connect(ui->endMinEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
    connect(ui->endSecEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
    connect(ui->fadeOutEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
    connect(ui->encodeSourceCheckbox, &QCheckBox::toggled, this, &MainWindow::updateSettings);
    connect(ui->extractAudioCheckbox, &QCheckBox::toggled, this, &MainWindow::updateSettings);
    connect(ui->combineAVCheckbox, &QCheckBox::toggled, this, &MainWindow::updateSettings);
    connect(ui->createAACCheckbox, &QCheckBox::toggled, this, &MainWindow::updateSettings);
    connect(ui->projectNameEdit, &QLineEdit::editingFinished, this, &MainWindow::updateSettings);
}

MainWindow::~MainWindow()
{
    delete ui;

    if(m_pDirSettings)
    {
        delete m_pDirSettings;
        m_pDirSettings = nullptr;
    }
}

void MainWindow::openFolder()
{
    QString defaultPath = (ui->dirEdit->text().isEmpty()) ? QDir::currentPath() : ui->dirEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Source Folder", defaultPath);

    // Get a list of the files from the directory
    m_SourceDir = dir;
    QStringList names;
    names << "*_src*.mp4" << "*_src*.mts" << "*_src*.m2t" << "*src_*.m2ts" << "*_src*.mkv" << "*_src*.mov" << "*_src*.m4v";

    m_SourceDir.setNameFilters(names);
    m_SourceList = m_SourceDir.entryList(
                QDir::NoDotAndDotDot
                | QDir::Files);//(QDir::Filter::Files,QDir::SortFlag::NoSort)

    // Sort the list of files.
    m_SourceList.sort(Qt::CaseInsensitive);

    ui->sourceFilesLable->setText(m_SourceList.join("\n"));
    ui->dirEdit->setText(dir);

    createSourceListFile();
    loadDirSettings();
}

void MainWindow::createSourceListFile()
{
    if (m_SourceList.empty())
        return;

    QFile file(m_SourceDir.path() + "/SourceFiles.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "# Input files for the video encoder. Files are in the order they will be combined in.\n";

    for (const QString& file : m_SourceList)
        out << "file '" << file << "'\n";
}

void MainWindow::loadDirSettings()
{
    // Clear old settings
    if(m_pDirSettings)
    {
        m_AllowChangeSettings = false;
        delete m_pDirSettings;
        m_pDirSettings = nullptr;
        ui->encodeSourceCheckbox->setChecked(false);
        ui->srcDurationLbl->setText("");
        ui->projectNameEdit->setText("");
    }

    // Load settings
    if (m_SourceDir.exists() && !m_SourceList.empty())
    {
        m_pDirSettings = new QSettings(m_SourceDir.path() + "/settings.ini", QSettings::IniFormat);
        int startMin = m_pDirSettings->value("StartMin", 0).toInt();
        double startSec = m_pDirSettings->value("StartSec", 0).toDouble();
        int endMin = m_pDirSettings->value("EndMin", -1.0).toInt();
        double endSec = m_pDirSettings->value("EndSec", -1.0).toDouble();

        double duration = calculationDuration();

        int totalEndMin = static_cast<int>(duration) / 60;
        double totalEndSec = duration - totalEndMin * 60;

        ui->srcDurationLbl->setText(QString("Source Duration: %1:%2").arg(totalEndMin).arg(totalEndSec, 2, 'g', -1, '0'));

        if (endMin <= 0 && endSec <= 0)
        {
            endMin = totalEndMin;
            endSec = round(totalEndSec * 10) / 10;
        }

        ui->startMinEdit->setText(QString::number(startMin));
        ui->startSecEdit->setText(QString::number(startSec).rightJustified(2, '0'));

        ui->endMinEdit->setText(QString::number(endMin));
        ui->endSecEdit->setText(QString::number(endSec).rightJustified(2, '0'));

        ui->fadeOutEdit->setText(m_pDirSettings->value("VidFadeOut", 1).toString());
        ui->encodeSourceCheckbox->setChecked(m_pDirSettings->value("EncodeSource", true).toBool());
        ui->extractAudioCheckbox->setChecked(m_pDirSettings->value("ExtractAudio", true).toBool());
        ui->combineAVCheckbox->setChecked(m_pDirSettings->value("CombineAudio", false).toBool());
        ui->createAACCheckbox->setChecked(m_pDirSettings->value("CreateAAC", false).toBool());


        QString name = m_pDirSettings->value("ProjectName").toString();
        if (name.isEmpty())
        {
            name = determineProjectName();
            if (!name.isEmpty())
                m_pDirSettings->setValue("ProjectName", name);
        }
        ui->projectNameEdit->setText(name);

        ui->startMinEdit->setEnabled(true);
        ui->startSecEdit->setEnabled(true);
        ui->endMinEdit->setEnabled(true);
        ui->endSecEdit->setEnabled(true);
        ui->fadeOutEdit->setEnabled(true);
        ui->encodeSourceCheckbox->setEnabled(true);
        ui->extractAudioCheckbox->setEnabled(true);
        ui->combineAVCheckbox->setEnabled(true);
        ui->createAACCheckbox->setEnabled(true);
        ui->encodeButton->setEnabled(true);
        ui->projectNameEdit->setEnabled(true);

        m_AllowChangeSettings = true;

        // Update the settings now so that if this is a new settings file, it is fully initialized
        updateSettings();
    }
    else
    {
        ui->startMinEdit->setEnabled(false);
        ui->startSecEdit->setEnabled(false);
        ui->endMinEdit->setEnabled(false);
        ui->endSecEdit->setEnabled(false);
        ui->fadeOutEdit->setEnabled(false);
        ui->encodeSourceCheckbox->setEnabled(false);
        ui->extractAudioCheckbox->setEnabled(false);
        ui->combineAVCheckbox->setEnabled(false);
        ui->createAACCheckbox->setEnabled(false);
        ui->encodeButton->setEnabled(false);
        ui->projectNameEdit->setEnabled(false);

        m_AllowChangeSettings = false;
    }
}

double MainWindow::calculationDuration()
{
    double duration = 0;
    for(const QString& file : m_SourceList)
    {
        QProcess process;
        process.start("\"C:/Program Files/ffmpeg/bin/ffprobe\" -v error -select_streams v:0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 \""
                      + m_SourceDir.path() + "/\"" + file + "\"");

        if (!process.waitForFinished(40000))
            return 0;

        QStringList out = QString(process.readAllStandardOutput()).split(QRegularExpression("\\r?\\n"));
        if (!out.empty())
            duration += out[0].toDouble();
    }

    return duration;
}

void MainWindow::updateSettings()
{
    if(m_pDirSettings && m_AllowChangeSettings)
    {
        m_pDirSettings->setValue("StartMin", ui->startMinEdit->text().toInt());
        m_pDirSettings->setValue("StartSec", ui->startSecEdit->text().toDouble());
        m_pDirSettings->setValue("EndMin", ui->endMinEdit->text().toInt());
        m_pDirSettings->setValue("EndSec", ui->endSecEdit->text().toDouble());
        m_pDirSettings->setValue("VidFadeOut", ui->fadeOutEdit->text().toDouble());
        m_pDirSettings->setValue("EncodeSource", ui->encodeSourceCheckbox->isChecked());
        m_pDirSettings->setValue("ExtractAudio", ui->extractAudioCheckbox->isChecked());
        m_pDirSettings->setValue("CombineAudio", ui->combineAVCheckbox->isChecked());
        m_pDirSettings->setValue("CreateAAC", ui->createAACCheckbox->isChecked());
        m_pDirSettings->setValue("ProjectName", ui->projectNameEdit->text());
    }
}

void MainWindow::runEncoder()
{
    if (!m_SourceDir.exists() || m_SourceList.empty())
        return;

    EncoderThread *pWorkerThread = new EncoderThread(*m_pDirSettings, m_SourceDir.path());
    //connect(pWorkerThread, &EncoderThread::encodingComplete, this, &MainWindow::handleResults);
    connect(pWorkerThread, &EncoderThread::finished, pWorkerThread, &QObject::deleteLater);
    pWorkerThread->start();
}

QString MainWindow::determineProjectName()
{
    if (m_SourceList.isEmpty())
        return "";

    const QString& first = m_SourceList.first();

    QRegularExpression re("(.+)_src[\\-\\.].+");
    QRegularExpressionMatch match = re.match(first);
    if(match.hasMatch())
    {
        qInfo() << match.captured(1);
        return match.captured(1);
    }
    else if (first.length() <= 4)
    {
        return first + "_encoded";
    }
    else if (m_SourceList.length() == 1)
    {
        return first.left(first.length() - 4);
    }
    else
    {
        return "encoded";
    }
}
