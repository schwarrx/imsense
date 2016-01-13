#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Tue Jan 12 17:25:19 2016

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

def mog(x,mean_array,covariance_array, weight_array): #mixture of gaussians
    # x is the samples in a range , e.g. [0,256] for monochrome
    assert (len(mean_array)==len(covariance_array))
    assert (len(mean_array)==len(weight_array))
    theta_k = zip(mean_array,covariance_array,weight_array)
    # sample normal distribution functions with the means and covariances
    f_k = [weight*norm.pdf(x,mean,cov) for (mean,cov,weight) in theta_k ]
    # aggregate the corresponding elements into tuples and sum the tuples
    f = map(sum, zip(*f_k))  
    return f


def main():
    # main function
    low = 0
    high= 256
    x = linspace(low,high,num=350)
    mean_array= [80,100,200]
    covariance_array = [20,5,10] 
    weight_array=[0.2,0.2,0.6]
    f = mog(x,mean_array,covariance_array,weight_array) 
    show(x,f)
    

# Et voila
if __name__ == "__main__":
    main()