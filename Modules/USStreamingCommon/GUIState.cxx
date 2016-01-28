#include "GUIState.h"

#include <iostream>
#include <fstream>
/// Helper class with the GUI state
namespace GUI {

State::State(){
		this->m_last_host = "192.168.2.2";
		this->m_save_all_images_on = false;
		this->m_enable_3D_on = true;
		this->m_last_label = -1; /// None (Default)
		this->m_last_perceived_force = 1; /// Medium
		this->m_transducer_selection = 0; /// Multiplexer off
		this->m_mesh_files.resize(0);
		this->m_surface_files.resize(0);
		this->m_transducer_matrix_file = "NULL";
		this->m_emtracker_matrix_file = "NULL";
		this->m_save_filename = "tmp";
		this->m_filename = "~/.DNLClient_config.txt";
		this->m_last_label_text = "Default";
		this->m_framerate = 25;
		this->m_imageBufferSize = 1;

    }

    // Copy constructor
State::State(const State &cSource)
    {
		this->m_last_host = cSource.m_last_host;
		this->m_save_all_images_on = cSource.m_save_all_images_on;
		this->m_enable_3D_on = cSource.m_enable_3D_on;
		this->m_last_label = cSource.m_last_label;
		this->m_last_perceived_force = cSource.m_last_perceived_force;
		this->m_transducer_selection = cSource.m_transducer_selection;
		this->m_mesh_files = cSource.m_mesh_files;
		this->m_surface_files = cSource.m_surface_files;
		this->m_transducer_matrix_file = cSource.m_transducer_matrix_file;
		this->m_emtracker_matrix_file = cSource.m_emtracker_matrix_file;
		this->m_save_filename = cSource.m_save_filename;
		this->m_filename = cSource.m_filename;
		this->m_last_label_text = cSource.m_last_label_text;
		this->m_framerate = cSource.m_framerate;
        this->m_imageBufferSize = cSource.m_imageBufferSize;
}

State& State::operator=(const State &cSource)
    {
		this->m_last_host = cSource.m_last_host;
		this->m_save_all_images_on = cSource.m_save_all_images_on;
		this->m_enable_3D_on = cSource.m_enable_3D_on;
		this->m_last_label = cSource.m_last_label;
		this->m_last_perceived_force = cSource.m_last_perceived_force;
		this->m_transducer_selection = cSource.m_transducer_selection;
		this->m_mesh_files = cSource.m_mesh_files;
		this->m_surface_files = cSource.m_surface_files;
		this->m_transducer_matrix_file = cSource.m_transducer_matrix_file;
		this->m_emtracker_matrix_file = cSource.m_emtracker_matrix_file;
		this->m_save_filename = cSource.m_save_filename;
		this->m_filename = cSource.m_filename;
		this->m_last_label_text = cSource.m_last_label_text;
		this->m_framerate = cSource.m_framerate;
        this->m_imageBufferSize = cSource.m_imageBufferSize;
		// return the existing object
		return *this;
    }

    State::~State(){};


    void State::writeToFile(std::string & config_file){

		std::ofstream cfo(config_file);
		cfo << "# DNL Client configuration file" << std::endl;
		cfo << "FILENAME\t" << this->m_filename << std::endl;
		for (int i = 0; i<this->m_mesh_files.size(); i++){
			cfo << "MESH_FILE\t" << this->m_mesh_files[i] << std::endl;
		}
		for (int i = 0; i<this->m_surface_files.size(); i++){
			cfo << "SURFACE_FILE\t" << this->m_surface_files[i] << std::endl;
		}
		cfo << "TRANSDUCER_FILE\t" << this->m_transducer_matrix_file << std::endl;
		cfo << "EMTACKER_CALIBRATION_FILE\t" << this->m_emtracker_matrix_file << std::endl;
		cfo << "HOST\t" << this->m_last_host << std::endl;
		cfo << "IMAGE_LABEL\t" << this->m_last_label << std::endl;
		cfo << "IMAGE_LABEL_TEXT\t" << this->m_last_label_text << std::endl;
		cfo << "PERCEIVED_FORCE\t" << this->m_last_perceived_force << std::endl;
		cfo << "IMAGE_SAVE_FILENAME\t" << this->m_save_filename << std::endl;
		cfo << "TRANSDUCER_SELECTION\t" << this->m_transducer_selection << std::endl;
		cfo << "SAVE_ALL_IMAGES\t" << this->m_save_all_images_on << std::endl;
		cfo << "ENABLE_3D\t" << this->m_enable_3D_on << std::endl;
		cfo << "FRAMERATE\t" << this->m_framerate << std::endl;
		cfo << "IMAGE_BUFFER_SIZE\t" << this->m_imageBufferSize  << std::endl;
		cfo.close();

    }


    void State::readFromFile(std::string & config_file){

		std::ifstream cfi(config_file);
		std::string line;
		std::getline(cfi, line);
		while (true){
			std::string dummy, name;
			cfi >> dummy >> name;
			if (dummy.size() == 0)
				break;
			if (dummy.compare("MESH_FILE") == 0){
				//if (debug) std::cout << "\tAdd mesh "<<name<<std::endl;
				this->m_mesh_files.push_back(name);
			}
			else if (dummy.compare("SURFACE_FILE") == 0){
				//if (debug) std::cout << "\tAdd surface "<<name<<std::endl;
				this->m_surface_files.push_back(name);
			}
			else if (dummy.compare("TRANSDUCER_FILE") == 0){
				//if (debug) std::cout << "\tAdd transducer file "<<name<<std::endl;
				this->m_transducer_matrix_file = name;
			}
			else if (dummy.compare("EMTACKER_CALIBRATION_FILE") == 0){
				//if (debug) std::cout << "\tAdd EM tracker calibration file "<<name<<std::endl;
				this->m_emtracker_matrix_file = name;
			}
			else if (dummy.compare("HOST") == 0){
				this->m_last_host = name;
			}
			else if (dummy.compare("IMAGE_LABEL") == 0){
				this->m_last_label = atoi(name.data());
			}
			else if (dummy.compare("IMAGE_LABEL_TEXT") == 0){
				this->m_last_label_text = name;
			}
			else if (dummy.compare("PERCEIVED_FORCE") == 0){
				this->m_last_perceived_force = atoi(name.data());
			}
			else if (dummy.compare("IMAGE_SAVE_FILENAME") == 0){
				this->m_save_filename = name;
			}
			else if (dummy.compare("TRANSDUCER_SELECTION") == 0){
				this->m_transducer_selection = atoi(name.data());
			}
			else if (dummy.compare("SAVE_ALL_IMAGES") == 0){
				this->m_save_all_images_on = atoi(name.data());
			}
			else if (dummy.compare("ENABLE_3D") == 0){
				this->m_enable_3D_on = atoi(name.data());
			}
			else if (dummy.compare("FRAMERATE") == 0){
				this->m_framerate = atof(name.data());
			}
			else if (dummy.compare("IMAGE_BUFFER_SIZE") == 0){
				this->m_imageBufferSize = atoi(name.data());
			}			

			this->m_filename = config_file;


		}
		cfi.close();

    }


}
