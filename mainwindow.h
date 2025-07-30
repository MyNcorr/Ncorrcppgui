#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFileInfo>

#include "ncorr/Image2D.h"
#include "ncorr/ROI2D.h"
#include "ncorr/ncorr.h"
#include "ncorr/Data2D.h"
#include "ncorr/Strain2D.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionLoad_Reference_Image_triggered();
    void on_actionLoad_Current_Images_triggered();
    void on_actionSet_Reference_ROI_triggered();
    void on_actionExit_triggered();
    void on_actionSet_DIC_Parameters_triggered();
    void on_actionPerform_DIC_Analysis_triggered();
    void on_button_left_clicked();
    void on_button_right_clicked();
    void on_actionPlot_U_triggered();
    void on_actionPlot_V_triggered();
    void on_actionPlot_Exx_triggered();
    void on_actionPlot_Eyy_triggered();
    void on_actionPlot_Exy_triggered();

private:
    Ui::MainWindow *ui;

    ncorr::Image2D reference_image;
    std::vector<ncorr::Image2D> current_images;
    ncorr::ROI2D reference_roi;
    int current_image_index;
    int dic_subset_radius;
    int dic_subset_spacing;
    std::vector<ncorr::Data2D> analysis_data;

    void update_image_displays();
    void plot_data(const ncorr::Array2D<double>& data_to_plot, const QString& window_title);
};
#endif // MAINWINDOW_H
