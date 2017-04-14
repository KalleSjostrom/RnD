/*
 * ModelLoader.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: Fnatte
 */

#include "ModelLoader.h"

ModelLoader::ModelLoader() {}
ModelLoader::~ModelLoader() {}

Node* ModelLoader::readFile(string fileName) {
	ifstream indata;
	string buf;
	vector<Face*> faces;
	vector<Vertex*> vertices;
	vector<Vector*> normals;
	vector<Point*> texCoords;
	vector<Color*> colors;
	map<string, State*>* states = NULL;

	float x, y, z;
	cout << "Trying to open file " << fileName << endl;
    indata.open(fileName.c_str());

	if(!indata) {
		cerr << "Error: file could not be opened" << endl;
		exit(-1);
	}

	string line;
	size_t size = 0;

	State* currentState = NULL;
	Geometry* currentGeometry = NULL;
	Group* currentGroup = NULL;
	Group* root = new Group();
	bool setCurrentGeometry = false;
	bool hasNormals = false;
	bool hasTexCoords = false;
	bool hasColors = false;
	while (getline(indata, line)) {
		if (line.size() > 1) {
			stringstream ss(line);
			ss >> buf;
			//cout << line << endl;
			if (buf == "mtllib") {
				MTLLoader mtl;
				string file;
				ss >> file;
				cout << "mtllib " << file << endl;
				states = mtl.readFile(file);
			} else if (buf == "usemtl") {
				string file;
				ss >> file;
				cout << "usemtl " << file << endl;
				map<string, State*>::iterator it;
				for (it = states->begin(); it != states->end(); it++) {
					State* m = it->second;
					cout << "state " << it->first << " " << m << endl;
				}
				currentState = states->at(file);
			} else if (buf == "o") {

			} else if (buf == "g") {
				if (setCurrentGeometry) {
					Model* m = new Model(faces, size, hasNormals, hasTexCoords, hasColors);
					currentGeometry = new Geometry(m);
					currentGroup->addChild(currentGeometry);
					faces.clear();
					size = 0;
				}
				currentGroup = new Group();
				currentGroup->setState(currentState);
				root->addChild(currentGroup);
				setCurrentGeometry = true;
			} else if (buf == "s") {
				cout << "s" << endl;
				//int setting = shadeModel;
				// currentState->setSettings(setting, setting);
			} else if (buf == "v") {
				ss >> x >> y >> z;
				vertices.push_back(new Vertex(x, y, z));
			} else if (buf == "vt") {
				ss >> x >> y;
				texCoords.push_back(new Point(x, y, 0));
			} else if (buf == "vn") {
				ss >> x >> y >> z;
				Vector* n = new Vector(x, y, z);
				// n->flipNormal();
				n->normalize();
				normals.push_back(n);
			} else if (buf == "vc") {
				float r, g, b, a;
				ss >> r >> g >> b >> a;
				Color* c = new Color(r, g, b, a);
				colors.push_back(c);
			} else if (buf == "f") {
				int v, vt, vn, vc;
				vector<string> res;
				stringstream temp;
				vector<Vertex*> faceVerts;
				vector<Point*> faceTexCoords;
				vector<Vector*> faceNormals;
				vector<Color*> faceColors;
				while (ss >> buf) {
					res.clear();
					v = vt = vn = vc = 0;

					size_t cutAt;
					unsigned int i = 0;
					float* data = new float[4];
                    data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0;
					while ((cutAt = buf.find_first_of("/")) != string::npos) {
						if (cutAt > 0) {
							temp.clear();
							temp << buf.substr(0, cutAt);
							temp >> data[i++];
						} else {
							data[i++] = 0;
						}
						buf = buf.substr(cutAt + 1);
					}
					if (buf.length() > 0) {
						temp.clear();
						temp << buf;
						temp >> data[i];
					}

					v = data[0];
					vt = data[1];
					vn = data[2];
					vc = data[3];
					delete data;

					faceVerts.push_back(vertices[v - 1]);
					if (vt) {
						Point* p = texCoords[vt - 1];
						faceTexCoords.push_back(p);
					}

					if (vn) {
						Vector* n = normals[vn - 1];
						faceNormals.push_back(n);
					}
					if (vc) {
						Color* c = colors[vc - 1];
						faceColors.push_back(c);
					}

				}

				hasTexCoords = faceTexCoords.size() > 0;
				hasNormals = faceNormals.size() > 0;
				hasColors = faceColors.size() > 0;
				Face* f = new Face(faceVerts, faceTexCoords, faceNormals, faceColors);
				size += faceVerts.size() * 3;
				size += faceTexCoords.size() * 2;
				size += faceNormals.size() * 3;
				size += faceColors.size() * 4;
				faces.push_back(f);
			}
		}
	}
	Model* m = new Model(faces, size, hasNormals, hasTexCoords, hasColors);
	currentGeometry = new Geometry(m);
	if (setCurrentGeometry) {
		currentGroup->addChild(currentGeometry);
	}

	indata.close();
	faces.clear();
	size = 0;

	if (root->nrChildren() > 1) {
		return root;
	} else if (currentGroup != NULL) {
		return currentGroup;
	} else {
		return currentGeometry;
	}
}

void ModelLoader::splitString(string str, string delim, vector<string>& results) {
	size_t cutAt;
	while ((cutAt = str.find_first_of(delim)) != string::npos) {
		if (cutAt > 0) {
			results.push_back(str.substr(0, cutAt));
		}
		str = str.substr(cutAt + 1);
	}
	if (str.length() > 0) {
		results.push_back(str);
	}
}
