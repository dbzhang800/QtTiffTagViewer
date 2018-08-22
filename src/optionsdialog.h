/**************************************************************************
**
** Copyright (C) 2018 Focus e-Beam Technology (Beijing) Co.,Ltd.
** Contact: contact@focus-ebeam.com
**
** This file is part of <......>.
**
** This source code is confidential information and can be used only as
** authorized by a licensing agreement from FBT.
**
**************************************************************************/
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "tifffile.h"
#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog();

    TiffParserOptions parserOptions() const;
    void setParserOptions(const TiffParserOptions &options);

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
