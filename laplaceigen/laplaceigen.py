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
from scipy.sparse.linalg import eigs  
import networkx.drawing.nx_pydot as pyd
import matplotlib.pyplot as plt

def visualize(tet, field_vals):
      
    grid = tet.grid
    grid.plot(scalars=field_vals, stitle='Eigenfunction', 
              cmap='bwr', show_edges = False,)
    

def eigenfunctions(laplacian, n):
    tic = now()
    evals, evecs = eigs(laplacian,n, return_eigenvectors=True)
    toc = now()-tic
    print('Computed Laplacian eigenfunctions in '+ repr(toc*1000) + ' ms')
    return (evals, np.real(evecs))

def drawGraph(dual):
    pos = nx.spectral_layout(dual)
    nx.draw(dual, pos=pos, with_labels=False)
    plt.show()

def laplacian(mesh):
    tic = now()
    # given a tet mesh (nodes, elements) derive a dual graph
    # whose nodes are the elements, and edges are drawn between 
    # nodes whose corresponding tetrahedra share a face
    dual = nx.Graph()
    # map each face to the elements that bound it
    fmap = defaultdict(set) 
    for (elid, tetvertices) in enumerate(mesh.elem): 
        dual.add_node(elid, verts=tetvertices) 
        for face in combinations(tetvertices,3):
            # iterate through the faces and map to the parent element
            face = tuple(sorted(face))
            #print(repr(face) + '--->' + repr(elid))
            fmap[face].add(elid)  
    # now create the edges between nodes in the dual graph 
    for k,v in fmap.items():
            # draw an edge between the items in the fmap
            # (i.e. the elements sharing a face)
            flist = list(v)
            #print(repr(k) + '--->' +  repr(flist))
            if(len(flist) ==2):
                # only add an edge when it bounds two faces,
                # not for boundary faces
                dual.add_edge(flist[0],flist[1])
                
    # Draw the dual graph 
    #drawGraph(dual)
    assert(nx.is_connected(dual))
    # convert the graph to an adjacency matrix
    A = nx.to_numpy_matrix(dual)
    degrees= np.array(dual.degree(range(len(mesh.elem))))
    row,col = np.diag_indices(A.shape[0])
    A[row,col] = degrees[:,1] 
    toc = now()-tic
    print('Constructed mesh Laplacian in '+repr(toc *1000) + ' ms' )
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
    evals, evecs = eigenfunctions(laplacian(tet),50)
    evecs = np.array(evecs)  
    visualize(tet, evecs[:,40])
    

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

