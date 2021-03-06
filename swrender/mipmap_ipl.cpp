/*
    This file is part of Mitsuba, a physically based rendering system.

    Copyright (c) 2007-2011 by Wenzel Jakob and others.

    Mitsuba is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Mitsuba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mipmap_ipl.h"
#include "render_utils.h"
#if defined(__OSX__)
#include <sys/sysctl.h>
#elif defined(WIN32)
#include <direct.h>
#else
#include <malloc.h>
#endif
#define Float MIPMap::Float

int log2i(uint32_t value) {
        int r = 0;
        while ((value >> r) != 0)
                r++;
        return r-1;
}

int log2i(uint64_t value) {
        int r = 0;
        while ((value >> r) != 0)
                r++;
        return r-1;
}
/// Integer floor function
inline int ifloor( float x )
{
   if (x >= 0)
   {
       return (int)x;
   }
   else
   {
       int y = (int)x;
       return ((float)y == x) ? y : y - 1;
   }
}
/// Integer floor function
inline int floorToInt(Float value) {
       return ifloor(value);
}

int modulo(int a, int b) {
        int result = a - (a/b) * b;
        return (result < 0) ? result+b : result;
}

Float modulo(Float a, Float b) {
        Float result = a - int(a/b) * b;
        return (result < 0) ? result+b : result;
}

/* Fast rounding & power-of-two test algorithms from PBRT */
uint32_t roundToPowerOfTwo(uint32_t i) {
        i--;
        i |= i >> 1; i |= i >> 2;
        i |= i >> 4; i |= i >> 8;
        i |= i >> 16;
        return i+1;
}

uint64_t roundToPowerOfTwo(uint64_t i) {
        i--;
        i |= i >> 1;  i |= i >> 2;
        i |= i >> 4;  i |= i >> 8;
        i |= i >> 16; i |= i >> 32;
        return i+1;
}
#define Epsilon 1e-4f


Float MIPMap::lanczosSinc(Float t, Float tau) const{
        t = std::abs(t);
        if (t < Epsilon)
                return 1.0f;
        else if (t > 1.0f)
                return 0.0f;
        t *= M_PI;
        Float sincTerm = std::sin(t*tau)/(t*tau);
        Float windowTerm = std::sin(t)/t;
        return sincTerm * windowTerm;
}
#if !defined(L1_CACHE_LINE_SIZE)
#define L1_CACHE_LINE_SIZE 64
#endif

void * __restrict allocAligned(size_t size) {
#if defined(WIN32)
        return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined(__APPLE__)
        /* OSX malloc already returns 16-byte aligned data suitable
           for AltiVec and SSE computations */
        return malloc(size);
#else
        return memalign(L1_CACHE_LINE_SIZE, size);
#endif
}
/// Simple floating point clamping function
/*inline Float clamp(Float value, Float min, Float max) {
        if (value < min)
                return min;
        else if (value > max)
                return max;
        else return value;
}*/
void freeAligned(void *ptr) {
#if defined(WIN32)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
}


