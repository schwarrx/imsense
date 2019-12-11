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
import sys
import pyvista as pv
import tetgen


def constructAdjacencyMatrix(grid):
    print(grid.cells)


def createTetMeshGrid(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    assert isinstance(tet.grid, object)
    grid = tet.grid
    return grid


def testBasicVisualization():
    # basic example of cut tetrahedralized sphere visualization
    # from tetgen documentation
    mesh = pv.Sphere()
    grid = createTetMeshGrid(mesh)
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
    # test the computation of eigenfunctions on the sphere
    mesh = pv.Sphere()
    grid = createTetMeshGrid(mesh)
    constructAdjacencyMatrix(grid)


def parsenputI():
    parser = OptionParser() 
    parser.add_option("-v", default=False, action='store_true', 
                      help="visualize eigenfunctions")
    parser.add_option("-s", default=False, action='store_true',
                      help="test spherical harmonics")
    
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
    spherical = options.s
    mesh = pv.read(args[0])
    #grid = createTetMeshGrid(mesh)

    if visualize :
        testBasicVisualization()

    if spherical:
        testSphericalHarmonics()
  
main() 

