#ifndef DICPARAMETERSDIALOG_H
#define DICPARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class DICParametersDialog;
}

class DICParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DICParametersDialog(QWidget *parent = nullptr);
    ~DICParametersDialog();

    // Public methods to get the values
    int getSubsetRadius() const;
    int getSubsetSpacing() const;

private:
    Ui::DICParametersDialog *ui;
};

#endif // DICPARAMETERSDIALOG_H
