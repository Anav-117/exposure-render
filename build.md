# exposure-render-for-Linux  
Super Volume Render of Monte Carlo Path tracing for Linux  
## Introduction  
The code is a Linux distribution of exposure render.

## Dependency  
QT 5.14   (maybe 5.12+)  
CUDA 11.0  (maybe 10.0+)  
VTK 8.2  
GCC 9  

## BUILD && RUN  
```
cd source
mkdir build  
cd build
cmake ..  
make  
./ExposureRender  
```

## Reference  
* [An interactive photo-realistic volume rendering framework](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0038586)  
T. Kroes, F. H. Post, C. P. Botha
* [Visibility sweeps for joint-hierarchical importance sampling of direct lighting for stochastic volume rendering](http://graphicsinterface.org/proceedings/gi2015/gi2015-13/)  
T. Kroes, M. Eisemann, E. Eisemann
