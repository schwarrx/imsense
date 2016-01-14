# -*- coding: utf-8 -*-
"""
Created on Thu Jan 14 10:32:07 2016

@author: nelaturi
"""

from numpy import *
from scipy.stats import norm
import matplotlib.pyplot as plt

def show(x,f):
    # plot a function f sampled over points x
    fig,ax = plt.subplots(1,1)
    ax.plot(x,f)
    ax.legend(loc='best', frameon=False)
    plt.show()

def mog(x,weights, means,sigmas): #mixture of gaussians
    # To implement the algorithm it isn't necessary to actually compute a mog,
    # just useful to check examples and debug
    # x is the samples in a range , e.g. [0,256] for monochrome
    # Eq 2 in the paper
    assert (len(means)==len(sigmas))
    assert (len(means)==len(weights))
    theta_k = zip(means,sigmas,weights)
    # sample normal distribution functions with the means and sigmas
    f_k = [weight*norm.pdf(x,mean,sigma) for (mean,sigma,weight) in theta_k ] 
    # aggregate the corresponding elements into tuples and sum the tuples
    f = map(sum, zip(*f_k))  
    return array(f)