#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 17:25:19 2016

@author: nelaturi
"""
from numpy import *

# Implementation of 'Understanding Background Mixture Models for Foreground
# Segmentation' by P.W. Power and J.A. Schoonees. This paper describes how
# to implement 'Adaptive background mixture models for real-time tracking' 
# by C. Stauffer and W.E.L.Grimson
    
def find_match(X_t, weights_t, means_t, sigmas_t):
    # X_t is the observed pixel value (see paper notation) at time t 
    # check if the Mahalanobis distance is < 2.5 standard devs for x \in X
    dists = [pow((X_t-mean)/sigma,2) < 6.25 for (mean,sigma) in zip(means_t,sigmas_t)]
    match = [x.astype(int) for x in array(dists)] # Equation 15 in the paper
    # if an intensity is matched to two Gaussians, pick the one with largest 
    # weight/ std-dev 
    nmatches = len(nonzero(match)[0])   
    if (nmatches >1): 
        #more than one match, select the distribution with the highest peak
        #while masking the peak array with zeros for non-matching distributions 
        peaks = [m*weight/sigma for (weight,sigma,m) in zip(weights_t,sigmas_t,match)]  
        max_peak = peaks.index(max(peaks)) 
        match = [0] * len(dists)
        match[max_peak] = 1
        return match
    elif (nmatches ==0):
        # no matches at all.
        # lowest peaking distribution is replaced with a new wide Gaussian 
        # centered at the new pixel value. Gives the opportunity for newly 
        # stationary objects to get gradually absorbed into the background 
        peaks = [weight/sigma for (weight,sigma) in zip(weights_t,sigmas_t)]
        idx = peaks.index(min(peaks))
        means_t[idx] = X_t
        sigmas_t[idx] = 50 # some large number
        return match # an empty list 
    else:
        #assert (len(matches) ==1) # assert there is only one match
        # return the index of the Gaussian in the mixture model for which there is match 
        return match

def update_priors(X_t, match_t, t, weights_t, means_t, sigmas_t):
    # update rules based on Equations 10-13 , t is time ,k is the matched index
    alpha_t = max(1/t,0.005)  # learning rate is lower bounded by 0.005 (5 fps) 
    for k in range(len(weights_t)): 
        weights_t[k] = (1-alpha_t) * weights_t[k] + alpha_t *match_t[k] 
        rho_kt = alpha_t/weights_t[k] #Equation 16 
        means_t[k] = (1-rho_kt) * means_t[k] + rho_kt * X_t  
        sigmas_t[k] = sqrt((1-rho_kt)* pow(sigmas_t[k],2) + rho_kt * pow(X_t-means_t[k],2))   
        
def segment(weights_t, sigmas_t):
    # rank the states in each distribution 
    mask = zeros_like(weights_t)
    ranks = array([weight/sigma for (weight,sigma) in zip(weights_t,sigmas_t)])  
    print 'weighted peaks =' +repr(ranks)
    temp = ranks.argsort() 
    ranks[temp] = arange(len(ranks)) # rank positions measured by weight/sigma   
    print 'ranked weighted peaks = ' +repr(ranks)
    for i,j in enumerate(ranks.astype(int)): 
        mask[j] = weights_t[i] 
    print 'weights =' +repr(weights_t)
    print 'Cumulative sums = ' +repr(cumsum(mask))
    B = list(cumsum(mask) > 0.7).index(True) # see Equation 5
    bg = (ranks <= B)
    return bg
        
def main():
    # main function 
    means= [80,100,200]
    sigmas = [20,5,10] 
    weights=[0.2,0.2,0.6]
    #f = mog(x,weights, means,sigmas) 
    #show(x,f)
    # create current state for one of the components 
    X_t = list(array([100,101,101,100,100,102,99,97,101,150,151,150,100,100])+5)
    #print X_t
    
    t = 1
    for x_t in X_t:  
        t+=1
        matches = find_match(x_t, weights,means,sigmas)   
        if not(matches):
            # no matches found, classify X_t as foreground
            print 'no matches'
        else: 
            print x_t 
            print segment(weights, sigmas)   
            update_priors(x_t,matches,t,weights,means,sigmas)  
          

# Et voila
if __name__ == "__main__":
    main()