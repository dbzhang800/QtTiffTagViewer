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
#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

TiffParserOptions OptionsDialog::parserOptions() const
{
    TiffParserOptions options;
    options.parserSubIfds = ui->parser_subIfds_button->isChecked();
    return options;
}

void OptionsDialog::setParserOptions(const TiffParserOptions &options)
{
    ui->parser_subIfds_button->setChecked(options.parserSubIfds);
}
