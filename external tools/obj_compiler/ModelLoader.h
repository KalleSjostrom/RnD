/*
 * ModelLoader.h
 *
 *  Created on: Jan 22, 2010
 *      Author: Fnatte
 */

#ifndef MODELLOADER_H_
#define MODELLOADER_H_

#include <string>
#include <fstream>
#include <sstream>
#include "Face.h"
#include "Vertex.h"
#include "Vector.h"
#include "Point.h"
#include "MTLLoader.h"

#include <vector>
#include "Model.h"
#include "Node.h"
#include "Geometry.h"
#include "Group.h"
#include "State.h"
#include "Color.h"

using namespace std;

class ModelLoader {
public:
	ModelLoader();
	~ModelLoader();
	Node* readFile(string fileName);

private:
	void splitString(string str, string delim, vector<string>& results);
};

#endif /* MODELLOADER_H_ */
