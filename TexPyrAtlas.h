/* * structured - Tools for the Generation and Visualization of Large-scale
 * Three-dimensional Reconstructions from Image Data. This software includes
 * source code from other projects, which is subject to different licensing,
 * see COPYING for details. If this project is used for research see COPYING
 * for making the appropriate citations.
 * Copyright (C) 2013 Matthew Johnson-Roberson <mattkjr@gmail.com>
 *
 * This file is part of structured.
 *
 * structured is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * structured is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with structured.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEXPYRATLAS_H
#define TEXPYRATLAS_H
#include <osgUtil/Optimizer>
#include <string>
#include <osg/State>
#include <osg/Referenced>
#include "SpatialIndex.h"
#include <unistd.h>
using SpatialIndex::id_type;
typedef std::vector<std::pair<std::string,int> > texcache_t;

class TexPyrAtlas : public osgUtil::Optimizer::TextureAtlasBuilder , public osg::Referenced
{
public:

    TexPyrAtlas(texcache_t imgdir);
    void addSources(std::vector<std::pair<id_type ,std::string> > imageList);
    int getAtlasId(id_type id);
    osg::ref_ptr<osg::Image> getImage(int index,int sizeIndex);
    int getDownsampleSize(int idx){if(idx > (int) _downsampleSizes.size()) return 0; return _downsampleSizes[idx];}
    osg::ref_ptr<osg::Texture> getTexture(int index,int sizeIndex);
    unsigned int getNumAtlases(){return _atlasList.size();}
    void loadTextureFiles(int size);
    osg::Image *getAtlasByNumber(unsigned int i){
        if(i < _atlasList.size())
            return _atlasList[i]->_image;
        return NULL;
    }
    void setAllID(std::map<id_type,int>  allIDs){_allIDs=allIDs;}
    std::vector<osg::ref_ptr<osg::Image> > getImages(void){
        return _images;
    }
    bool getClosestDir(std::string &str,int size){
        for(int i=0; i< (int)_imgdir.size(); i++){
            if(_imgdir[i].second <= size){
                str= _imgdir[i].first;
                return true;
            }
        }
        return false;
    }
    int getMaxNumImagesPerAtlas(void);

    bool getTextureMatrixByIDAtlas(osg::Matrix &matrix,id_type id,char atlas);

    osg::Matrix getTextureMatrixByID(id_type id);
    bool computeTextureMatrixFreedImage(osg::Matrix &matrix,Source *s);
    void computeImageNumberToAtlasMap(void);
    bool _useStub;
    std::map<id_type ,std::string> _totalImageList;
    bool _useTextureArray;
    bool _useAtlas;
    std::map<id_type,int>  _allIDs;
    std::vector<std::set<id_type> > *_sets;
    void setSets(std::vector<std::set<id_type> > *sets){
        _sets=sets;
    }
    std::vector <char> *_vertexToAtlas;

protected:
    void buildAtlas(  );
    osg::ref_ptr<osg::Image> getImageFullorStub(std::string fname,int size);
    std::vector<int> _downsampleSizes;
    osg::ref_ptr<osg::State> _state;
    std::vector<osg::ref_ptr<osg::Image> > _images;
    std::map<Source*,osg::Vec2> _sourceToSize;
    std::map<Source*,id_type> _sourceToId;

    /**
          * Resizes an image using nearest-neighbor resampling. Returns a new image, leaving
          * the input image unaltered.
          *
          * Note. If the output parameter is NULL, this method will allocate a new image and
          * resize into that new image. If the output parameter is non-NULL, this method will
          * assume that the output image is already allocated to the proper size, and will
          * do a resize+copy into that image. In the latter case, it is your responsibility
          * to make sure the output image is allocated to the proper size and with the proper
          * pixel configuration (it must match that of the input image).
          *
          * If the output parameter is non-NULL, then the mipmapLevel is also considered.
          * This lets you resize directly into a particular mipmap level of the output image.
          */
    bool
            resizeImage(const osg::Image* input,
                        unsigned int out_s, unsigned int out_t,
                        osg::ref_ptr<osg::Image>& output,
                        unsigned int mipmapLevel=0 );
    texcache_t _imgdir;
    std::map<id_type,int> _idToAtlas;
   // std::map<id_type,Source*> _idToSource;
    std::vector<std::map<id_type,Source*> > _idToSourceAtlas;

    std::map<id_type,Source* > _idToSource;
    OpenThreads::Mutex _imageListMutex;
    int maxImageSize;



        };
typedef   long blend_id_t[4];

typedef std::vector<std::pair<unsigned int,blend_id_t> > blend_pt_t;
std::vector< std::set<SpatialIndex::id_type>  >  calc_atlases(const osg::Vec3Array *pts,
                                        const osg::PrimitiveSet& prset,
                                        const osg::Vec4Array *blendIdx,
                                        std::vector<char> &atlasmap,
                                        int max_img_per_atlas );
std::string getUUID(void);
#endif // TEXPYRATLAS_H
