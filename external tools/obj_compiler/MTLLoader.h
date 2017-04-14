/*
 * MTLLoader.h
 *
 *  Created on: Jan 28, 2010
 *      Author: Fnatte
 */

#ifndef MTLLOADER_H_
#define MTLLOADER_H_

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "Material.h"
#include "State.h"

using namespace std;

class MTLLoader {
public:
	MTLLoader();
	~MTLLoader();

	map<string, State*>* readFile(string fileName);
};

#endif /* MTLLOADER_H_ */
