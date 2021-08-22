#pragma once

#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "glm/glm.hpp"

using json = nlohmann::json;
//Khronos group makes the best stuff as always
enum dataType{POSITION, NORMAL, TEXCOORD};

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

std::vector<float> getFloats(json JSON, std::vector<unsigned char> data, dataType attrib, int meshInd) {
	unsigned int posInd;
	switch (attrib) {
	case dataType::POSITION:
		posInd = JSON["meshes"][meshInd]["primitives"][0]["attributes"]["POSITION"];
		break;

	case dataType::NORMAL:
		posInd = JSON["meshes"][meshInd]["primitives"][0]["attributes"]["NORMAL"];
		break;

	case dataType::TEXCOORD:
		posInd = JSON["meshes"][meshInd]["primitives"][0]["attributes"]["TEXCOORD_0"];
		break;

	default:
		throw std::invalid_argument("idk how did this happen, you must be proud to get this message");
	}
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
	//if its a VEC2 its 8, if its a VEC3 its 12 and so on

	for (unsigned int i = dataBegin; i < (dataBegin + dataLength); i) {
		//a float has 4 bytes so we move by 1 byte each time and store it in a temp array
		unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++] };
		float val;
		std::memcpy(&val, bytes, sizeof(float));//convert them into floats
		floatVec.push_back(val); //and finally add it to out vector
	}
	
	return floatVec;
}

std::vector<unsigned int>getIndices(json JSON, std::vector<unsigned char> data) {
	unsigned int indiceInd;
	indiceInd = JSON["meshes"][0]["primitives"][0]["indices"];

	json accessor = JSON["accessors"][indiceInd];

	std::vector<unsigned int> indices;

	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor["count"];
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor["componentType"];

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView["byteOffset"];

	// Get indices with regards to their type: unsigned int, unsigned short, or short
	unsigned int beginningOfData = byteOffset + accByteOffset;
	if (componentType == 5125)
	{
		for (unsigned int i = beginningOfData; i < beginningOfData + count * 4; i)
		{
			unsigned char bytes[] = { data[i++], data[i++], data[i++], data[i++] }; //4 bytes(32-bits)
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((unsigned int)value);
		}
	}
	else if (componentType == 5123)
	{
		for (unsigned int i = beginningOfData; i < beginningOfData + count * 2; i)
		{
			unsigned char bytes[] = { data[i++], data[i++] };
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short)); //2 bytes(16-bits)
			indices.push_back((unsigned int)value);
		}
	}
	else if (componentType == 5122)
	{
		for (unsigned int i = beginningOfData; i < beginningOfData + count * 2; i)
		{
			unsigned char bytes[] = { data[i++], data[i++] };
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((unsigned int)value);
		}
	}

	return indices;

}

std::string getTextures(json JSON, std::string fileDir, int texInd) {
	std::string texUri = JSON["images"][texInd]["uri"];

	return fileDir + texUri;
}

std::vector<glm::vec3> groupVec3(std::vector<float> floatVec) {
	std::vector<glm::vec3> vectors;
	for (int i = 0; i < floatVec.size(); i) {
		vectors.push_back(glm::vec3(floatVec[i++], floatVec[i++], floatVec[i++]));
	}

	return vectors;
}

std::vector<glm::vec2> groupVec2(std::vector<float> floatVec) {
	std::vector<glm::vec2> vectors;
	for (int i = 0; i < floatVec.size(); i) {
		vectors.push_back(glm::vec2(floatVec[i++], floatVec[i++]));
	}

	return vectors;
}

//an attempt to get the position, normal and texCoord vectors of cub.gltf in just one file(given its a single mesh)
int main() {
	//get the binary
	std::string file = "C:/dev/json_demo/json_demo/models/textured_plane/textured_plane.gltf";
	json JSON = json::parse(readFile(file.c_str()));
	std::string uri = JSON["buffers"][0]["uri"];
	std::string fileDir = file.substr(0, file.find_last_of('/') + 1);
	std::string bin = readFile((fileDir + uri).c_str());

	std::vector<unsigned char> data(bin.begin(), bin.end());

	std::vector<float> posFloats = getFloats(JSON, data, dataType::POSITION, 0);
	std::vector<glm::vec3> vertPos = groupVec3(posFloats);

	std::vector<float> normalFloats = getFloats(JSON, data, dataType::NORMAL, 0);
	std::vector<glm::vec3> normals = groupVec3(normalFloats);

	std::vector<unsigned int> indices = getIndices(JSON, data);

	std::vector<float> texFloats = getFloats(JSON, data, dataType::TEXCOORD, 0);
	std::vector<glm::vec2> texCoords = groupVec2(texFloats);
	
 }
