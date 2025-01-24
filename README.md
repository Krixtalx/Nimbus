# Virtualized Point Cloud Rendering

![c++](https://img.shields.io/github/languages/top/Krixtalx/Nimbus) 
![opengl](https://img.shields.io/badge/-OpenGL_4.5-yellow?logo=opengl)

![License: CC BY-NC-SA 4.0](https://licensebuttons.net/l/by-nc-sa/4.0/80x15.png)

# About

This repository contains the source code for the paper 'Virtualized Point Cloud Rendering', currently submitted to IEEE Transactions on Graphics and Visualization. Our paper is inspired by previous works of the TU Wien research group. We move the rasterization pipeline from OpenGL's vertex and fragment shaders to compute shaders. However, one of the remaining drawbacks of this approach is that the point cloud must entirely fit in the GPU VRAM. While some computers out there have a large VRAM capacity, commodity hardware typically oscillates between 6 and 8GB. 

We propose that point cloud data can be transferred from disk to CPU and GPU as required. Therefore, we avoid having all the data in VRAM in what we call a virtualized point cloud rendering. Points are transferred on demand, and we can even simulate that our computer has a lower VRAM than it has.

<p align="center">
    <img src="readme_assets/response_time.png" width=400/>
</p>

# Features

The main feature of this project is the virtualization of point cloud data. However, some of the drawbacks of previous work remain even with virtualization. For instance, point clouds that do not entirely fit into the VRAM must be subsampled via a Level of Detail (LoD) system. The LoD is kind of a tradeoff between performance and not providing visual cues of missing points. Besides this, we also use a fast hole-filling algorithm that fills gaps as long as there is at least one non-empty pixel in a 3x3 neighborhood (which can be changed). We also offer occlusion checks that help to clean point clouds with several overlapping surfaces, as illustrated in the following image.

<p align="center">
    <img src="readme_assets/occlusion.png" width=400/>
</p>

We also propose another aggregation of points into meshlets (coined pointlets in our paper) using Hilbert encoding rather than Morton. While not perfect, it mitigates the spatial jumps reported in Morton. Note that a large pointlet is less likely to be discarded during frustum culling, and therefore, we would project points into our viewport even though they are not visible. 

<p align="center">
    <img src="readme_assets/sort_spot.png" width=400/>
</p>

# Building

- Clone the repository.
- Go to `Project scripts` and execute a setup file depending on your operative system. We provide it for Linux and Windows O.S.
- Compile the project with Visual Studio after opening `Nimbus.sln`. 

# How to cite

     @article{Araque_2024,
        title={Virtualized Point Cloud Rendering},
        url={http://dx.doi.org/10.36227/techrxiv.172254406.67063382/v1},
        DOI={10.36227/techrxiv.172254406.67063382/v1},
        publisher={Institute of Electrical and Electronics Engineers (IEEE)},
        author={Araque, José Antonio Collado and López, Alfonso and Jurado, Juan M and Jiménez, J Roberto},
        year={2024},
        month=aug 
    }