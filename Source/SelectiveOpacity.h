class SelectiveOpacity {

public: 
    SelectiveOpacity();
    virtual ~SelectiveOpacity();

    void    InitOpacityArray(float* Opacity, int num);
    void    InitDensityScaleArray(float* DensityScale, int num);
    void    SetDensityScaleArray(float* Opacity);
    void    SetOpacityArray(float* DensityScale);
    float*  GetOpacityArray(void);
    float*  GetDensityScaleArray(void);
    void    SetNumSegments(int num);
    int     GetNumSegments(void);

private:
    float*  OpacityArray;
    float*  DensityScaleArray;
    int     NumSegments;
};

extern SelectiveOpacity gSelectiveOpacity;