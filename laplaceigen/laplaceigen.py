#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Dec  9 13:31:36 2019

@author: nelaturi
"""

from optparse import OptionParser
from time import time as now
from typing import TextIO

import numpy as np 
import sys
import pyvista as pv
import tetgen 
import networkx as nx
from itertools import combinations
from collections import defaultdict
 

def laplacian(mesh):
    # given a tet mesh (nodes, elements) derive a dual graph
    # whose nodes are the elements, and edges are drawn between 
    # nodes whose corresponding tetrahedra share a vertex
    # note: this is not strictly the dual in the sense of simplicial complex
    dual = nx.Graph()
    # map each vertex to all the elements that contain it
    vmap = defaultdict(list) 
    for (elid, tetvertices) in enumerate(mesh.elem): 
        dual.add_node(elid, verts=tetvertices)
        for vertex in tetvertices:
            # iterate through the nodes in the element and create vertex map
            vmap[vertex].append(elid)
    # now create the edges between nodes in the dual graph 
    for k,v in vmap.items():
            # draw an edge between the pairwise combinations of
            # the value corresponding to a vertex key
            dual.add_edges_from(list(combinations(v,2)))  
    # convert the graph to an adjacency matrix
    A = nx.to_numpy_matrix(dual)
    degrees= np.array(dual.degree(range(len(mesh.elem))))
    row,col = np.diag_indices(A.shape[0])
    A[row,col] = degrees[:,1] 
    return A
    


def createTetMeshGrid(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    #tet2File(tet)
    assert isinstance(tet.grid, object)
    print('Number of tets =' + repr(len(tet.elem)))
    return tet

def computeLaplacian(mesh):
    tet = createTetMeshGrid(mesh)
    tet.make_manifold()  
    tic = now()
    laplacian(tet) 
    toc = now()-tic
    print('Constructed mesh Laplacian in '+repr(toc*1000) + 'ms')  

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
            computeLaplacian(mesh)
            #print(laplacian)
main() 

