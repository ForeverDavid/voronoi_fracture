// Libs and headers
#include <cassert>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

//Classes
#include "LoadObj.h"

bool LoadObj::loadObject(std::string objName) {

    std::string fileName = "assets/" + objName + ".obj";
    
    std::cout << "\nLoading obj-file: " << fileName <<  " ...\n";

    std::filebuf fb;
    if(fb.open (fileName, std::ios::in)) {
        std::istream is(&fb);

    bool success = readHeader(is);
    if(!success)
        return false;

    success = readData(is);
    if(!success) 
        return false;

    fb.close();

    } else
        return false;
    
    // Build Mesh
    const unsigned int numTriangles = loadData.triangles.size();
    for(unsigned int t = 0; t < numTriangles; t++){
        Vector3<unsigned int> & triangle = loadData.triangles[t];
        std::vector<Vector3 <float> > verts;

        verts.push_back(loadData.verts[triangle[0]]);
        verts.push_back(loadData.verts[triangle[1]]);
        verts.push_back(loadData.verts[triangle[2]]);
        
        mObjects[objName].push_back(verts);
    }

    std::cout << "\n" << fileName.substr(fileName.find("/")+1) << " successfully loaded!\n";

    return true;
}

bool LoadObj::readHeader(std::istream &is){
    std::string buf;
    //read only to the first line starting with "v"
    while(!is.eof() && is.peek() != 'v'){
        getline(is, buf);
    }
    if (is.good())
        return true;
    else
        return false;
}

bool LoadObj::readData(std::istream & is){
    std::string lineBuf;
    int c;
    int i=0;
    while(!is.eof()){
        c = is.peek();
        switch (c) {
        case 'V':
        case 'v': {
            std::string startBuf;
            is >> startBuf; // get the start of the line
            getline(is, lineBuf); // get the rest of the line
            if(startBuf == "v")
                loadData.verts.push_back(Vector3<float>(lineBuf));
        }
            break;
        case 'F':
        case 'f': {
            std::stringstream buf;
            is.get(*buf.rdbuf(), '\n'); // read a line into buf
            is.get(); // read the not extracted \n
            buf << "\n"; // and add it to the string stream

            std::string tmp;
            buf >> tmp; // get the first f or F (+ whitespace)

            // count the number of faces, delimited by whitespace
            int count = 0;
            while (buf >> tmp)
                count++;

            // reset stream
            buf.clear();
            buf.seekg(0, std::ios::beg);

            // Determine wheter we have a triangle or a quad
            if (count == 3)
                loadData.triangles.push_back(readTri(buf));
            else {
              std::cerr << "Encountered polygon with " << count << " faces. i'm giving up.\n";
              return false;
            }
        }
            break;
        default:
            // otherwise just skip the row
            getline(is, lineBuf);

            break;
        }
        i++;
    }
    return true;
}

Vector3<unsigned int> LoadObj::readTri(std::istream &is){
    //  This is a simplified version of an obj reader that can't read normal and texture indices
    std::string buf, v;
    is >> buf;
    assert(buf == "f" || buf=="F");

    getline(is, v); // read indices
    return Vector3<unsigned int>(v) - Vector3<unsigned int>(1,1,1); // obj file format is 1-based
}

std::vector<std::vector<Vector3<float> > > LoadObj::getMeshVertexList(std::string objName) {

    std::map<std::string, std::vector<std::vector<Vector3<float> > > >::iterator it = mObjects.find(objName);

    if(it == mObjects.end()) {
        if(loadObject(objName))
            it = mObjects.find(objName);
        else
            return std::vector<std::vector<Vector3<float> > >();
    }

    return it->second;
}