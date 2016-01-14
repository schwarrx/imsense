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
# Segmentation' by P.W. Power and J.A. Schoonees. This paper describes how
# to implement 'Adaptive background mixture models for real-time tracking' 
# by C. Stauffer and W.E.L.Grimson

def show(x,f):
    # plot a function f sampled over points x
    fig,ax = plt.subplots(1,1)
    ax.plot(x,f)
    ax.legend(loc='best', frameon=False)
    plt.show()



def mog(x,weights, means,sigmas): #mixture of gaussians
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
    
def match(X_t, weights_t, means_t, sigmas_t):
    # X_t is the observed pixel value (see paper notation) at time t 
    # check if the Mahalanobis distance is < 2.5 standard devs for x \in X
    dists = [pow((X_t-mean)/sigma,2) < 6.25 for (mean,sigma) in zip(means_t,sigmas_t)]
    approx = [x.astype(int) for x in array(dists)] # Equation 15 in the paper
    # if an intensity is matched to two Gaussians, pick the one with largest 
    # weight/ std-dev
    weighted_approx = array([x * w/sigma for (x,w,sigma) in zip(approx,weights_t,sigmas_t)]) 
    maxs = weighted_approx.max(axis=0)
    putmask(weighted_approx, weighted_approx < maxs, 0)
    matches= nonzero(weighted_approx)[0]
    if not(matches):
        # lowest peaking distribution is replaced with a new wide Gaussian 
        # centered at the new pixel value

        ##### Need to classify as foreground ..otherwise everything will always
        # be in the background    
    
        peaks = [weight/(sigma*sqrt(2*pi)) for (weight,sigma) in zip(weights_t,sigmas_t)]
        idx = peaks.index(min(peaks))
        means_t[idx] = X_t
        sigmas_t[idx] = 50 # some large number 
    else:
        assert (len(matches) ==1) # assert there is only one match
        # return the index of the Gaussian in the mixture model for which there is match
        return matches[0]

def update_priors(X_t, k, t, weights_t, means_t, sigmas_t):
    # update rules based on Equations 10-13 , t is time ,k is the matched index
    alpha_t = max(1/t,0.005)  # learning rate is lower bounded by 0.005 (5 fps) 
    weights_t[k] = (1-alpha_t) * weights_t[k] + alpha_t # only need to update the matched component 
    rho_kt = alpha_t/weights_t[k] #Equation 16 
    means_t[k] = (1-rho_kt) * means_t[k] + rho_kt * X_t  
    sigmas_t[k] = sqrt((1-rho_kt)* pow(sigmas_t[k],2) + rho_kt * pow(X_t-means_t[k],2))   
        
def segment(weights_t, sigmas_t):
    # rank the states in each distribution 
    mask = zeros_like(weights_t)
    ranks = array([weight/sigma for (weight,sigma) in zip(weights_t,sigmas_t)]) 
    temp = ranks.argsort() 
    ranks[temp] = arange(len(ranks)) # rank positions measured by weight/sigma  
    for i,j in enumerate(ranks.astype(int)): 
        mask[j] = weights_t[i] 
    B = list(cumsum(mask) > 0.7).index(True) # see Equation 5
    bg = (ranks <= B)
    return bg
        
def main():
    # main function
    low = 0
    high= 256
    x = linspace(low,high,num=350)
    means= [80,100,200]
    sigmas = [20,5,10] 
    weights=[0.2,0.2,0.6]
    f = mog(x,weights, means,sigmas) 
    #show(x,f)
    # create current state for one of the components 
    X_t = 250
    t = 5
    k = match(X_t, weights,means,sigmas)  
    if(not(not(k))):
        update_priors(X_t,k,t,weights,means,sigmas)   

    print segment(weights, sigmas)
    print weights,sigmas

# Et voila
if __name__ == "__main__":
    main()