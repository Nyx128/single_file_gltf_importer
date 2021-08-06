#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "glm/glm.hpp"

using json = nlohmann::json;
//Khronos group makes the best stuff as always
std::string readFile(const char* path) {
	std::ifstream fileStream;
	std::stringstream textStream;
	fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		fileStream.open(path);
		textStream << fileStream.rdbuf();
		fileStream.close();
	}
	catch(std::ifstream::failure f){
		std::cout << "ERROR:can't read("<< path << ")" << std::endl;
	}

	return textStream.str();
}

//an attempt to get the position vectors of cub.gltf in just the main function
int main() {
	//get the binary
	std::string file = "C:/dev/json_demo/json_demo/models/cube.gltf";
	json JSON = json::parse(readFile(file.c_str()));
	std::string uri = JSON["buffers"][0]["uri"];
	std::string fileDir = file.substr(0, file.find_last_of('/') + 1);
	std::string bin = readFile((fileDir + uri).c_str());

	std::vector<unsigned char> data(bin.begin(), bin.end());
	//get the binary

	//get the floats
	unsigned int posInd = JSON["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
	json accessor = JSON["accessors"][posInd];
	std::vector<float> floatVec;

	unsigned int bufferViewInd = accessor.value("bufferView", 1); //returns 1 if bufferView doesnt have a value
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0); //returns 0 if bufferOffset doesnt have a value
	std::string type = accessor["type"];

	json bufferView = JSON["bufferViews"][bufferViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	unsigned int compPerVert;
	//get the type of the float
	if (type == "SCALAR") { compPerVert = 1; }
	else if (type == "VEC2") { compPerVert = 2; }
	else if (type == "VEC3") { compPerVert = 3; }
	else if (type == "VEC4") { compPerVert = 4; }
	else { throw std::invalid_argument("Type isn't valid it's not(SCALAR, VEC2, VEC3, VEC4)"); }

	unsigned int dataBegin = byteOffset + accByteOffset;
	unsigned int dataLength = count * sizeof(float) * compPerVert;//total byte length

	for (unsigned int i = dataBegin; i < (dataBegin + dataLength); i) {
		//a float has 4 bytes so we move by 1 byte each time and store it in a temp array
		unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++]};
		float val;
		std::memcpy(&val, bytes, sizeof(float));//convert them into floats
		floatVec.push_back(val); //and finally add it to out vector
	}
	//get the floats

	//group the vertex positions into glm::vec3
	std::vector<glm::vec3> vertexPositions;
	glm::vec3 pos = glm::vec3(0.0f);
	for (int i = 0; i < floatVec.size() / 3; i++) {
		pos.x = floatVec[i + 0];
		pos.y = floatVec[i + 1];
		pos.z = floatVec[i + 2];
		vertexPositions.push_back(pos);
	}
	std::cout << file << std::endl;
	for (glm::vec3 position : vertexPositions) {
		printf("(%f, %f, %f)\n", position.x, position.y, position.z);
	}
	//group the vertex positions into glm::vec3
}