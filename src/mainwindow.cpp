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
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

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
}
