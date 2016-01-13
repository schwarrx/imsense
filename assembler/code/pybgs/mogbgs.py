#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 17:25:19 2016

@author: nelaturi
"""
from numpy import *
from scipy.stats import norm
import matplotlib.pyplot as plt

# Implementation of 'Understanding Background Mixture Models for Foreground
# Segmentation' by P.W. Power and J.A. Schoonees

def show(x,f):
    # plot a function f sampled over points x
    fig,ax = plt.subplots(1,1)
    ax.plot(x,f)
    ax.legend(loc='best', frameon=False)
    plt.show()


def mog(x,means,covariances, weights): #mixture of gaussians
    # x is the samples in a range , e.g. [0,256] for monochrome
    # Eq 2 in the paper
    assert (len(means)==len(covariances))
    assert (len(means)==len(weights))
    theta_k = zip(means,covariances,weights)
    # sample normal distribution functions with the means and covariances
    f_k = [weight*norm.pdf(x,mean,cov) for (mean,cov,weight) in theta_k ]
    # aggregate the corresponding elements into tuples and sum the tuples
    f = map(sum, zip(*f_k))  
    return array(f)

def posterior(x,mean,cov, weight, mog):
    # Apply Bayes' theorem to estimate the probability of the current state
    # = prior * weight/ mog. Eq 3 in the paper - only needed for checking
    # Useful in estimating which of the k distributions in the mog most likely
    # give rise to the current sample
    prior = norm.pdf(x,mean,cov)
    return ((prior* weight)/mog) 
    
def approx_posterior(X,means_t, covariances_t, weights_t):
    # X is the observed signal (see paper notation) at time t 
    # check if the Mahalanobis distance is < 2.5 standard devs for x \in X
    dists = [pow((X-mean)/covar,2) < 6.25 for (mean,covar) in zip(means_t,covariances_t)]
    approx = [x.astype(int) for x in dists] # Equation 15 in the paper
    # if an intensity is matched to two Gaussians, pick the one with largest 
    # weight/ covariance
    weighted_approx = array([x * w for (x,w) in zip(approx,weights_t)])
    maxs = weighted_approx.max(axis=0)
    putmask(weighted_approx, weighted_approx < maxs, 0)
    return array(weighted_approx >0).astype(int)
    
    
    return approx

def main():
    # main function
    low = 0
    high= 256
    x = linspace(low,high,num=350)
    means= [80,100,200]
    covariances = [20,5,10] 
    weights=[0.2,0.2,0.6]
    f = mog(x,means,covariances,weights) 
    # create current state for one of the components
    n = 2
    s = posterior(x,means[n], covariances[n],weights[n],f)
    #show(x,s)
    s1 = approx_posterior(f,means,covariances,weights)
    print s1

# Et voila
if __name__ == "__main__":
    main()