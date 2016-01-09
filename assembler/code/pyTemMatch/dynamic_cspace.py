#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Sun Aug 10 15:05:49 2014

@author: nelaturi
"""

import sys,os
from time import time as now
from optparse import OptionParser
from numpy import *   
from skimage import io
from skimage import filter
from skimage import morphology
from skimage import feature
from skimage import transform
from skimage import segmentation 

from skimage import data, color
from skimage.transform import hough_circle
from skimage.feature import peak_local_max
from skimage.draw import circle_perimeter
from skimage.util import img_as_ubyte



import matplotlib.pyplot as plt

from pycuda.compiler import SourceModule
import pycuda.gpuarray as gpuarray
import pycuda.autoinit
import pycuda.driver as cuda
from pyfft.cuda import Plan



def conv_numPy(Y,Z,isCorr):
	#print "Using serial FFT "
	#Implement image cross-correlation using numPy FFT routines
	Ysize = array(Y.shape);
	Zsize = array(Z.shape);
	convSize = Ysize + Zsize;
	w = convSize[0]; 
	h = convSize[1] ;
	# Account for wraparound error in the convolution by padding zeros
	im1 = zeros((w,h));
	im1[:Ysize[0],:Ysize[1]] = Y;
	im2 = zeros((w,h));
	if isCorr:
		im2[w-Zsize[0]:, h-Zsize[1]:] = Z;
	else:
		im2[:Zsize[0], :Zsize[1]] = Z;	
	# Compute Fourier Transforms
	im1_ft = fft.fft2(im1);
	im2_ft = fft.fft2(im2);
	if isCorr:
		im2_ft = im2_ft.conj(); 	# 'Reflect' the shape for correlation
	# Convolution in spatial domain after inverting product of Fourier Transforms in frequency domain
	A = fft.ifft2(im1_ft * im2_ft); 
	return A.real
 
def conv_GPU(Y,Z,isCorr):
	#Implement image cross correlation on the GPU using pyCUDA 
	# Spatial anti-aliasing padding 
	Ysize = array(Y.shape);
	Zsize = array(Z.shape);
	convSize = Ysize + Zsize;
	w = convSize[0]; 
	w = int(pow(2,ceil(log(w)/log(2))))	# convert to power of 2 for FT and cast to int for Plan 
	h = convSize[1];
	h = int(pow(2,ceil(log(h)/log(2))))
	# Account for wraparound error in the convolution by padding zeros
	im1 = zeros((w,h));
	im1[:Ysize[0],:Ysize[1]] = Y;
	im2 = zeros((w,h));
	imsize = im1.size + im2.size
	if isCorr:
		im2[w-Zsize[0]:, h-Zsize[1]:] = Z;
	else:
		im2[:Zsize[0], :Zsize[1]] = Z;
	# Define data as single precision arrays
	im1 = im1.astype(complex64)
	im2 = im2.astype(complex64)
	# Map image data to GPU
	im1_gpu = gpuarray.to_gpu(im1);
	im2_gpu = gpuarray.to_gpu(im2);
	plan = Plan((w,h),normalize=True,fast_math=False)	
	# Execute plan and compute Fourier Transforms on GPU
	plan.execute(im1_gpu)
	plan.execute(im2_gpu)
	# Apply convolution theorem
	if isCorr:
		im2_gpu = gpuarray.GPUArray.conj(im2_gpu)  # Conjugate for correlation - on the GPU
	im1_gpu = gpuarray.GPUArray.__mul__(im1_gpu,im2_gpu); 
	del im2_gpu		
        plan.execute(im1_gpu, inverse=True)
	conv = im1_gpu.get();
	conv = conv.real;
	conv = conv[:convSize[0], :convSize[1]];		
	return conv

def obstacle_space(X,sweep):
    # DO convolution of X with sweep to compute c-obstacle
    res = conv_GPU(X,sweep,True)
    res =  1 * (res[:,:] > 0.1) 
    plt.imshow(res) 
    plt.show()
    plt.imsave('res.jpeg',res)

def update_edge_zone(edges,template,centers):
    # when we know the centers and the orientations of the template at the 
    # matched points, we can replace the region of the edge detected portion
    # with the template
    for i in range(centers.shape[0]):
        center = centers[i,:]
        angle = center[-1]
        rot_template = transform.rotate(template,angle,order=0)
        cx = center[0]
        cy = center[1]
        sx = template.shape[0]
        sy = template.shape[1] 
        edges[cy:cy+sx, cx:cx+sy] = rot_template
    
    plt.imshow(edges)
    plt.show()
    plt.imsave('zone.jpeg',edges)
    return edges
    
    
def display_corr(edges, template, result, x,y):
    fig, (ax1, ax2, ax3) = plt.subplots(ncols=3, figsize=(8, 3))

    ax1.imshow(template)
    ax1.set_axis_off()
    ax1.set_title('template')
    
    ax2.imshow(edges)
    ax2.set_axis_off()
    ax2.set_title('image')
    # highlight matched region
    hcoin, wcoin = template.shape
    rect = plt.Rectangle((x, y), wcoin, hcoin, edgecolor='r', facecolor='none')
    ax2.add_patch(rect)
    
    ax3.imshow(result)
    ax3.set_axis_off()
    ax3.set_title('`match_template`\nresult')
    # highlight matched region
    ax3.autoscale(False)
    ax3.plot(x, y, 'o', markeredgecolor='r', markerfacecolor='none', markersize=10)
    
    plt.show()

def find_centers(edges,template):
    # Find centers because the edge detected image is not cleanly
    # identified as a rectangle (chip shape) - instead it's a 
    # collection of almost connected lines near the actual boundary
    # therefore we do template matching with thickened rectangle. The
    # thickened shape is also useful because it's a tolerance zone around
    # the real chip which can be used in the obstacle computation. Thus we
    # do template matching using a thickened chip boundary as the
    # template and identify the centers of the chips whose edges
    # have been detected
    centers = []
    # Note that the peaks in the output of match_template correspond 
    # to the origin (i.e. top-left corner) of the template. Therefore
    # we need to identify the center wrt the top left corner in the template 
    # \todo --- find the shift from the template image by taking centroid 
    
    # only need the shift to visualize plots
    #shift = [88,119] # if you look inside template.png, the center is approx here 
    
    print 'Doing template matching' 
    for angle in range(0,360,60):
        rot_template = transform.rotate(template,angle,order=0)
        result = feature.match_template(edges,rot_template)  
        ij = unravel_index(argmax(result), result.shape) 
        x, y = ij[::-1]    
        #print x,y,result[y,x]
        display_corr(edges,template,result,x,y)
        if(not centers):
            centers.append((x,y, result[y,x], angle)) 
        else:
            new_center = True
            for pt in centers:
                dist = max(abs(x-pt[0]), abs(y-pt[1])) 
                if( dist <10):
                    # treat as a new center point
                    new_center = new_center and False
                    if (result[y,x] > pt[2] and dist < 10):
                        centers[-1] = ((x,y, result[y,x], angle))
                else:
                    new_center = new_center and True
            if(new_center):
                centers.append((x,y, result[y,x], angle))         
        sys.stdout.write('.')
        sys.stdout.flush() 
    print 'Done' 
    centers = array(centers) 
    #centers[:,0] = centers[:,0]+shift[0]
    #centers[:,1] = centers[:,1]+shift[1] 
    #plt.imshow(edges)
    #plt.plot(centers[:,0], centers[:,1],'o',markeredgecolor='w', markerfacecolor='none', markersize=5)
    #plt.show()    
    return centers 
    
    # KNOWN BUG = if there are more than one maxima in rectangles that are separated 
    # by significant distance, we end up picking just one because of the argmax business
    # within the first for loop.

def chip_edges(img, roi):
    # sigma >=4 works well for assembler images 
    
    edges= filter.canny(img,sigma=2)   
    # force binary image
    edges =  1 * (edges[:,:] > 0.1)  
    print edges.shape
    roi_minx = roi[0]
    roi_miny = roi[1]
    roi_maxx = roi[2]
    roi_maxy = roi[3]
    res = edges[roi_minx:roi_maxx, roi_miny:roi_maxy] 

    
    #construct_template(edges)
    #plt.imshow(edges)
    #plt.show()
    #plt.imsave('canny.jpeg',edges)
    return res

def old_main_rectangle_chiplet():
    
    #9,18
    parser = OptionParser()
    (options, args) = parser.parse_args()
    # reading input files 
    
    img = io.imread(args[0])
    template = io.imread(args[1])
    template_sweep = io.imread(args[2])
    
    # remove alpha component
    img = img[:,:,0] 
    template = template[:,:,0]
    template_sweep= template_sweep[:,:,0]
    
    # specify a region of interest to crop the image
    # using ffmpeg -i videos/Movie\ S4\ Fig\ 3B\ wth\ caption.mp4 -r 1 -s 1600x1200  -f image2 foo-%03d.jpeg
    # the images were sized to be 1600x1200
    roi = [150,200,1100,150]
    edges = chip_edges(img,roi)
    
    centers = find_centers(edges,template)
    clean_zones = update_edge_zone(edges,template,centers) 
    sp = obstacle_space(clean_zones,template_sweep) 
    # The zones are generated conservatively
    # by doing obstacle computation wrt the rot-swept area of a chip
        
   
def main():
    parser = OptionParser()
    (options, args) = parser.parse_args()
    # reading input files 
     
    path = args[0] 

    # ref is the image that is used as a reference to subtract the background.
    
    roi = [125,125,1050,1250]
    roi_minx = roi[0]
    roi_miny = roi[1]
    roi_maxx = roi[2]
    roi_maxy = roi[3]
    
    ref = io.imread(args[1])  
    ref = ref[:,:,0] 
    #ref = ref[roi_minx:roi_maxx, roi_miny:roi_maxy] 
    
    for subdir, dirs, files in os.walk(path):
        for file in files:
            fn = os.path.join(subdir, file) 
            img = io.imread(fn)
            img = img[:,:,0]  
            #img = img[roi_minx:roi_maxx, roi_miny:roi_maxy] 
            find_centers(img,ref)
            

if __name__ == "__main__":
    main()