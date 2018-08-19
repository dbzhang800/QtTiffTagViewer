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
#include "tifffile.h"
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>

/* QByteArray::toHex(char separator) is introduced in Qt5.9, but we need to support older versions.
 */
static QByteArray toHex(const QByteArray &bytes, char separator)
{
    const int length = separator ? (bytes.size() * 3 - 1) : (bytes.size() * 2);
    QByteArray hex(length, Qt::Uninitialized);
    char *hexData = hex.data();
    const uchar *data = reinterpret_cast<const uchar *>(bytes.data());
    for (int i = 0, o = 0; i < bytes.size(); ++i) {
        hexData[o++] = "0123456789ABCEDF"[data[i] >> 4];
        hexData[o++] = "0123456789ABCEDF"[data[i] & 0xf];

        if ((separator) && (o < length))
            hexData[o++] = separator;
    }
    return hex;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpenTriggered);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);

    loadSettings();
    if (qApp->arguments().size() > 1) {
        auto filePath = qApp->arguments().value(1);
        if (QFileInfo(filePath).exists())
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
    auto filePath = QFileDialog::getOpenFileName(this, tr("Open Tiff"), QString(),
                                                 tr("Tiff Image(*.tiff *.tif)"));
    if (filePath.isEmpty())
        return;

    doOpenTiffFile(filePath);
}

void MainWindow::onActionAboutTriggered()
{
    QString text = tr("<b>%1 %2</b><br/>"
                      "Copyright %3 Debao Zhang. All rights reserved.")
                       .arg(qApp->applicationName())
                       .arg(qApp->applicationVersion())
                       .arg(2018);

    QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()), text);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("geometry");
    restoreGeometry(settings.value("rect").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("geometry");
    settings.setValue("rect", saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

void MainWindow::doOpenTiffFile(const QString &filePath)
{
    TiffFile tiff(filePath);

    ui->treeWidget->clear();

    if (tiff.hasError()) {
        ui->logEdit->appendPlainText(
            QString("Fail to open the tiff file: %1 [%2]").arg(filePath).arg(tiff.errorString()));
        return;
    }

    // headeritem
    {
        auto headerItem = new QTreeWidgetItem(ui->treeWidget);
        headerItem->setText(0, tr("Header"));
        headerItem->setText(1, toHex(tiff.headerBytes(), ' '));
        headerItem->setExpanded(true);

        auto childItem = new QTreeWidgetItem(headerItem);

        childItem->setText(0, tr("ByteOrder"));
        childItem->setText(
            1, tiff.byteOrder() == TiffFile::BigEndian ? tr("BigEndian") : tr("LittleEndian"));

        childItem = new QTreeWidgetItem(headerItem);
        childItem->setText(0, tr("Version"));
        childItem->setText(
            1, QString("%1 %2").arg(tiff.version()).arg(tiff.isBigTiff() ? "BigTiff" : ""));

        childItem = new QTreeWidgetItem(headerItem);
        childItem->setText(0, tr("IFD0Offset"));
        childItem->setText(1, QString::number(tiff.ifd0Offset()));
    }

    // IfdItem
    {
        foreach (const auto ifd, tiff.ifds()) {
            auto ifdItem = new QTreeWidgetItem(ui->treeWidget);
            ifdItem->setText(0, tr("IFD"));
            ifdItem->setText(1, "");
            ifdItem->setExpanded(true);

            auto childItem = new QTreeWidgetItem(ifdItem);
            childItem->setText(0, tr("EntriesCount"));
            childItem->setText(1, QString::number(ifd.ifdEntries().size()));

            // ifd entity items
            foreach (const auto de, ifd.ifdEntries())
                fillIfdEntryItem(ifdItem, de);

            // sub ifd items
            foreach (const auto subIfd, ifd.subIfds())
                fillSubIfdItem(ifdItem, subIfd);

            childItem = new QTreeWidgetItem(ifdItem);
            childItem->setText(0, tr("NextIFDOffset"));
            childItem->setText(1, QString::number(ifd.nextIfdOffset()));
        }
    }
}

void MainWindow::fillIfdEntryItem(QTreeWidgetItem *parentItem, const TiffFileIfdEntry &de)
{
    auto deItem = new QTreeWidgetItem(parentItem);
    deItem->setText(0, tr("IFDEntry"));
    deItem->setText(1, QString("%1 %2 %3").arg(de.tag()).arg(de.type()).arg(de.count()));

    auto item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("Tag"));
    item->setText(1, QString::number(de.tag()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("DataType"));
    item->setText(1, QString::number(de.type()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("Count"));
    item->setText(1, QString::number(de.count()));

    item = new QTreeWidgetItem(deItem);
    item->setText(0, tr("ValueOrOffset"));
    item->setText(1, toHex(de.valueOrOffset(), ' '));
}

void MainWindow::fillSubIfdItem(QTreeWidgetItem *parentItem, const TiffFileIfd &ifd)
{
    auto ifdItem = new QTreeWidgetItem(parentItem);
    ifdItem->setText(0, tr("IFD"));
    ifdItem->setText(1, "");
    ifdItem->setExpanded(true);

    auto childItem = new QTreeWidgetItem(ifdItem);
    childItem->setText(0, tr("EntriesCount"));
    childItem->setText(1, QString::number(ifd.ifdEntries().size()));

    // ifd entity items
    foreach (const auto de, ifd.ifdEntries())
        fillIfdEntryItem(ifdItem, de);

    // sub ifd items
    foreach (const auto subIfd, ifd.subIfds())
        fillSubIfdItem(ifdItem, subIfd);

    childItem = new QTreeWidgetItem(ifdItem);
    childItem->setText(0, tr("NextIFDOffset"));
    childItem->setText(1, QString::number(ifd.nextIfdOffset()));
}
