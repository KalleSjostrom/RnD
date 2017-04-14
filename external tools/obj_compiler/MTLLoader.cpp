/*
 * MTLLoader.cpp
 *
 *  Created on: Jan 28, 2010
 *      Author: Fnatte
 */

#include "MTLLoader.h"

MTLLoader::MTLLoader() {
	// TODO Auto-generated constructor stub

}

MTLLoader::~MTLLoader() {
	// TODO Auto-generated destructor stub
}

map<string, State*>* MTLLoader::readFile(string fileName) {
	ifstream indata;
	string buf;
	map<string, State*>* matMap = new map<string, State*>();
	float x, y, z;
	float temp;
	int illum;
	string s("objs/");
	s.append(fileName);

	cout << "Trying to open file " << s << endl;
    indata.open(s.c_str());

	if(!indata) {
		cerr << "Error: file could not be opened" << endl;
		exit(-1);
	}

	State* currentS = NULL;
	Material* currentM = NULL;
	string line;
	string name;
	while (getline(indata, line)) {
		if (line.size() > 1) {
			stringstream ss(line);
			ss >> buf;
			if (buf == "newmtl") {
				currentS = new State();
				currentM = new Material();
				currentS->setMaterial(currentM);
				ss >> name;
				(*matMap)[name] = currentS;
				cout << currentS << endl;
				cout << matMap->at(name) << endl;
			} else if (buf == "Ka") {
				ss >> x >> y >> z;
				currentM->setAmbient(x, y, z, 1);
			} else if (buf == "Kd") {
				ss >> x >> y >> z;
				currentM->setDiffuse(x, y, z, 1);
			} else if (buf == "Ks") {
				ss >> x >> y >> z;
				currentM->setSpecular(x, y, z, 1);
			} else if (buf == "d" || buf == "Tr") {
				currentS->addSettings(blending, blending);
				currentS->addLight(0, 0xffffffff);
				// set alpha...
			} else if (buf == "Ns") {
				ss >> temp;
				currentM->setShininess(temp);
			} else if (buf == "s") {
				ss >> temp;
				if (temp) {
					currentS->addSettings(smoothShading, smoothShading);
				} else {
					currentS->addSettings(0, smoothShading);
				}
			} else if (buf == "illum") {
				// Set illumination model
				/*
				the illumination model to be used by the material
				0: no lighting
				1: diffuse lighting only
				2: both diffuse lighting and specular highlights
				 */
				ss >> illum;
			} else if (buf == "map_Kd") {
				ss >> buf;
				Texture* t = new Texture();
				t->loadTexture(buf);
				currentS->addTexture(t);
			}
		}
	}
	indata.close();
	return matMap;
}