/// Simple integer clamping function
/*inline int clamp(int value, int min, int max) {
        if (value < min)
                return min;
        else if (value > max)
                return max;
        else return value;
}
*/
/* Isotropic/anisotropic EWA mip-map texture map class based on PBRT */
MIPMap::MIPMap(IplImage *image,
	EFilterType filterType, EWrapMode wrapMode, Float maxAnisotropy) 
                : m_filterType(filterType),
		  m_wrapMode(wrapMode), m_maxAnisotropy(maxAnisotropy) {
        IplImage *texture = image;
        if(!image){
            fprintf(stderr,"Invalid image\n");
            exit(-1);
        }
         m_width=image->width;
         m_height=image->height;
       /* if (filterType != ENone && filterType != EBilinear && (!isPowerOfTwo(image->width) || !isPowerOfTwo(image->height))) {
                m_width = (int) roundToPowerOfTwo((uint32_t) image->width);
                m_height = (int) roundToPowerOfTwo((uint32_t) image->height);


                texture = cvCreateImage(cvSize(m_width,m_height),IPL_DEPTH_8U,image->nChannels);
                cvResize(image,texture);
                //cvReleaseImage(&image);
        }*/

	if (m_filterType != ENone)
                m_levels = 1 + log2i((uint32_t) std::max(m_width, m_height));
	else
		m_levels = 1;

        m_pyramid = new IplImage*[m_levels];
	m_pyramid[0] = texture;
	m_levelWidth = new int[m_levels];
	m_levelHeight= new int[m_levels];
	m_levelWidth[0] = m_width;
	m_levelHeight[0] = m_height;

	/* Generate the mip-map hierarchy */
	for (int i=1; i<m_levels; i++) {
		m_levelWidth[i]  = std::max(1, m_levelWidth[i-1]/2);
		m_levelHeight[i] = std::max(1, m_levelHeight[i-1]/2);
                m_pyramid[i] = cvCreateImage(cvSize(m_levelWidth[i] , m_levelHeight[i]),IPL_DEPTH_8U,texture->nChannels);
              //  printf("[%d %d] [%d %d]\n",m_levelWidth[i],m_levelHeight[i],m_levelWidth[i-1],m_levelHeight[i-1]);
                if(m_levelHeight[i]  == 1 || m_levelWidth[i]==1)
                    cvResize(m_pyramid[i-1],m_pyramid[i]);
                else
                    fast_mipmap_downsize(m_pyramid[i-1],m_pyramid[i]);

	}

	if (m_filterType == EEWA) {
		m_weightLut = static_cast<Float *>(allocAligned(sizeof(Float)*MIPMAP_LUTSIZE));
		for (int i=0; i<MIPMAP_LUTSIZE; ++i) {
			Float pos = (Float) i / (Float) (MIPMAP_LUTSIZE-1);
                       m_weightLut[i] = std::exp(-2.0f * pos) - std::exp(-2.0f);
		}
	}
}

MIPMap::~MIPMap() {
	if (m_filterType == EEWA) 
		freeAligned(m_weightLut);
	for (int i=0; i<m_levels; i++)
                cvReleaseImage(&m_pyramid[i]);
	delete[] m_levelHeight;
	delete[] m_levelWidth;
	delete[] m_pyramid;
}

/*Spectrum MIPMap::getMaximum() const {
	Spectrum max(0.0f);
	int height = m_levelHeight[0];
	int width = m_levelWidth[0];
        Spectrum *pixels = m_pyramid[0];
	for (int y=0; y<height; ++y) {
		for (int x=0; x<width; ++x) {
			Spectrum value = *pixels++;
			for (int j=0; j<SPECTRUM_SAMPLES; ++j)
				max[j] = std::max(max[j], value[j]);
		}
	}
	if (m_wrapMode == EWhite) {
		for (int i=0; i<SPECTRUM_SAMPLES; ++i)
			max[i] = std::max(max[i], (Float) 1.0f);
	}
	return max;
}
        */
 MIPMap * MIPMap::fromBitmap(IplImage *bitmap, EFilterType filterType,
                EWrapMode wrapMode, Float maxAnisotropy
                ) {
       /* int width = bitmap->width;
        int height = bitmap->height;
        unsigned char *data = (unsigned char *)bitmap->imageData;
	Spectrum s, *pixels = new Spectrum[width*height];

	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
                        float r = CV_IMAGE_ELEM(bitmap,unsigned char,y,(bitmap->nChannels*x)+2)/255.0;
                        float g = CV_IMAGE_ELEM(bitmap,unsigned char,y,(bitmap->nChannels*x)+1)/255.0;
                        float b = CV_IMAGE_ELEM(bitmap,unsigned char,y,(bitmap->nChannels*x)+0)/255.0;
                        // Convert to a spectral representation
                        s.fromLinearRGB(r, g, b);
			s.clampNegative();
			pixels[y*width+x] = s;
		}
	}
*/
        return new MIPMap(bitmap,
		filterType, wrapMode, maxAnisotropy);
}

