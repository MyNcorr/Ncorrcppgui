#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dicparametersdialog.h" // Include the dialog header

#include <QMessageBox>
#include <QProgressDialog>
#include <QDebug>

// OpenCV header for displaying images
#include "opencv2/highgui.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , current_image_index(0)
    , dic_subset_radius(-1) // Initialize with an invalid value
    , dic_subset_spacing(-1)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//----------------------------------------------------------------//
// FILE AND ROI ACTIONS
//----------------------------------------------------------------//

void MainWindow::on_actionLoad_Reference_Image_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Reference Image", "", "Image Files (*.png *.jpg *.bmp *.tif)");
    if (!fileName.isEmpty()) {
        try {
            reference_image = ncorr::Image2D(fileName.toStdString());
            ui->text_refloaded_s->setText("SET");
            ui->text_refloaded_s->setStyleSheet("color: green; font-weight: bold;");
            ui->text_refname->setText("Name: " + QFileInfo(fileName).fileName());
            update_image_displays();
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString("Failed to load reference image: %1").arg(e.what()));
        }
    }
}

void MainWindow::on_actionLoad_Current_Images_triggered()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Open Current Images", "", "Image Files (*.png *.jpg *.bmp *.tif)");
    if (!files.isEmpty()) {
        current_images.clear();
        analysis_data.clear(); // Clear old analysis results
        for (const QString &file : files) {
            current_images.push_back(ncorr::Image2D(file.toStdString()));
        }
        current_image_index = 0;
        ui->text_curloaded_s->setText("SET");
        ui->text_curloaded_s->setStyleSheet("color: green; font-weight: bold;");
        update_image_displays();
    }
}

