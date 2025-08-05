#include "dicparametersdialog.h"
#include "ui_dicparametersdialog.h"

DICParametersDialog::DICParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DICParametersDialog)
{
    ui->setupUi(this);
    // Connect the buttonBox signals to the dialog's slots
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DICParametersDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DICParametersDialog::reject);
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
