#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 17:25:19 2016

@author: nelaturi
"""
from numpy import *

#import helpers
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
    ranks = array([weight/sigma for (weight,sigma) in zip(weights_t,sigmas_t)])   
    temp = ranks.argsort() 
    ranks[temp] = arange(len(ranks)) # rank positions measured by weight/sigma    
    sorted_weights= array(sorted(zip(weights_t, ranks,range(len(weights_t))),key=lambda x:(-x[1],x[0])))
    cumulative_weights = cumsum(sorted_weights[:,0]) 
    bg_cutoff = list(cumulative_weights > 0.8).index(True)
    bg = sorted_weights[:bg_cutoff+1,:]
    fg = sorted_weights[bg_cutoff+1:,:]  
    return bg,fg
        
def main():
    # main function 
    means= [30, 64,128,192,220]
    sigmas = [10,20,10,10,5] 
    weights=[0.1, 0.1,0.2,0.4,0.2]
    #x = linspace(0,256,300)
    #f = helpers.mog(x,weights, means,sigmas) 
    #helpers.show(x,f)
    # create current state for one of the components 
    X_t = list(array([100,101,101,100,100,102,99,97,101,150,151,150,100,100])+5)
    #print X_t
    
    assert (sum(weights) == 1.0)
    t = 1
    for x_t in X_t:  
        t+=1
        matches = find_match(x_t, weights,means,sigmas)   
        if not(matches):
            # no matches found, classify X_t as foreground
            print 'no matches'
        else: 
            #print x_t 
            bg, fg = segment(weights, sigmas)   
            bg_ids = bg[:,2]
            fg_ids = fg[:,2]
            print bg_ids, fg_ids 
            update_priors(x_t,matches,t,weights,means,sigmas)  
          

# Et voila
if __name__ == "__main__":
    main()