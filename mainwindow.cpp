#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dicparametersdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFileInfo>
#include <QProgressDialog>
#include <QDebug>

#include <thread>
#include <algorithm>

// OpenCV includes needed for image conversion and display
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , current_image_index(0)
    , dic_subset_radius(-1)
    , dic_subset_spacing(-1)
    , analysis_completed(false)
{
    ui->setupUi(this);
    update_status_labels();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//----------------------------------------------------------------//
// HELPER FUNCTIONS
//----------------------------------------------------------------//

// This function correctly converts Ncorr's custom array to an OpenCV Mat
cv::Mat MainWindow::array2d_to_cvmat(const ncorr::Array2D<double>& array)
{
    // The min/max helpers in ncorr require two arguments for ROI-aware
    // searches and break when `Array2D<bool>` is supplied.  Compute the
    // minimum and maximum directly to keep this routine generic.
    if (array.empty()) {
        return cv::Mat();
    }

    cv::Mat mat(array.height(), array.width(), CV_8UC1);

    const double *begin = array.get_pointer();
    const double *end   = begin + array.size();
    auto minmax = std::minmax_element(begin, end);
    double min_val = *minmax.first;
    double max_val = *minmax.second;

    double scale = 255.0;
    if (max_val > min_val) {
        scale = 255.0 / (max_val - min_val);
    }

    for (int r = 0; r < array.height(); ++r) {
        for (int c = 0; c < array.width(); ++c) {
            mat.at<uchar>(r, c) =
                static_cast<uchar>((array(r, c) - min_val) * scale);
        }
    }
    return mat;
}


QPixmap MainWindow::mat_to_qpixmap(const cv::Mat& mat) {
    if (mat.empty()) return QPixmap();
    if(mat.type() == CV_8UC1) {
        // Create QImage from the grayscale Mat
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return QPixmap::fromImage(image.copy()); // Use copy to avoid issues with data ownership
    } else if(mat.type() == CV_8UC3) {
        // For color images, swap Red and Blue channels for Qt
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);
        return QPixmap::fromImage(image.rgbSwapped());
    }
    return QPixmap();
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
            ui->text_refname->setText("Name: " + QFileInfo(fileName).fileName());
            analysis_completed = false; // Reset analysis state
            update_status_labels();
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
        analysis_completed = false; // Reset analysis state
        try {
            for (const QString &file : files) {
                current_images.push_back(ncorr::Image2D(file.toStdString()));
            }
            current_image_index = 0;
            update_status_labels();
            update_image_displays();
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString("Failed to load current images: %1").arg(e.what()));
        }
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

            // Display ROI
            ncorr::Array2D<bool> mask = reference_roi.get_mask();
            QImage q_roi_image(mask.width(), mask.height(), QImage::Format_Mono);
            for (int r = 0; r < mask.height(); ++r) {
                for (int c = 0; c < mask.width(); ++c) {
                    q_roi_image.setPixel(c, r, mask(r, c) ? 1 : 0);
                }
            }
            ui->axes_roi->setPixmap(QPixmap::fromImage(q_roi_image).scaled(ui->axes_roi->size(), Qt::KeepAspectRatio));
            update_status_labels();
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
        update_status_labels();
    }
}

void MainWindow::on_actionPerform_DIC_Analysis_triggered()
{
    if (reference_image.get_gs().height() == 0 || current_images.empty() || reference_roi.get_mask().height() == 0 || dic_subset_radius <= 0) {
        QMessageBox::critical(this, "Error", "Please load all images, set the ROI, and set DIC parameters before analysis.");
        return;
    }

    analysis_completed = false;
    dic_output.reset();
    strain_output.reset();

    std::vector<ncorr::Image2D> all_images;
    all_images.push_back(reference_image);
    all_images.insert(all_images.end(), current_images.begin(), current_images.end());

    QProgressDialog progress("Performing DIC analysis...", "Cancel", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);


    try {
        ncorr::DIC_analysis_input dic_input(
            all_images,
            reference_roi,
            dic_subset_spacing,
            ncorr::INTERP::QUINTIC_BSPLINE_PRECOMPUTE,
            ncorr::SUBREGION::CIRCLE,
            dic_subset_radius,
            std::thread::hardware_concurrency(), // Use available hardware threads
            ncorr::DIC_analysis_config::NO_UPDATE,
            false // Debug
            );

        progress.setLabelText("Running Ncorr DIC Analysis...");
        progress.setValue(10);
        QApplication::processEvents();

        dic_output.reset(new ncorr::DIC_analysis_output(ncorr::DIC_analysis(dic_input)));

        analysis_completed = true;

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Analysis Failed", e.what());
        progress.close();
        update_status_labels();
        return;
    }

    progress.setValue(100);
    update_status_labels();
    QMessageBox::information(this, "Success", "DIC analysis completed successfully!");
}

//----------------------------------------------------------------//
// PLOTTING ACTIONS
//----------------------------------------------------------------//