MIPMap::ResampleWeight *MIPMap::resampleWeights(int oldRes, int newRes) const {
	/* Resample using a Lanczos windowed sinc reconstruction filter */
        assert(newRes >= oldRes);
	Float filterWidth = 2.0f;
	ResampleWeight *weights = new ResampleWeight[newRes];
	for (int i=0; i<newRes; i++) {
		Float center = (i + .5f) * oldRes / newRes;
		weights[i].firstTexel = floorToInt(center - filterWidth + (Float) 0.5f);
		Float weightSum = 0;
		for (int j=0; j<4; j++) {
			Float pos = weights[i].firstTexel + j + .5f;
			Float weight = lanczosSinc((pos - center) / filterWidth);
			weightSum += weight;
			weights[i].weight[j] = weight;
		}

		/* Normalize */
		Float invWeights = 1.0f / weightSum;
		for (int j=0; j<4; j++)
			weights[i].weight[j] *= invWeights;
	}
	return weights;
}
	
Spectrum MIPMap::getTexel(int level, int x, int y) const {
	int levelWidth = m_levelWidth[level];
	int levelHeight = m_levelHeight[level];

	if (x <= 0 || y < 0 || x >= levelWidth || y >= levelHeight) {
		switch (m_wrapMode) {
			case ERepeat:
				x = modulo(x, levelWidth);
				y = modulo(y, levelHeight);
				break;
			case EClamp:
				x = clamp(x, 0, levelWidth - 1);
				y = clamp(y, 0, levelHeight - 1);
				break;
			case EBlack:
				return Spectrum(0.0f);
			case EWhite:
				return Spectrum(1.0f);
		}
	}
        Spectrum tmp;
        unsigned char r,g,b;
        r=CV_IMAGE_ELEM(m_pyramid[level],unsigned char,y,(m_pyramid[level]->nChannels*x)+2);
        g=CV_IMAGE_ELEM(m_pyramid[level],unsigned char,y,(m_pyramid[level]->nChannels*x)+1);
        b=CV_IMAGE_ELEM(m_pyramid[level],unsigned char,y,(m_pyramid[level]->nChannels*x)+0);
        tmp.fromLinearRGB(r/255.0,g/255.0,b/255.0);
        return tmp;
}
	
Spectrum MIPMap::triangle(int level, Float x, Float y) const {
	if (m_filterType == ENone) {
		int xPos = floorToInt(x*m_levelWidth[0]),
			yPos = floorToInt(y*m_levelHeight[0]);
		return getTexel(0, xPos, yPos);
	} else {
		level = clamp(level, 0, m_levels - 1);
		x = x * m_levelWidth[level] - 0.5f;
		y = y * m_levelHeight[level] - 0.5f;
		int xPos = floorToInt(x), yPos = floorToInt(y);
		Float dx = x - xPos, dy = y - yPos;
		return getTexel(level, xPos, yPos) * (1.0f - dx) * (1.0f - dy)
			+ getTexel(level, xPos, yPos + 1) * (1.0f - dx) * dy
			+ getTexel(level, xPos + 1, yPos) * dx * (1.0f - dy)
			+ getTexel(level, xPos + 1, yPos + 1) * dx * dy;
	}	
}
		
