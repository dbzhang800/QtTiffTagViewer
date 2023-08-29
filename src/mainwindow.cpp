/****************************************************************************
** Copyright (c) 2018 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optionsdialog.h"
#include "tifffile.h"
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QFontMetricsF>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create action for recent files
    for (int i = 0; i < MaxRecentFiles; ++i) {
        auto act = new QAction(this);
        act->setVisible(false);
        act->setProperty("id", i);
        ui->menu_File->insertAction(ui->actionExit, act);
        m_actionRecentFiles[i] = act;
        connect(act, &QAction::triggered, this, &MainWindow::onActionRecentFileTriggered);
    }
    m_actionSeparator = ui->menu_File->insertSeparator(ui->actionExit);
    m_actionSeparator->setVisible(false);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::onActionOptionsTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);
    connect(ui->actionAboutQt, &QAction::triggered, this, [this]() { QMessageBox::aboutQt(this); });

    loadSettings();
    if (qApp->arguments().size() > 1) {
        auto filePath = qApp->arguments().value(1);
        if (QFileInfo::exists((filePath)))
            doOpenTiffFile(filePath);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    saveSettings();
    evt->accept();
}

void MainWindow::onActionOpenTriggered()
{
    auto filePath = QFileDialog::getOpenFileName(
        this, tr("Open Tiff"), m_recentFiles.value(0, QString()), tr("Tiff Image(*.tiff *.tif)"));
    if (filePath.isEmpty())
        return;

    doOpenTiffFile(filePath);
}

void MainWindow::onActionOptionsTriggered()
{
    OptionsDialog dlg(this);
    dlg.setParserOptions(m_parserOptions);

    if (dlg.exec() == QDialog::Accepted)
        m_parserOptions = dlg.parserOptions();
}

void MainWindow::onActionAboutTriggered()
{
    QString text = tr("<b>%1 %2</b><br/><br/>"
                      "Copyright %3 Debao Zhang &lt;hello@debao.me&gt;<br/>"
                      "All right reserved.<br/>"
                      "<br/>"
                      "Permission is hereby granted, free of charge, to any person obtaining"
                      "a copy of this software and associated documentation files (the"
                      "\"Software\"), to deal in the Software without restriction, including"
                      "without limitation the rights to use, copy, modify, merge, publish,"
                      "distribute, sublicense, and/or sell copies of the Software, and to"
                      "permit persons to whom the Software is furnished to do so, subject to"
                      "the following conditions:<br/>"
                      "<br/>"
                      "The above copyright notice and this permission notice shall be"
                      "included in all copies or substantial portions of the Software.<br/>"
                      "<br/>"
                      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,"
                      "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF"
                      "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND"
                      "NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE"
                      "LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION"
                      "OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION"
                      "WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.")
                       .arg(qApp->applicationName(), qApp->applicationVersion())
                       .arg(2023);

    QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()), text);
}

void MainWindow::onActionRecentFileTriggered()
{
    auto action = qobject_cast<QAction *>(sender());
    auto filePath = m_recentFiles.value(action->property("id").toInt(), QString());
    if (!QFileInfo::exists(filePath))
        return;
    doOpenTiffFile(filePath);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("geometry");
    restoreGeometry(settings.value("rect").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();

    settings.beginGroup("parser");
    m_parserOptions.parserSubIfds = settings.value("parsersubifds", true).toBool();
    settings.endGroup();

    m_recentFiles = settings.value("recentfiles").toStringList();
    updateActionRecentFiles();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("geometry");
    settings.setValue("rect", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();

    settings.beginGroup("parser");
    settings.setValue("parsersubifds", m_parserOptions.parserSubIfds);
    settings.endGroup();

    settings.setValue("recentfiles", m_recentFiles);
}

void MainWindow::doOpenTiffFile(const QString &filePath)
{
    m_recentFiles.removeOne(filePath);
    m_recentFiles.insert(0, filePath);
    if (m_recentFiles.size() > MaxRecentFiles)
        m_recentFiles.removeLast();
    updateActionRecentFiles();

    TiffFile tiff(filePath, m_parserOptions);

    ui->treeWidget->clear();

    if (tiff.hasError()) {
        ui->logEdit->appendPlainText(
            QString("Fail to open the tiff file: %1 [%2]").arg(filePath).arg(tiff.errorString()));
        return;
    }
    setWindowTitle(tr("%1 - QtTiffTagViewer").arg(filePath));

    // headeritem
    {
        auto headerItem = new QTreeWidgetItem(ui->treeWidget);
        headerItem->setText(0, tr("Header"));
        headerItem->setText(1, tiff.headerBytes().toHex(' '));
        headerItem->setExpanded(true);

        auto childItem = new QTreeWidgetItem(headerItem);

        childItem->setText(0, tr("ByteOrder"));
        childItem->setText(1,
                           QString("%1 (%2)")
                               .arg(tiff.headerBytes().left(2))
                               .arg(tiff.byteOrder() == TiffFile::BigEndian ? tr("BigEndian")
                                                                            : tr("LittleEndian")));

        childItem = new QTreeWidgetItem(headerItem);
        childItem->setText(0, tr("Version"));
        childItem->setText(1,
                           QString("%1 (%2)")
                               .arg(tiff.version())
                               .arg(tiff.isBigTiff() ? "BigTiff" : "Classic Tiff"));

        childItem = new QTreeWidgetItem(headerItem);
        childItem->setText(0, tr("IFD0Offset"));
        childItem->setText(1, QString::number(tiff.ifd0Offset()));
    }

    // IfdItem
    foreach (const auto ifd, tiff.ifds())
        fillSubIfdItem(nullptr, ifd);
}

void MainWindow::updateActionRecentFiles()
{
    auto count = qMin(m_recentFiles.size(), static_cast<int>(MaxRecentFiles));
    for (int i = 0; i < count; ++i) {
        m_actionRecentFiles[i]->setText(QString("%1 %2").arg(i).arg(m_recentFiles[i]));
        m_actionRecentFiles[i]->setVisible(true);
    }
    for (int i = count; i < MaxRecentFiles; ++i)
        m_actionRecentFiles[i]->setVisible(false);

    m_actionSeparator->setVisible(count > 0);
}

void MainWindow::fillIfdEntryItem(QTreeWidgetItem *parentItem, const TiffIfdEntry &de)
{
    const auto tagName = de.tagName();
    QStringList valueStrings;
    foreach (auto v, de.values()) {
        auto valueString = v.toString();
        if (v.typeId() == QMetaType::QString) {
            valueString.replace(QLatin1String("\r"), QLatin1String("\\r"));
            valueString.replace(QLatin1String("\n"), QLatin1String("\\n"));
            valueString.replace(QLatin1String("\t"), QLatin1String("\\t"));
            valueString.replace(QLatin1String("\v"), QLatin1String("\\v"));
            valueString.replace(QLatin1String("\b"), QLatin1String("\\b"));
        }
        valueStrings.append(valueString);
    }
    auto valueString = valueStrings.join(" ");
    valueString =
        QFontMetricsF(ui->treeWidget->font()).elidedText(valueString, Qt::ElideRight, 400);

    auto deItem = new QTreeWidgetItem(parentItem);
    deItem->setText(0, tr("DE %1").arg(tagName));
    deItem->setText(1,
                    QString("Type=%2, Count=%3, Values=%4")
                        .arg(de.typeName())
                        .arg(de.count())
                        .arg(valueString));

    auto item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("Tag"));
    item->setText(1, QString("%1 %2").arg(tagName).arg(de.tag()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("DataType"));
    item->setText(1, QString("%1 %2").arg(de.typeName()).arg(de.type()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("Count"));
    item->setText(1, QString::number(de.count()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("ValueOrOffset"));
    item->setText(1, de.valueOrOffset().toHex(' '));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("Values"));
    item->setText(1, valueString);
    for (auto i = 0; i < valueStrings.size(); ++i) {
        auto valueItem = new QTreeWidgetItem(item);
        valueItem->setText(0, tr("Value[%1]").arg(i));
        valueItem->setText(1, valueStrings[i]);
    }
}

void MainWindow::fillSubIfdItem(QTreeWidgetItem *parentItem, const TiffIfd &ifd)
{
    auto ifdItem = new QTreeWidgetItem(parentItem);
    if (!parentItem)
        ui->treeWidget->addTopLevelItem(ifdItem);

    ifdItem->setText(0, tr("IFD"));
    ifdItem->setText(1, "");
    ifdItem->setExpanded(true);

    int width = -1;
    int height = -1;

    auto childItem = new QTreeWidgetItem(ifdItem);
    childItem->setText(0, tr("EntriesCount"));
    childItem->setText(1, QString::number(ifd.ifdEntries().size()));

    // ifd entity items
    foreach (const auto de, ifd.ifdEntries()) {
        fillIfdEntryItem(ifdItem, de);
        if (de.tag() == TiffIfdEntry::T_ImageWidth)
            width = de.values().value(0).toInt();
        if (de.tag() == TiffIfdEntry::T_ImageLength)
            height = de.values().value(0).toInt();
    }

    // sub ifd items
    foreach (const auto subIfd, ifd.subIfds())
        fillSubIfdItem(ifdItem, subIfd);

    childItem = new QTreeWidgetItem(ifdItem);
    childItem->setText(0, tr("NextIFDOffset"));
    childItem->setText(1, QString::number(ifd.nextIfdOffset()));

    if (width != -1 && height != -1)
        ifdItem->setText(1, tr("Image(%1x%2)").arg(width).arg(height));
}
