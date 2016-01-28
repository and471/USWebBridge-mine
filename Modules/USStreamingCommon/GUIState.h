#ifndef GUISTATE_H_
#define GUISTATE_H_
#include <string>
#include <vector>

namespace GUI {

	class State {
	public:
		State();
		// Copy constructor
		State(const State &cSource);
		State& operator=(const State &cSource);
		~State();


		void writeToFile(std::string & config_file);
		void readFromFile(std::string & config_file);


		std::string m_last_host;
		bool m_save_all_images_on;
		bool m_enable_3D_on;
		int m_last_label;
		std::string m_last_label_text;
		int m_last_perceived_force;
		int m_transducer_selection;
		int m_imageBufferSize;
		double m_framerate;

		std::vector<std::string> m_mesh_files;
		std::vector<std::string> m_surface_files;
		std::string m_transducer_matrix_file;
		std::string m_emtracker_matrix_file;

		std::string m_save_filename;
		std::string m_filename; // name of the config file

	};

}
#endif // GUISTATE_H_