void MainWindow::plot_data(const ncorr::Data2D& data_to_plot, const QString& window_title)
{
    if (data_to_plot.data_height() == 0) {
        QMessageBox::warning(this, "No Data", "No data available to display.");
        return;
    }
    // Using namedWindow to avoid conflicts with other windows
    cv::destroyWindow(window_title.toStdString());
    ncorr::imshow(data_to_plot);
    cv::setWindowTitle(window_title.toStdString(), window_title.toStdString());
}

void MainWindow::on_actionPlot_U_triggered()
{
    if (!analysis_completed || !dic_output || current_image_index >= static_cast<int>(dic_output->disps.size())) return;
    plot_data(dic_output->disps[current_image_index].get_u(), "Displacement - U");
}

void MainWindow::on_actionPlot_V_triggered()
{
    if (!analysis_completed || !dic_output || current_image_index >= static_cast<int>(dic_output->disps.size())) return;
    plot_data(dic_output->disps[current_image_index].get_v(), "Displacement - V");
}

void MainWindow::on_actionPlot_Exx_triggered()
{
    if (!analysis_completed || !dic_output) return;
    if (!strain_output) {
        // Lazily compute strain if not already done
        ncorr::DIC_analysis_input dummy_input; // Dummy input, not used by strain analysis
        ncorr::strain_analysis_input strain_input(dummy_input, *dic_output, ncorr::SUBREGION::CIRCLE, 5);
        strain_output.reset(new ncorr::strain_analysis_output(ncorr::strain_analysis(strain_input)));
    }

    if (current_image_index < static_cast<int>(strain_output->strains.size())) {
        plot_data(strain_output->strains[current_image_index].get_exx(), "Strain - Exx");
    }
}

void MainWindow::on_actionPlot_Eyy_triggered()
{
    if (!analysis_completed || !dic_output) return;
    if (!strain_output) {
        ncorr::DIC_analysis_input dummy_input;
        ncorr::strain_analysis_input strain_input(dummy_input, *dic_output, ncorr::SUBREGION::CIRCLE, 5);
        strain_output.reset(new ncorr::strain_analysis_output(ncorr::strain_analysis(strain_input)));
    }

    if (current_image_index < static_cast<int>(strain_output->strains.size())) {
        plot_data(strain_output->strains[current_image_index].get_eyy(), "Strain - Eyy");
    }
}

void MainWindow::on_actionPlot_Exy_triggered()
{
    if (!analysis_completed || !dic_output) return;
    if (!strain_output) {
        ncorr::DIC_analysis_input dummy_input;
        ncorr::strain_analysis_input strain_input(dummy_input, *dic_output, ncorr::SUBREGION::CIRCLE, 5);
        strain_output.reset(new ncorr::strain_analysis_output(ncorr::strain_analysis(strain_input)));
    }

    if (current_image_index < static_cast<int>(strain_output->strains.size())) {
        plot_data(strain_output->strains[current_image_index].get_exy(), "Strain - Exy");
    }
}

//----------------------------------------------------------------//
// UI UPDATE AND NAVIGATION
//----------------------------------------------------------------//

void MainWindow::update_image_displays()
{
    if (reference_image.get_gs().height() != 0) {
        cv::Mat ref_mat = array2d_to_cvmat(reference_image.get_gs());
        ui->axes_ref->setPixmap(mat_to_qpixmap(ref_mat).scaled(ui->axes_ref->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->axes_ref->clear();
    }

    if (!current_images.empty()) {
        cv::Mat cur_mat = array2d_to_cvmat(current_images[current_image_index].get_gs());
        ui->axes_cur->setPixmap(mat_to_qpixmap(cur_mat).scaled(ui->axes_cur->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->text_curname->setText("Name: " + QFileInfo(QString::fromStdString(current_images[current_image_index].get_filename())).fileName());
        ui->edit_imgnum->setText(QString::number(current_image_index + 1));
    } else {
        ui->axes_cur->clear();
        ui->text_curname->setText("Name:");
        ui->edit_imgnum->setText("");
    }
}

void MainWindow::update_status_labels()
{
    // A lambda to reduce code duplication for setting label styles
    auto set_status = [](QLabel* label, bool condition) {
        if (condition) {
            label->setText("SET");
            label->setStyleSheet("color: green; font-weight: bold;");
        } else {
            label->setText("NOT SET");
            label->setStyleSheet("color: red; font-weight: normal;");
        }
    };

    set_status(ui->status_ref_loaded, reference_image.get_gs().height() != 0);
    set_status(ui->status_cur_loaded, !current_images.empty());
    set_status(ui->status_roi_loaded, reference_roi.get_mask().height() != 0);
    set_status(ui->status_dic_params_loaded, dic_subset_radius > 0);
    set_status(ui->status_dic_analysis_loaded, analysis_completed);
    set_status(ui->status_disp_loaded, analysis_completed);
    set_status(ui->status_strain_loaded, strain_output != nullptr);
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

void MainWindow::on_edit_imgnum_returnPressed()
{
    bool ok;
    int new_index = ui->edit_imgnum->text().toInt(&ok);
    if (ok && new_index > 0 && new_index <= static_cast<int>(current_images.size())) {
        current_image_index = new_index - 1;
        update_image_displays();
    } else {
        // Reset to current index if input is invalid
        ui->edit_imgnum->setText(QString::number(current_image_index + 1));
    }
}
