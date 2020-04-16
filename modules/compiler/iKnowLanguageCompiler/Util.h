#include <fstream>
#include <iostream>
#include <string>

namespace iknow {
	namespace AHO {
		class Util {

		};
		class OutputToFile {
		public:
			OutputToFile(std::string dir_name, std::string file_name) : open_for_write(false) {
				ofs_ = std::ofstream(dir_name + "/" + file_name, std::ofstream::out); // Do ##class(Util).OutputToFile(dir _ "/OneStateMap.inl")
				if (ofs_.is_open()) open_for_write = true;
				else {
					std::cerr << "Cannot open " << dir_name << "/" << file_name << " for writing !" << std::endl;
				}
			}
			~OutputToFile() {
				if (open_for_write)
					ofs_.close();
			}
			std::ofstream& o() {
				return ofs_;
			}
			bool IsOpen() {
				return open_for_write;
			}
		private:
			std::ofstream ofs_;
			bool open_for_write;
		};
	}
}