#ifndef VPBINTERFACE_HPP
#define VPBINTERFACE_HPP
#include "stereo_cells.hpp"
#include "Extents.h"
#include <vpb/Commandline>
#include <vector>
#include <string>

void doQuadTreeVPB(std::string cacheddir,std::vector<std::vector<std::string> > datalist_lod,Bounds bounds,Camera_Calib &calib,texcache_t cacheDirs,bool useTextureArray);

#endif // VPBINTERFACE_HPP
