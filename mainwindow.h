#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <memory> // Required for std::unique_ptr

// Ncorr includes - Include full definitions here to solve incomplete type errors
#include "Image2D.h"
#include "ROI2D.h"
#include "ncorr.h" // This includes all other necessary ncorr headers

// Forward declaration for the UI class
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward declaration for cv::Mat
namespace cv {
class Mat;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File Menu Actions
    void on_actionLoad_Reference_Image_triggered();
    void on_actionLoad_Current_Images_triggered();
    void on_actionExit_triggered();

    // ROI Menu Actions
    void on_actionSet_Reference_ROI_triggered();

    // Analysis Menu Actions
    void on_actionSet_DIC_Parameters_triggered();
    void on_actionPerform_DIC_Analysis_triggered();

    // Plot Menu Actions
    void on_actionPlot_U_triggered();
    void on_actionPlot_V_triggered();
    void on_actionPlot_Exx_triggered();
    void on_actionPlot_Eyy_triggered();
    void on_actionPlot_Exy_triggered();

    // Navigation Buttons
    void on_button_left_clicked();
    void on_button_right_clicked();
    void on_edit_imgnum_returnPressed();

private:
    // UI object
    Ui::MainWindow *ui;

    // Ncorr related data members
    ncorr::Image2D reference_image;
    std::vector<ncorr::Image2D> current_images;
    ncorr::ROI2D reference_roi;

    // DIC parameters
    int dic_subset_radius;
    int dic_subset_spacing;

    // Analysis results held by smart pointers for better memory management
    std::unique_ptr<ncorr::DIC_analysis_output> dic_output;
    std::unique_ptr<ncorr::strain_analysis_output> strain_output;
    bool analysis_completed;

    // UI state
    int current_image_index;

    // Helper functions
    void update_image_displays();
    void update_status_labels();
    void plot_data(const ncorr::Data2D& data_to_plot, const QString& window_title);
    cv::Mat array2d_to_cvmat(const ncorr::Array2D<double>& array);
    QPixmap mat_to_qpixmap(const cv::Mat& mat);
};

#endif // MAINWINDOW_H
