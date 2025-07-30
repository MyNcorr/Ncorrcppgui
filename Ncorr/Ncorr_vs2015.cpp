#include "ncorr.h"

using namespace ncorr;

int main(int argc, char *argv[]) {

	// =================================================================================
	// Part 1: Common Setup
	// =================================================================================

	// Note: Update this root path to where your images are located.
	auto root = std::string("..\\..\\test\\images\\");

	// Load images based on your data: S2_1.jpg is the reference, and S2_2 to S2_10 are current.
	std::vector<Image2D> imgs;
	imgs.push_back(root + std::string("S2_1.jpg"));
	imgs.push_back(root + std::string("S2_3.jpg"));


	// Assuming 'roi.png' is the correct ROI mask for your analysis.
	auto roiImg = Image2D(root + std::string("roi.png")).get_gs();
	auto roi = ROI2D(roiImg > 0.5);

	// Set DIC_input with parameters from your images.
	// Using a reasonable cutoff_corrcoef to filter noise for a smoother plot.
	DIC_analysis_input DIC_input = DIC_analysis_input(imgs,      // Images
		roi,		                                                // ROI
		1,                                                      // scalefactor (spacing)
		INTERP::QUINTIC_BSPLINE_PRECOMPUTE,			            // Interpolation
		SUBREGION::CIRCLE,					                    // Subregion shape
		30,                                         		    // Subregion radius
		1,                                          		    // # of threads
		0.7,							                        // cutoff_corrcoef -> Balanced value to match smooth plot
		0.5,							                        // update_corrcoef (default for high strain)
		1.0,							                        // prctile_corrcoef (default for high strain)
		true);							                        // Debugging enabled/disabled

	// Perform DIC_analysis    
	DIC_analysis_output DIC_output_lagrangian = DIC_analysis(DIC_input);

	// Set units of DIC_output (provide units/pixel)
	DIC_output_lagrangian = set_units(DIC_output_lagrangian, "mm", 0.12889);


	// =================================================================================
	// Part 2: Lagrangian Analysis (Matches R10.jpg)
	// =================================================================================

	strain_analysis_input strain_input_lagrangian = strain_analysis_input(DIC_input,
		DIC_output_lagrangian,
		SUBREGION::CIRCLE,					// Strain subregion shape
		15);						// Strain subregion radius

	strain_analysis_output strain_output_lagrangian = strain_analysis(strain_input_lagrangian);

	// Save Lagrangian Exx strain video
	save_strain_video("../../test/video/Exx_Green_Lagrangian.avi",
		strain_input_lagrangian,
		strain_output_lagrangian,
		STRAIN::EXX,
		0.75,		// Alpha
		15,         // FPS
		-0.0018,    // Min strain value for colorbar (from your screenshot)
		0.3787,     // Max strain value for colorbar (from your screenshot)
		true,       // Enable colorbar
		true,       // Enable axes
		true,       // Enable scalebar
		185.0); 	// Scalebar length in units


	// =================================================================================
	// Part 3: Eulerian Analysis (Matches Euler.JPG)
	// =================================================================================

	// Convert Lagrangian DIC_output to Eulerian perspective
	DIC_analysis_output DIC_output_eulerian = change_perspective(DIC_output_lagrangian, INTERP::QUINTIC_BSPLINE_PRECOMPUTE);

	// Set new strain input with the Eulerian output
	strain_analysis_input strain_input_eulerian = strain_analysis_input(DIC_input,
		DIC_output_eulerian,
		SUBREGION::CIRCLE,					// Strain subregion shape
		15);						// Strain subregion radius

	strain_analysis_output strain_output_eulerian = strain_analysis(strain_input_eulerian);

	// Save Eulerian Exx strain video
	save_strain_video("../../test/video/Exx_Eulerian_Almansi.avi",
		strain_input_eulerian,
		strain_output_eulerian,
		STRAIN::EXX,
		0.75,		// Alpha
		15,         // FPS
		-0.0069,    // Min strain value for colorbar (from your screenshot)
		0.3134,     // Max strain value for colorbar (from your screenshot)
		true,       // Enable colorbar
		true,       // Enable axes
		true,       // Enable scalebar
		185.0); 	// Scalebar length in units

	return 0;
}