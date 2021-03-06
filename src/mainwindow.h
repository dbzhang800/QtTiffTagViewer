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
#pragma once

#include "tifffile.h"
#include <QMainWindow>

class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *evt);

private:
    void onActionOpenTriggered();
    void onActionOptionsTriggered();
    void onActionAboutTriggered();
    void onActionRecentFileTriggered();

    void loadSettings();
    void saveSettings();
    void doOpenTiffFile(const QString &filePath);
    void updateActionRecentFiles();

    void fillIfdEntryItem(QTreeWidgetItem *parentItem, const TiffIfdEntry &de);
    void fillSubIfdItem(QTreeWidgetItem *parentItem, const TiffIfd &ifd);

    Ui::MainWindow *ui;

    TiffParserOptions m_parserOptions;

    enum { MaxRecentFiles = 10 };
    QAction *m_actionRecentFiles[MaxRecentFiles];
    QAction *m_actionSeparator;
    QStringList m_recentFiles;
};