Spectrum MIPMap::getValue(Float u, Float v, 
		Float dudx, Float dudy, Float dvdx, Float dvdy) const {
	if (m_filterType == ETrilinear) {
		/* Conservatively estimate a square lookup region */
		Float width = 2.0f * std::max(
			std::max(std::abs(dudx), std::abs(dudy)),
			std::max(std::abs(dvdx), std::abs(dvdy)));
		Float mipmapLevel = m_levels - 1 + 
			log2(std::max(width, (Float) 1e-8f));

		if (mipmapLevel < 0) {
			/* The lookup is smaller than one pixel */
			return triangle(0, u, v);
		} else if (mipmapLevel >= m_levels - 1) {
			/* The lookup is larger than the whole texture */
			return getTexel(m_levels - 1, 0, 0);
		} else {
			/* Tri-linear interpolation */
			int level = (int) mipmapLevel;
			Float delta = mipmapLevel - level;
			return triangle(level, u, v) * (1.0f - delta)
				+ triangle(level, u, v) * delta;
		}
	} else if (m_filterType == EEWA) {
		if (dudx*dudx + dudy*dudy < dvdx*dvdx + dvdy*dvdy) {
			std::swap(dudx, dvdx);
			std::swap(dudy, dvdy);
		}

		Float majorLength = std::sqrt(dudx * dudx + dudy * dudy);
		Float minorLength = std::sqrt(dvdx * dvdx + dvdy * dvdy);

		if (minorLength * m_maxAnisotropy < majorLength && minorLength > 0.0f) {
			Float scale = majorLength / (minorLength * m_maxAnisotropy);
			dvdx *= scale; dvdy *= scale;
			minorLength *= scale;
		}

		if (minorLength == 0)
			return triangle(0, u, v);
	
		// The min() below avoids overflow in the int conversion when lod=inf
		Float lod = 
                        std::min(std::max((Float) 0, (Float)(m_levels - 1 + log2(minorLength))),
				(Float) (m_levels-1));
		int ilod = floorToInt(lod);
		Float d = lod - ilod;

		return EWA(u, v, dudx, dudy, dvdx, dvdy, ilod)   * (1-d) +
			   EWA(u, v, dudx, dudy, dvdx, dvdy, ilod+1) * d;
	} else {
		int xPos = floorToInt(u*m_levelWidth[0]),
			yPos = floorToInt(v*m_levelHeight[0]);
		return getTexel(0, xPos, yPos);
	}
}
	
Spectrum MIPMap::EWA(Float u, Float v, Float dudx, Float dudy, Float dvdx, 
	Float dvdy, int level) const {
	if (level >= m_levels)
		return getTexel(m_levels-1, 0, 0);

	Spectrum result(0.0f);
	Float denominator = 0.0f;
	u = u * m_levelWidth[level]; v = v * m_levelHeight[level];
	dudx = dudx * m_levelWidth[level]; dudy = dudy * m_levelHeight[level];
	dvdx = dvdx * m_levelWidth[level]; dvdy = dvdy * m_levelHeight[level];

	Float A = dudy * dudy + dvdy * dvdy + 1.0f;
	Float B = -2.0f * (dudx * dudy + dvdx * dvdy);
	Float C = dudx * dudx + dvdx * dvdx + 1.0f;
	Float F = A * C - B * B * 0.25f;
	Float du = std::sqrt(C), dv = std::sqrt(A);
	int u0 = (int) std::ceil(u - du);
	int u1 = (int) std::floor(u + du);
	int v0 = (int) std::ceil(v - dv);
	int v1 = (int) std::floor(v + dv);
	Float invF = 1.0f / F;
	A *= invF; B *= invF; C *= invF;

	for (int ut = u0; ut <= u1; ++ut) {
		const Float uu = ut - u;
		for (int vt = v0; vt <= v1; ++vt) {
			const Float vv = vt - v;
			const Float r2 = A*uu*uu + B*uu*vv + C*vv*vv;
			if (r2 < 1) {
				const Float weight = m_weightLut[
					std::max(0, std::min((int) (r2 * MIPMAP_LUTSIZE), MIPMAP_LUTSIZE - 1))];
				result += getTexel(level, ut, vt) * weight;
				denominator += weight;
			}
		}
	}
	return result / denominator;
}

/*IplImage *MIPMap::getLDRBitmap() const {
        IplImage *bitmap = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,SPECTRUM_SAMPLES);
        uint8_t *data = (unsigned char *)bitmap->imageData;
	Spectrum *specData = m_pyramid[0];

	for (int y=0; y<m_height; ++y) {
		for (int x=0; x<m_width; ++x) {
			Float r, g, b;
			(specData++)->toLinearRGB(r, g, b);
                        *data++ = (uint8_t) std::min(255, std::max(0, (int) (r*255)));
                        *data++ = (uint8_t) std::min(255, std::max(0, (int) (g*255)));
                        *data++ = (uint8_t) std::min(255, std::max(0, (int) (b*255)));
		}
	}
	return bitmap;
}*/

