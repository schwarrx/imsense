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
from itertools import combinations, product
from collections import defaultdict
from scipy.sparse.linalg import eigs  
import networkx.drawing.nx_pydot as pyd
import matplotlib.pyplot as plt
from scipy.special import sph_harm

def visualize(tet, field_vals, nx, ny): 
    p = pv.Plotter(shape=(nx,ny))
    idx= 0
    for x,y in product(range(nx), range(ny)):
        p.subplot(x,y)
        #print(field_vals[:,idx])
        field_val = field_vals[:,idx]
        field_val/= np.linalg.norm(field_val)
        p.add_mesh(tet.grid, scalars=field_val, show_edges=False )
        idx +=1
    p.show()

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
    # note - cell_dim = 2 for surface meshes, 3 for tet meshes
    # TODO -- update the laplacian code to be cell dimension independent
    # Right now it only works for tet meshes
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
    #assert(nx.is_connected(dual))
    # convert the graph to an adjacency matrix
    A = nx.to_numpy_matrix(dual)
    degrees= np.array(dual.degree(range(len(mesh.elem))))
    row,col = np.diag_indices(A.shape[0])
    A[row,col] = degrees[:,1] 
    toc = now()-tic
    print('Constructed mesh Laplacian in '+repr(toc *1000) + ' ms' )
    print(A)
    return A
    
def createTetMesh(mesh):
    tet = tetgen.TetGen(mesh)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    tet.make_manifold()
    #tet2File(tet)
    assert isinstance(tet.grid, object)
    print('Number of tets =' + repr(len(tet.elem)))  
    return tet

def computeLaplacian(tet, nevecs):  
    evals, evecs = eigenfunctions(laplacian(tet),nevecs)
    evecs = np.array(evecs)  
    return (evals, evecs)
    

def parseInput():
    parser = OptionParser()  
    parser.add_option("-s", default=False, action='store_true',
                      help="test spherical harmonics")
    parser.add_option("-t", default=False, action='store_true', 
                      help='test basic Laplacian matrix')
    
    parser.usage = " ./laplaceigen.py [options] mesh"    
    (options, args) = parser.parse_args()
    return (options,args) 

def testSphericalHarmonics():
    # test spherical harmonics as eigenfunctions of laplacian on sphere
    sphere = pv.Sphere(theta_resolution=20, phi_resolution=20)
    # note that this function tests against the surface mesh of the sphere
    # does not invoke tetrahedralization. If the code is correctly written 
    # the approach should scale by switching dimensions 
    
    tet = tetgen.TetGen(sphere)
    tet.tetrahedralize(order=1, mindihedral=20, minratio=1.5)
    #tet.make_manifold()
    print('Number of tets =' + repr(len(tet.elem)))
    nevecs = 12
    evals, evecs = computeLaplacian(tet, nevecs)  
    # test against spherical harmonic Y^m_l (see Wikipedia)
    coords = tet.node  
    # compute the spherical coords from cartesian coords  
    sphmap = lambda x,y,z: (np.hypot(np.hypot(x,y),z), 
                            np.arctan2(y,x),
                            np.arccos(z/ np.hypot(np.hypot(x,y),z))) 
    sphcoords = np.array(list(map(sphmap, coords[:,0], coords[:,1], 
                                  coords[:,2])))
    
    # assert that the conversion was done coorrectly
    cartmap = lambda r,phi, theta: (r*np.sin(theta)*np.cos(phi), 
                                    r*np.sin(theta)*np.sin(phi), 
                                    r* np.cos(theta))
    remap = np.array(list(map(cartmap, sphcoords[:,0], sphcoords[:,1], sphcoords[:,2])))
    assert(np.linalg.norm(remap-coords) < 1e-10)
    
    m,n = 0,0
    harmonics = np.real(sph_harm(m,n,sphcoords[:,1], sphcoords[:,2]))
    evec = evecs[:,0]
    normeigs = evec/np.linalg.norm(evec)
    
    #print(normeigs, harmonics)
    visualize(tet,evecs,3,4)
     
    '''
    #visualize
    p = pv.Plotter(shape=(1,2))
    p.subplot(0,0)
    p.add_mesh(tet.grid,scalars=harmonics, show_edges=False)
    p.subplot(0,1)
    p.add_mesh(tet.grid,scalars=normeigs, show_edges=True)
    p.show()
    '''
    
    

def main():  
    print("Computing Laplacian Eigenfunctions")
    (options,args) = parseInput() 
    spherical = options.s
    basictest = options.t
    
    if spherical:
        testSphericalHarmonics()
    elif basictest:
        testBasicLaplacian()
    else:
        if len(args)!=1:
            print(parser.usage)
            sys.exit("Program requires an input mesh")
        else:
            mesh = pv.read(args[0])
            tet = createTetMesh(mesh)
            nevecs = 100
            (evals, evecs) = computeLaplacian(tet,nevecs)
            visualize(tet,evecs, 10,10)
main() 

