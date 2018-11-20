#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Apr 21 19:35:32 2018

@author: linnea
"""

def minDist(graph, dist, d):
    min = float("inf")
    
    for v in range(len(graph)):
        if dist[v] < min and d[v] == False:
            min = dist[v]
            min_index = v
    
    return min_index

def dijkstra(graph, start, end):

    p = {start:None}
    dist = [float("inf")] * len(graph)
    dist[start] = 0
    d = [False] * len(graph)

    for _ in range(len(graph)):
        u = minDist(graph, dist, d)
        d[u] = True
        
        for v in range(len(graph)):
            if graph[u][v] > 0 and d[v] == False and dist[v] > dist[u] + graph[u][v]:
                dist[v] = dist[u] + graph[u][v]
                p[v] = u
       
    node = end
    path = []         
    while node != None:
        path.append(node)
        node = p[node]
    
    return path[::-1]

if __name__ == "__main__":
    roads =  [[ 0,  2,  0, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
              [ 2,  0,  5,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0],
              [ 0,  5,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0],
              [10,  0,  0,  0, 15,  0,  0,  0,  0,  0, 35,  0,  0,  0],
              [ 0,  1,  0, 15,  0,  7,  0,  0,  0,  0,  0,  0,  0,  0],
              [ 0,  0,  0,  0,  7,  0, 10,  0,  0,  0,  0,  0, 12,  0],
              [ 0,  0,  6,  0,  0, 10,  0,  0,  0,  0,  0,  0,  0,  2],
              [ 0,  0,  0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0],
              [ 0,  0,  0,  0,  0,  0,  0,  5,  0,  2,  0,  4,  0,  0],
              [ 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0],
              [ 0,  0,  0, 35,  0,  0,  0,  0,  0,  0,  0, 15,  0,  0],
              [ 0,  0,  0,  0,  0,  0,  0,  0,  4,  0, 15,  0,  8,  0],
              [ 0,  0,  0,  0,  0, 12,  0,  0,  0,  0,  0,  8,  0,  3],
              [ 0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  3,  0]]
    
    
    print(dijkstra(roads,0,8))

