#include "ncorr.h"

using namespace ncorr;

int main(int argc, char *argv[]) {

	DIC_analysis_input DIC_input;
	DIC_analysis_output DIC_output;
	strain_analysis_input strain_input;
	strain_analysis_output strain_output;

	auto root = std::string("..\\..\\test\\images\\");

	std::vector<Image2D> imgs;
	imgs.push_back(root + std::string("ohtcfrp_00.png"));
	imgs.push_back(root + std::string("ohtcfrp_11.png"));

	auto roiImg = Image2D(root + std::string("roi.png")).get_gs();
	auto roi = ROI2D(roiImg > 0.5);

	// Set DIC_input
	DIC_input = DIC_analysis_input(imgs, 							// Images
		roi,		// ROI
		3,                                         		// scalefactor
		INTERP::QUINTIC_BSPLINE_PRECOMPUTE,			// Interpolation
		SUBREGION::CIRCLE,					// Subregion shape
		20,                                        		// Subregion radius
		4,                                         		// # of threads
		DIC_analysis_config::NO_UPDATE,				// DIC configuration for reference image updates
		true);							// Debugging enabled/disabled

										// Perform DIC_analysis    
	DIC_output = DIC_analysis(DIC_input);

	// Convert DIC_output to Eulerian perspective
	DIC_output = change_perspective(DIC_output, INTERP::QUINTIC_BSPLINE_PRECOMPUTE);

	// Set units of DIC_output (provide units/pixel)
	DIC_output = set_units(DIC_output, "mm", 0.2);

	// Set strain input
	strain_input = strain_analysis_input(DIC_input,
		DIC_output,
		SUBREGION::CIRCLE,					// Strain subregion shape
		5);						// Strain subregion radius

	strain_output = strain_analysis(strain_input);

	save(DIC_input, "../../test/save/DIC_input.bin");
	save(DIC_output, "../../test/save/DIC_output.bin");
	save(strain_input, "../../test/save/strain_input.bin");
	save(strain_output, "../../test/save/strain_output.bin");


	save_DIC_video("../../test/video/test_v_eulerian.avi",
		DIC_input,
		DIC_output,
		DISP::V,
		0.5,		// Alpha		
		15);		// FPS

	save_DIC_video("../../test/video/test_u_eulerian.avi",
		DIC_input,
		DIC_output,
		DISP::U,
		0.5,		// Alpha
		15);		// FPS

	save_strain_video("../../test/video/test_eyy_eulerian.avi",
		strain_input,
		strain_output,
		STRAIN::EYY,
		0.5,		// Alpha
		15);		// FPS

	save_strain_video("../../test/video/test_exy_eulerian.avi",
		strain_input,
		strain_output,
		STRAIN::EXY,
		0.5,		// Alpha
		15);		// FPS

	save_strain_video("../../test/video/test_exx_eulerian.avi",
		strain_input,
		strain_output,
		STRAIN::EXX,
		0.5,		// Alpha
		15); 		// FPS

	return 0;
}