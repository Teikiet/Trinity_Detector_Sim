#ifndef F_FILEOPS_H
#define F_FILEOPS_H

#include<iostream>
#include<fstream>
#include<vector>

namespace IFHB{
	/** 
	 * \namespace FileOps
	 * \brief Utility algorithms used for obtaining contents from files
	 * 
	 * Files have diverse structures and using these tools facilitates access and retrieval
	 * 
	 * */
	namespace FileOps{

		/**
		 * 
		 * Find a specific token string in a file
		 * 
		 * @param fileName The name of the file to search in
		 * @param token The string to search for
		 * @param skipLines Optional in case lines should be skipped
		 * */
		inline int FindToken(std::string fileName, std::string token, int skipLines = 0){

			int lineNumber = skipLines;
			std::ifstream fileIn(fileName);
			std::string line;
			bool isFound = false;

			for(int i = 0; i<skipLines; i++){
				std::getline(fileIn, line);
			}

			while(std::getline(fileIn, line) && !isFound){
				if(line.find(token) == std::string::npos){
					lineNumber++;
				}else{
					isFound	= true;
				}
			}

			fileIn.close();

			return lineNumber;
		}

		/**
		 * 
		 * Retrieves the desired contents of the file
		 * 
		 * If the file is in column format, each line is read and pushed into a vector. It is designed for single column files. If the data is in horizontal format it tokenizes the line by white space and pushes it to a vector
		 * 
		 * @param fileName The file to retrieve data from
		 * @param orientation Optional parameter default is 'v' for Vertical but can be 'h' for Horizontal
		 * @param skipLines Optional parameter to skipLines from file
		 * */

		inline std::vector<double> RetrieveFileContent(std::string fileName,char orientation='v',int skipLines = 0){
			std::ifstream fileIn(fileName);
			std::string line;
			std::string token;
			std::string element;
			std::vector<double> content;
			for(int i = 0; i<skipLines; i++){
				std::getline(fileIn, line);
			}

			orientation	= (char)tolower(orientation);

			switch(orientation){
					case 'h':
						token = "*";
						std::getline(fileIn, line);
							if(line.find(token) == std::string::npos){
								std::istringstream iss(line);
								while(iss>>element){
									content.push_back(std::stod(element));
								}
							}else{
								content.push_back(-1.0);
							}
						break;
					case 'v':
						while(std::getline(fileIn, line)){
							content.push_back(std::stod(line));
						}
						break;
					default:
						std::cout<<"Wrong orientation given to RetrieveFileContent"<<std::endl;
						content.push_back(-1.0);
						break;
			}

			fileIn.close();
			
			return content;
		}
	}
}
#endif