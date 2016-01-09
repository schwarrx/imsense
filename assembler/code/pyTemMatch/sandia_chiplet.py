#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Sun Aug 10 15:05:49 2014

@author: nelaturi
"""

import sys,os 
from optparse import OptionParser
from numpy import *   
from skimage import io
from skimage import filter
from skimage import feature
from skimage import transform
import re

# for display
import matplotlib.pyplot as plt


    
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


def label(x,y,imshape):
    # given an x,y, position, label it as falling into one of 4 quadrants
    # chips are labeled clockwise from top left as 1,2,3,4
    
    xc = imshape[0]/2
    yc = imshape[1]/2
    if (x < xc and y < yc ):
        return 1
    elif (x > xc and y < yc):
        return 2
    elif (x > xc and y > yc):
        return 3
    elif (x < xc and y > yc):
        return 4

def find_centers(image,template):
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
    
    imshape = shape(image)  
    
    print 'Doing template matching' 
    for angle in range(0,360,60):
        rot_template = transform.rotate(template,angle,order=0)
        result = feature.match_template(image,rot_template)  
        #print result
        ij = unravel_index(argmax(result), result.shape) 
        x, y = ij[::-1]    
        # assign a label to this x,y position depending on the quadrant
        l = label(x,y,imshape)
        
        #print x,y,result[y,x] 
        display_corr(image,template,result,x,y)
        if(not centers):
            #centers.append((x,y, result[y,x], angle)) 
            centers.append((x,y, result[y,x], l))   
        else:
            new_center = True
            for pt in centers: 
                dist = max(abs(x-pt[0]), abs(y-pt[1])) 
                if( dist <10):
                    # treat as a new center point
                    new_center = new_center and False
                    if (result[y,x] > pt[2] and dist < 10):
                        #centers[-1] = ((x,y, result[y,x], angle))
                        centers[-1] = ((x,y, result[y,x],l))
                else:
                    new_center = new_center and True
            if(new_center):
                #centers.append((x,y, result[y,x], angle))   
                centers.append((x,y, result[y,x],l))  
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


def dists(chip1,ref1):
    chip1_coords = chip1[:,:2]-ref1
    chip1_dists = sqrt(pow(chip1_coords[:,0],2) + pow(chip1_coords[:,1],2) )
    chip1_data = array(zip(chip1_dists,chip1[:,2])) 
    return chip1_data

def post_process(output, roi):
    # we have sorted output with center locations for each chip
    chip1 = []
    chip2 = []
    chip3 = []
    chip4 = []    
    
    # first display the number of data points calculated for each chip
    for data in output:
        center_data = data[1]
        for info in center_data:
            chip = int(info[3])
            chip_info = zeros(3)
            chip_info[:2] = info[:2] 
            chip_info[2] = data[0] 
            if chip == 1:
                chip1.append(chip_info)
            elif chip ==2:
                chip2.append(chip_info)
            elif chip ==3:
                chip3.append(chip_info)
            elif chip ==4:
                chip4.append(chip_info)
        
    chip1 =  array(chip1)
    chip2 =  array(chip2)
    chip3 = array(chip3)
    chip4 = array(chip4)
          
    # manually identified refs from image 373
    ref1 = array((164,63))
    ref2 = array((784,34))
    ref3 = array((797,688))
    ref4 = array((172,715))
    

    dist1 = dists(chip1,ref1)
    dist2 = dists(chip2,ref2)
    dist3 = dists(chip3,ref3)
    dist4 = dists(chip4,ref4)
    
    plt.plot(dist1[:,0],dist1[:,1],color='g')
    #
    plt.plot(dist2[:,0],dist2[:,1],color='r')
    #plt.plot(dist3[:,0],dist3[:,1],color='b')
    plt.plot(dist4[:,0],dist4[:,1],color='k')
    plt.show()
    
   
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
    
    template = io.imread(args[1])  
    template = template[:,:,0] 
    #ref = ref[roi_minx:roi_maxx, roi_miny:roi_maxy] 
    
    # otsu threshold the ref
    thresh = filter.threshold_otsu(template)
    template = template < thresh
    #plt.imshow(ref)
    #plt.show()    
    
    ref = io.imread(args[2])
    ref = ref[:,:,0]
    ref = ref[roi_minx:roi_maxx, roi_miny:roi_maxy] 
    thresh = filter.threshold_otsu(ref)
    ref= ref < thresh
    #plt.imshow(ref)
    #plt.show()
    
    output = []    
    
    for subdir, dirs, files in os.walk(path):
        for file in files:
            fn = os.path.join(subdir, file) 
            img = io.imread(fn)
            img = img[:,:,0]  
            img = img[roi_minx:roi_maxx, roi_miny:roi_maxy] 
            # do otsu thresholding
            thresh = filter.threshold_otsu(img)
            img = img < thresh
            #img = abs(img-ref) 
            #plt.imshow(img)
            #plt.show()
            centers = find_centers(img,template)
            # get the image number from the file 
            
            n = int(re.search(r'\d+', file).group())
            output.append((n, centers))
            
    output = sorted(output, key=lambda x:x[0])

    # now post process the output
    post_process(output,roi)
        
    

if __name__ == "__main__":
    main()