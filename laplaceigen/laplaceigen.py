#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Dec  9 13:31:36 2019

@author: nelaturi
"""

from optparse import OptionParser
from time import time as now
from typing import TextIO

from numpy import *
from pylab import * 
import sys
import pyvista as pv
import tetgen
from simplicial import SimplicialComplex

def tet2File(tet):
    print('# elements: ', len(tet.elem))
    print(tet.elem)

    print('# nodes: ', len(tet.node))
    print(tet.node)

    f_node: TextIO
    with open('Tet.node', 'w') as f_node:
        for item in tet.node:
            assert isinstance(item, object)
            f_node.write("%s\n" % item)
    f_node.close()

    f_elem: TextIO
    with open('Tet.elem', 'w') as f_elem:
        for item in tet.elem:
            f_elem.write("%s\n" % item)
    f_elem.close()

def createSimplicialComplex(tet):
    # turn the tet mesh into an oriented simplicial complex
    complex = SimplicialComplex(oriented=True)
    for elem in tet.elem:
        print(elem)
        complex.add(elem)

    complex.plot(False, "poset.png")


def createTetMeshGrid(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    assert isinstance(tet.grid, object)
    return tet


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
    tet = createTetMeshGrid(mesh)
    #tet2File(tet)
    createSimplicialComplex(tet)




def parseInput():
    parser = OptionParser() 
    parser.add_option("-v", default=False, action='store_true', 
                      help="visualize eigenfunctions")
    parser.add_option("-s", default=False, action='store_true',
                      help="test spherical harmonics")
    
    parser.usage = " ./laplaceigen.py [options] mesh"    
    (options, args) = parser.parse_args()
      
        
    
    return (options,args)


def main(): 
    
    print("Computing Laplacian Eigenfunctions")
    (options,args) = parseInput()
    visualize = options.v
    spherical = options.s
     

    if visualize :
        testBasicVisualization()
    elif spherical:
        testSphericalHarmonics()
    else:
        if len(args)!=1:
            print(parser.usage)
            sys.exit("Program requires an input mesh")
        else:
            mesh = pv.read(args[0])
  
main() 