void MainWindow::on_actionSet_Reference_ROI_triggered()
{
    if (reference_image.get_gs().height() == 0) {
        QMessageBox::warning(this, "Warning", "Please load a reference image first.");
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Open ROI Image", "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        try {
            ncorr::Image2D roi_image(fileName.toStdString());
            reference_roi = ncorr::ROI2D(roi_image.get_gs() > 0.5);
            ncorr::Array2D<bool> mask = reference_roi.get_mask();
            QImage q_roi_image(mask.width(), mask.height(), QImage::Format_Mono);
            for (int y = 0; y < mask.height(); ++y) {
                for (int x = 0; x < mask.width(); ++x) {
                    q_roi_image.setPixel(x, y, mask(y, x) ? 1 : 0);
                }
            }
            ui->axes_roi->setPixmap(QPixmap::fromImage(q_roi_image).scaled(ui->axes_roi->size(), Qt::KeepAspectRatio));
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString("Failed to load ROI: %1").arg(e.what()));
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

//----------------------------------------------------------------//
// DIC PARAMETERS AND ANALYSIS
//----------------------------------------------------------------//

void MainWindow::on_actionSet_DIC_Parameters_triggered()
{
    DICParametersDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        this->dic_subset_radius = dialog.getSubsetRadius();
        this->dic_subset_spacing = dialog.getSubsetSpacing();
        QMessageBox::information(this, "Parameters Saved",
                                 QString("Subset Radius: %1\nSubset Spacing: %2").arg(this->dic_subset_radius).arg(this->dic_subset_spacing));
    }
}

void MainWindow::on_actionPerform_DIC_Analysis_triggered()
{
    if (reference_image.get_gs().height() == 0) {
        QMessageBox::critical(this, "Error", "Reference image not loaded.");
        return;
    }
    if (current_images.empty()) {
        QMessageBox::critical(this, "Error", "Current images not loaded.");
        return;
    }
    // CORRECTED: Check for ROI validity by checking the height of its mask.
    if (reference_roi.get_mask().height() == 0) {
        QMessageBox::critical(this, "Error", "Region of Interest (ROI) not set.");
        return;
    }
    if (dic_subset_radius <= 0 || dic_subset_spacing <= 0) {
        QMessageBox::critical(this, "Error", "DIC parameters not set.");
        return;
    }

    analysis_data.clear();
    analysis_data.resize(current_images.size());

    QProgressDialog progress("Performing DIC analysis...", "Cancel", 0, current_images.size(), this);
    progress.setWindowModality(Qt::WindowModal);

    try {
        for (size_t i = 0; i < current_images.size(); ++i) {
            progress.setValue(i);
            if (progress.wasCanceled()) break;

            // CORRECTED: The function is a simple free function, not a class.
            // Parameters are passed directly as arguments.
            analysis_data[i] = ncorr::ncorr_alg(reference_image,
                                                current_images[i],
                                                reference_roi,
                                                dic_subset_radius,
                                                dic_subset_spacing);
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Analysis Failed", e.what());
        progress.close();
        return;
    }

    progress.setValue(current_images.size());
    QMessageBox::information(this, "Success", "DIC analysis completed successfully!");
}


//----------------------------------------------------------------//
// PLOTTING ACTIONS
//----------------------------------------------------------------//

void MainWindow::plot_data(const ncorr::Array2D<double>& data_to_plot, const QString& window_title)
{
    if (data_to_plot.height() == 0) {
        QMessageBox::warning(this, "No Data", "No data to display. Please run the analysis first.");
        return;
    }

    // CORRECTED: The function to get a cv::Mat is a free function in the ncorr namespace.
    cv::Mat plot_mat = ncorr::get_cv_img(data_to_plot);

    cv::imshow(window_title.toStdString(), plot_mat);
}

void MainWindow::on_actionPlot_U_triggered()
{
    if (analysis_data.empty() || current_image_index >= analysis_data.size()) return;
    // CORRECTED: The result data is accessed via a getter function.
    plot_data(analysis_data[current_image_index].get_u(), "Displacement - U");
}

void MainWindow::on_actionPlot_V_triggered()
{
    if (analysis_data.empty() || current_image_index >= analysis_data.size()) return;
    // CORRECTED: The result data is accessed via a getter function.
    plot_data(analysis_data[current_image_index].get_v(), "Displacement - V");
}

void MainWindow::on_actionPlot_Exx_triggered()
{
    if (analysis_data.empty() || current_image_index >= analysis_data.size()) return;

    // CORRECTED: The strain analysis is a simple free function.
    ncorr::Strain2D strain_results = ncorr::strain_alg(analysis_data[current_image_index], 20); // Using default radius of 20

    // CORRECTED: Strain data is accessed via a getter function.
    plot_data(strain_results.get_exx(), "Strain - Exx");
}

void MainWindow::on_actionPlot_Eyy_triggered()
{
    if (analysis_data.empty() || current_image_index >= analysis_data.size()) return;
    ncorr::Strain2D strain_results = ncorr::strain_alg(analysis_data[current_image_index], 20);

    plot_data(strain_results.get_eyy(), "Strain - Eyy");
}

void MainWindow::on_actionPlot_Exy_triggered()
{
    if (analysis_data.empty() || current_image_index >= analysis_data.size()) return;
    ncorr::Strain2D strain_results = ncorr::strain_alg(analysis_data[current_image_index], 20);

    plot_data(strain_results.get_exy(), "Strain - Exy");
}

//----------------------------------------------------------------//
// UI UPDATE AND NAVIGATION
//----------------------------------------------------------------//

void MainWindow::update_image_displays()
{
    if (reference_image.get_gs().height() != 0) {
        QPixmap pix(QString::fromStdString(reference_image.get_filename()));
        ui->axes_ref->setPixmap(pix.scaled(ui->axes_ref->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (!current_images.empty()) {
        QPixmap pix(QString::fromStdString(current_images[current_image_index].get_filename()));
        ui->axes_cur->setPixmap(pix.scaled(ui->axes_cur->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->text_curname->setText("Name: " + QFileInfo(QString::fromStdString(current_images[current_image_index].get_filename())).fileName());
        ui->edit_imgnum->setText(QString::number(current_image_index + 1));
    }
}

void MainWindow::on_button_left_clicked()
{
    if (current_image_index > 0) {
        current_image_index--;
        update_image_displays();
    }
}

void MainWindow::on_button_right_clicked()
{
    if (current_image_index < static_cast<int>(current_images.size()) - 1) {
        current_image_index++;
        update_image_displays();
    }
}
