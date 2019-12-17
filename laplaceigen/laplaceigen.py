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
import networkx as nx
from itertools import combinations

def testBasicLaplacian():
    # check to see if  laplacian can be derived from simplicial complex
    c = SimplicialComplex(oriented=True)
    c.add([3,2,1])
    c.add([3,4,2])
    c.add([3,7,4])
    c.add([3,1,6])
    c.add([3,6,7]) 
    c.add([7,10,4])
    c.add([7,9,10])
    c.add([7,8,9])
    c.add([7,6,8])
    c.add([6,1,5])
    c.add([6,5,8])
    c.add([8,10,9]) 
    i = c.incidence_matrix(0,1)*c.incidence_matrix(1,0)
    
    l = i.toarray()
    print(l) 

def createDegMatrix(A):
    # use adjacency matrix to compute degree
    D = zeros_like(A)
    for i in range(A.shape[0]):
        D[i,i] = sum(A[i:])
    return D



def createAdjMatrix(tet): 
    # compute adjacency matrix
    nelems = len(tet.elem)
    elem_labels = range(nelems) 
    #find pairwise intersections  
    A = zeros((nelems,nelems))
    for p,q in combinations(elem_labels,2): 
        l1 = set(tet.elem[p])
        l2 = set(tet.elem[q])  
        adj = (len(l1 &l2) > 0) 
        if(adj):
            A[p,q] = 1  
    return A

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


def createTetMeshGrid(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    #tet2File(tet)
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

def computeLaplacian(mesh):
    tet = createTetMeshGrid(mesh)
    tet.make_manifold()
    print(tet.f)
    print("Tet mesh has " + repr(len(tet.elem)) + " elements")
    tic= now()
    A = createAdjMatrix(tet)
    toc = now()-tic
    print('Created adjacency matrix in '+repr(toc) +'s')
    tic = now()
    D = createDegMatrix(A)
    toc = now() - tic
    print('Created degree matrix in '+repr(toc) + ' s')
    L = D-A
    return L 

def parseInput():
    parser = OptionParser() 
    parser.add_option("-v", default=False, action='store_true', 
                      help="visualize eigenfunctions")
    parser.add_option("-s", default=False, action='store_true',
                      help="test spherical harmonics")
    parser.add_option("-t", default=False, action='store_true', 
                      help='test basic Laplacian matrix')
    
    parser.usage = " ./laplaceigen.py [options] mesh"    
    (options, args) = parser.parse_args()
      
    return (options,args)


def main(): 
    
    print("Computing Laplacian Eigenfunctions")
    (options,args) = parseInput()
    visualize = options.v
    spherical = options.s
    basictest = options.t
      

    if visualize :
        testBasicVisualization()
    elif spherical:
        testSphericalHarmonics()
    elif basictest:
        testBasicLaplacian()
    else:
        if len(args)!=1:
            print(parser.usage)
            sys.exit("Program requires an input mesh")
        else:
            mesh = pv.read(args[0])
            laplacian = computeLaplacian(mesh)
            print(laplacian)
main() 

