#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Dec  9 13:31:36 2019

@author: nelaturi
"""



from optparse import OptionParser
from time import time as now
from numpy import *
from pylab import *

import os
import sys
import pyvista as pv
import tetgen


def createTetMeshGrid(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    grid = tet.grid
    return grid


def testBasicVisualization(mesh, grid):
    # get cell centroids
    cells = grid.cells.reshape(-1, 5)[:, 1:]
    cell_center = grid.points[cells].mean(1)
    
    # extract cells below the 0 xy plane
    mask = cell_center[:, 2] < 0
    cell_ind = mask.nonzero()[0]
    subgrid = grid.extract_cells(cell_ind)
    
    cell_qual = subgrid.quality 
    # plot quality
    subgrid.plot(scalars=cell_qual, stitle='Quality', cmap='bwr', clim=[0,1],
                 flip_scalars=True, show_edges=True,) 
    
def testSphericalHarmonics():
    # check the code for correctness on spherical harmonics
    mesh = pv.Sphere()
    grid = createTetMeshGrid(mesh)
    testBasicVisualization(mesh,grid)
     

def parseInput():
    parser = OptionParser() 
    parser.add_option("-v", default=False, action='store_true', 
                      help="visualize eigenfunctions")  
    
    parser.usage = " ./laplaceigen.py [options] mesh"    
    (options, args) = parser.parse_args()
    if len(args)!=1:
        print(parser.usage)
        sys.exit("Program requires an input mesh") 
    else:
        print("Computing Laplacian Eigenfunctions")
    
    return (options,args)


def main(): 
    
    
    (options,args) = parseInput()
    visualize = options.v   
    mesh = pv.read(args[0])
    grid = createTetMeshGrid(mesh)
    
    
    
    if(visualize):
        testSphericalHarmonics()
  
main() 

