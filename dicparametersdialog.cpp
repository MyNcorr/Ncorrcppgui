#include "dicparametersdialog.h"
#include "ui_dicparametersdialog.h"

DICParametersDialog::DICParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DICParametersDialog)
{
    ui->setupUi(this);
}

DICParametersDialog::~DICParametersDialog()
{
    delete ui;
}

int DICParametersDialog::getSubsetRadius() const
{
    return ui->spin_radius->value();
}

int DICParametersDialog::getSubsetSpacing() const
{
    return ui->spin_spacing->value();
}