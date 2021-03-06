//
// structured - Tools for the Generation and Visualization of Large-scale
// Three-dimensional Reconstructions from Image Data. This software includes
// source code from other projects, which is subject to different licensing,
// see COPYING for details. If this project is used for research see COPYING
// for making the appropriate citations.
// Copyright (C) 2013 Matthew Johnson-Roberson <mattkjr@gmail.com>
//
// This file is part of structured.
//
// structured is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// structured is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with structured.  If not, see <http://www.gnu.org/licenses/>.
//

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/io_utils>
#include "Clipper.h"
#include <osgUtil/SmoothingVisitor>
#include "PLYWriterNodeVisitor.h"
#include <osgUtil/Optimizer>
#include <osgUtil/DelaunayTriangulator>
#include <osg/Point>
#include <iostream>
#include <osg/ComputeBoundsVisitor>
#include "vertexData.h"

void addDups(osg::Geode *geode);
using namespace std;
void write_header(std::ostream& _fout,int total_face_count,bool color,bool qual){
    _fout <<"ply\n";
    _fout <<"format binary_little_endian 1.0\n";
    //_fout <<"comment PLY exporter written by Paul Adams\n";
    _fout <<"element vertex "<<total_face_count <<std::endl;
    _fout <<"property float x\n";
    _fout <<"property float y\n";
    _fout <<"property float z\n";
    if(color){
        _fout <<"property uchar red\n";
        _fout <<"property uchar green\n";
        _fout <<"property uchar blue\n";
    }
    _fout <<"element face " <<total_face_count/3<<std::endl;
    _fout <<"property list uchar int vertex_indices\n";
    if(qual)
        _fout <<"property float quality\n";

    _fout <<"end_header\n";
}
void write_all(std::ostream& _fout,osg::DrawElementsUInt *tri,osg::Vec3Array *verts,osg::Vec4Array *colors,osg::Vec2Array *qual,bool flip,osg::Matrix m){
    int cnt=0;
    for(int i=0; i< (int)tri->size()-2; i+=3){
        for(int j=0; j<3; j++){
            osg::Vec3 v=verts->at(tri->at(i+j))*m;
            float vf[3];
            vf[0]=v[0];
            vf[1]=v[1];
            vf[2]=v[2];
            _fout.write((char *)vf,3*sizeof(float));
            if(colors && i+j <(int)colors->size() ){
                unsigned char col[3];
                osg::Vec4 c=colors->at(i+j);
               // cout <<c<<endl;
                col[0]=c[0]*255.0;
                col[1]=c[1]*255.0;
                col[2]=c[2]*255.0;
                _fout.write((char *)col,3*sizeof(unsigned char));

            }
        }
    }
    int iout[3];
    unsigned char c=3;
    for(int i=0; i<(int) tri->size()-2; i+=3){
        _fout.write((char *)&c,sizeof(char));

        if(flip){
            iout[0]=i;
            iout[1]=i+1;
            iout[2]=i+2;
        }else{
            iout[0]=i+2;
            iout[1]=i+1;
            iout[2]=i+0;

        }
        _fout.write((char*)iout,sizeof(int)*3);
        if(qual != NULL){
            float qualV =qual->at(i/3)[0];
            _fout.write((char*)&qualV,sizeof(float));
        }
        cnt++;

    }
    printf("%d\n",cnt);


}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    string outdump;

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() +" is the example which demonstrates Depth Peeling");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" filename");
    string outfilename="out.ply";
    arguments.read("--outfile",outfilename);
    bool flip=arguments.read("-F");
    OverlapMode mode=DUP;
    if(arguments.read("-gap"))
        mode=GAP;
    else if(arguments.read("-dup"))
        mode=DUP;
    else if(arguments.read("-cut"))
        mode=CUT;
    else if(arguments.read("-dump")){
        mode=DUMP;

    }else{
        cerr << "Must be -gap or  -dup  or -cut or  -dump\n";
        return -1;
    }//  arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    /*  if(arguments.argc() < (2+6)  ){
        fprintf(stderr,"Must pass two meshes and bbox arg must be base dir\n");
        arguments.getApplicationUsage()->write(std::cerr,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        exit(-1);
    }*/

    osg::Vec3 minV,maxV;
    /* minV.x() = atof(arguments[2]);
    minV.y() = atof(arguments[3]);
    minV.z() = atof(arguments[4]);

    maxV.x() = atof(arguments[5]);
    maxV.y() = atof(arguments[6]);
    maxV.z() = atof(arguments[7]);*/
    if(!arguments.read("--bbox",minV.x(),minV.y(),minV.z(),maxV.x() ,maxV.y() ,maxV.z() )){
        fprintf(stderr,"Must pass two meshes and bbox arg must be base dir\n");
        arguments.getApplicationUsage()->write(std::cerr,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        exit(-1);
    }

    osg::Matrix inverseM=osg::Matrix::identity();
    bool rot=false;
    double rx,ry,rz;
    if(arguments.read("--invrot",rx,ry,rz)){
        rot=true;
    }
    bool qualOn=arguments.read("-qual");

    ply::VertexData vertexData;
    osg::Node *root=NULL;
    osg::BoundingBox bb(minV,maxV);
    osg::notify(osg::NOTICE) << bb._min << " " << bb._max << endl;
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            // not an option so assume string is a filename.
            string fname= arguments[pos];
            cout <<"Loading:"<< fname <<endl;
            root= vertexData.readPlyFile(fname.c_str(),false,&bb,mode,qualOn);


        }
    }
    string empt=outfilename+".empty";

    if(!vertexData._vertices.valid() || vertexData._vertices->size() == 0){
        fprintf(stderr,"No valid data in bbox returning %lu\n",vertexData._vertices.valid() ?  vertexData._vertices->size() : -1 );
        FILE *fp=fopen(empt.c_str(),"w");
        fprintf(fp,"0\n");
        fclose(fp);
        return 0;
    }
    if(osgDB::fileExists(empt)){
        if( remove( empt.c_str() ) != 0 ){
            perror( "Error deleting empty file" );
            exit(-1);
        }
        else
            puts( "empty File successfully deleted" );
    }
    /*
    osgDB::Registry::instance()->setBuildKdTreesHint(osgDB::ReaderWriter::Options::BUILD_KDTREES);
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if(!model.valid()){
        fprintf(stderr,"Can't load model %s",arguments[1]);
        exit(-1);
    }

      osg::ref_ptr<osg::Node> root;
    // osg::Vec3Array *dumpPts=NULL;
    // do
    osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    model->accept(cbbv);
    osg::BoundingBox bb2 = cbbv.getBoundingBox();
    std::cout << bb2._min << " "<< bb2._max <<std::endl;
    //while(result);
    if(model.valid()){
        osg::Geode *geode= dynamic_cast<osg::Geode*>(model.get());
        if(!geode)
            geode=model->asGroup()->getChild(0)->asGeode();
        if(geode && geode->getNumDrawables()){


            osg::Drawable *drawable = geode->getDrawable(0);
            osg::KdTree *kdTree = dynamic_cast<osg::KdTree*>(drawable->getShape());
            if(kdTree){
                osg::Geometry *geom = dynamic_cast< osg::Geometry*>(drawable);
                geom_elems_src srcGeom;
                srcGeom.colors=(osg::Vec4Array*)geom->getColorArray();
                //    srcGeom.texcoords;
                srcGeom.texid=NULL;
                int numTex=0;
                geom_elems_dst dstGeom(numTex);
                KdTreeBbox *kdtreeBbox=new KdTreeBbox(*kdTree,srcGeom);

                root=kdtreeBbox->intersect(bb,dstGeom,mode);
            }else{
                fprintf(stderr,"Can't load kdTreee\n");
                root=new osg::Geode;

            }
        }else{
            osg::notify(osg::ALWAYS) << "Model can't be converted to geode\n";
            exit(-1);
        }

    }else{
        osg::notify(osg::ALWAYS)  << "Model can't be loaded\n";
        exit(-1);
    }
    */

    /*if( mode==DUMP){
        // create triangulator and set the points as the area
        if(!dumpPts || !dumpPts->size()){
            osg::notify(osg::ALWAYS)  << "Dump pts don't exist\n";
        }else{

            osg::Geometry* gm = new osg::Geometry;
            gm->setVertexArray(dumpPts);
            gm->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,dumpPts->size()));
            //   gm->addPrimitiveSet(trig->getTriangles());
            osg::Vec4Array* colors = new osg::Vec4Array(1);
            colors->push_back(osg::Vec4(rand()/(double) RAND_MAX,rand()/(double) RAND_MAX,rand()/(double) RAND_MAX,1));
            gm->setColorArray(colors);
            gm->setColorBinding(osg::Geometry::BIND_OVERALL);
            //create geometry and add it to scene graph
            // gm->getOrCreateStateSet()->setAttribute( new osg::Point( 3.0f ),osg::StateAttribute::ON );

            osg::Geode* geode = new osg::Geode();
            geode->addDrawable(gm);
            osgDB::writeNodeFile(*geode,outdump);
        }
    }*/
    if(rot){
        inverseM =osg::Matrix::rotate(
                osg::DegreesToRadians( rx ), osg::Vec3( 1, 0, 0 ),
                osg::DegreesToRadians( ry ), osg::Vec3( 0, 1, 0 ),
                osg::DegreesToRadians( rz ), osg::Vec3( 0, 0, 1 ) );

    }else
        inverseM=osg::Matrix::identity();

    if(osgDB::getFileExtension(outfilename) == "ply"){

        // osgUtil::SmoothingVisitor sv;
        //root->accept(sv);
        std::ofstream f(outfilename.c_str());
        bool color = vertexData._colors.valid() ? (vertexData._colors->size() >0) : false;
        osg::Vec2Array *validArr=(vertexData._qualArray.valid() && vertexData._qualArray->size()>0) ? vertexData._qualArray : NULL;
        bool qual =(validArr != NULL);
        printf("Writing QUAL?: %d %lu %lu\n",qual,qual ? vertexData._qualArray->size() : 0 ,vertexData._triangles->size());
        write_header(f,vertexData._triangles->size(),color,qual);

        write_all(f,vertexData._triangles,vertexData._vertices,vertexData._colors,validArr,flip,inverseM);
        //PLYWriterNodeVisitor nv(f);
        //root->accept(nv);
        f.close();;
    }else{
        osg::ref_ptr<osg::MatrixTransform>xform = new osg::MatrixTransform;
        xform->setDataVariance( osg::Object::STATIC );
        xform->setMatrix(inverseM);
        xform->addChild(root);
        osgUtil::Optimizer::FlattenStaticTransformsVisitor fstv(NULL);
        xform->accept(fstv);
        fstv.removeTransforms(xform);
        osgUtil::SmoothingVisitor sv;
        root->accept(sv);
        osgDB::writeNodeFile(*root,outfilename);
    }

}
